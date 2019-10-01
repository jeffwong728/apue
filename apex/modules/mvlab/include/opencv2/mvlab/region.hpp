#ifndef __OPENCV_REGION_HPP__
#define __OPENCV_REGION_HPP__

#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W Region {

    public:
    CV_PROP_RW Mat bytesList;         // marker code information
    CV_PROP_RW int markerSize;        // number of bits per dimension
    CV_PROP_RW int maxCorrectionBits; // maximum number of bits that can be corrected

    Region(const Mat &_bytesList = Mat(), int _markerSize = 0, int _maxcorr = 0);
    Region(const Ptr<Region> &_dictionary);

    CV_WRAP_AS(create) static Ptr<Region> create(int nMarkers, int markerSize, int randomSeed=0);
    CV_WRAP_AS(create_from) static Ptr<Region> create(int nMarkers, int markerSize, const Ptr<Region> &baseDictionary, int randomSeed=0);
    CV_WRAP static Ptr<Region> get(int dict);
    bool identify(const Mat &onlyBits, int &idx, int &rotation, double maxCorrectionRate) const;
    int getDistanceToId(InputArray bits, int id, bool allRotations = true) const;
    CV_WRAP void drawMarker(int id, int sidePixels, OutputArray _img, int borderBits = 1) const;
    CV_WRAP static Mat getByteListFromBits(const Mat &bits);

    CV_WRAP static Ptr<Region> createRectangle(const Size &rectSize);
    CV_WRAP int connect(const Ptr<Region> &srcRegion, CV_OUT std::vector<Ptr<Region>> &regions);
};

}
}

#endif
