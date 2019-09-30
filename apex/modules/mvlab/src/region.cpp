/*
By downloading, copying, installing or using the software you agree to this
license. If you do not agree to this license, do not download, install,
copy or use the software.

                          License Agreement
               For Open Source Computer Vision Library
                       (3-clause BSD License)

Copyright (C) 2013, OpenCV Foundation, all rights reserved.
Third party copyrights are property of their respective owners.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the names of the copyright holders nor the names of the contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are
disclaimed. In no event shall copyright holders or contributors be liable for
any direct, indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.
*/

#include "precomp.hpp"
#include "opencv2/mvlab/region.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "opencv2/core/hal/hal.hpp"

namespace cv {
namespace mvlab {

using namespace std;


/**
  */
Region::Region(const Ptr<Region> &_dictionary) {
    markerSize = _dictionary->markerSize;
    maxCorrectionBits = _dictionary->maxCorrectionBits;
    bytesList = _dictionary->bytesList.clone();
}


/**
  */
Region::Region(const Mat &_bytesList, int _markerSize, int _maxcorr) {
    markerSize = _markerSize;
    maxCorrectionBits = _maxcorr;
    bytesList = _bytesList;
}


/**
 */
Ptr<Region> Region::create(int nMarkers, int markerSize, int randomSeed) {
    const Ptr<Region> baseDictionary = makePtr<Region>();
    return create(nMarkers, markerSize, baseDictionary, randomSeed);
}


/**
 */
Ptr<Region> Region::create(int, int,
                                   const Ptr<Region> &, int) {

    return makePtr<Region>();
}


/**
 */
Ptr<Region> Region::get(int) {
    return makePtr<Region>();
}


/**
 */
bool Region::identify(const Mat &onlyBits, int &idx, int &rotation,
                          double maxCorrectionRate) const {

    CV_Assert(onlyBits.rows == markerSize && onlyBits.cols == markerSize);

    int maxCorrectionRecalculed = int(double(maxCorrectionBits) * maxCorrectionRate);

    // get as a byte list
    Mat candidateBytes = getByteListFromBits(onlyBits);

    idx = -1; // by default, not found

    // search closest marker in dict
    for(int m = 0; m < bytesList.rows; m++) {
        int currentMinDistance = markerSize * markerSize + 1;
        int currentRotation = -1;
        for(unsigned int r = 0; r < 4; r++) {
            int currentHamming = cv::hal::normHamming(
                    bytesList.ptr(m)+r*candidateBytes.cols,
                    candidateBytes.ptr(),
                    candidateBytes.cols);

            if(currentHamming < currentMinDistance) {
                currentMinDistance = currentHamming;
                currentRotation = r;
            }
        }

        // if maxCorrection is fullfilled, return this one
        if(currentMinDistance <= maxCorrectionRecalculed) {
            idx = m;
            rotation = currentRotation;
            break;
        }
    }

    return idx != -1;
}


/**
  */
int Region::getDistanceToId(InputArray bits, int id, bool allRotations) const {

    CV_Assert(id >= 0 && id < bytesList.rows);

    unsigned int nRotations = 4;
    if(!allRotations) nRotations = 1;

    Mat candidateBytes = getByteListFromBits(bits.getMat());
    int currentMinDistance = int(bits.total() * bits.total());
    for(unsigned int r = 0; r < nRotations; r++) {
        int currentHamming = cv::hal::normHamming(
                bytesList.ptr(id) + r*candidateBytes.cols,
                candidateBytes.ptr(),
                candidateBytes.cols);

        if(currentHamming < currentMinDistance) {
            currentMinDistance = currentHamming;
        }
    }
    return currentMinDistance;
}



/**
 * @brief Draw a canonical marker image
 */
void Region::drawMarker(int id, int sidePixels, OutputArray _img, int borderBits) const {

    CV_Assert(sidePixels >= (markerSize + 2*borderBits));
    CV_Assert(id < bytesList.rows);
    CV_Assert(borderBits > 0);

    _img.create(sidePixels, sidePixels, CV_8UC1);

    // create small marker with 1 pixel per bin
    Mat tinyMarker(markerSize + 2 * borderBits, markerSize + 2 * borderBits, CV_8UC1,
                   Scalar::all(0));
    Mat innerRegion = tinyMarker.rowRange(borderBits, tinyMarker.rows - borderBits)
                          .colRange(borderBits, tinyMarker.cols - borderBits);

    // resize tiny marker to output size
    cv::resize(tinyMarker, _img.getMat(), _img.getMat().size(), 0, 0, INTER_NEAREST);
}




/**
  * @brief Transform matrix of bits to list of bytes in the 4 rotations
  */
Mat Region::getByteListFromBits(const Mat &bits) {
    // integer ceil
    int nbytes = (bits.cols * bits.rows + 8 - 1) / 8;

    Mat candidateByteList(1, nbytes, CV_8UC4, Scalar::all(0));
    unsigned char currentBit = 0;
    int currentByte = 0;

    // the 4 rotations
    uchar* rot0 = candidateByteList.ptr();
    uchar* rot1 = candidateByteList.ptr() + 1*nbytes;
    uchar* rot2 = candidateByteList.ptr() + 2*nbytes;
    uchar* rot3 = candidateByteList.ptr() + 3*nbytes;

    for(int row = 0; row < bits.rows; row++) {
        for(int col = 0; col < bits.cols; col++) {
            // circular shift
            rot0[currentByte] <<= 1;
            rot1[currentByte] <<= 1;
            rot2[currentByte] <<= 1;
            rot3[currentByte] <<= 1;
            // set bit
            rot0[currentByte] |= bits.at<uchar>(row, col);
            rot1[currentByte] |= bits.at<uchar>(col, bits.cols - 1 - row);
            rot2[currentByte] |= bits.at<uchar>(bits.rows - 1 - row, bits.cols - 1 - col);
            rot3[currentByte] |= bits.at<uchar>(bits.rows - 1 - col, row);
            currentBit++;
            if(currentBit == 8) {
                // next byte
                currentBit = 0;
                currentByte++;
            }
        }
    }
    return candidateByteList;
}

}
}
