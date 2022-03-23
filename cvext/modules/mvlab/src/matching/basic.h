#ifndef SPAM_UI_PROC_BASIC_H
#define SPAM_UI_PROC_BASIC_H

#include <vector>
#include <forward_list>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#include <boost/core/noncopyable.hpp>
#include <tbb/scalable_allocator.h>

class BasicImgProc : private boost::noncopyable
{
public:
    BasicImgProc() = delete;

public:
    static cv::Mat AlignImageWidth(const cv::Mat &img);
    static cv::Mat Binarize(const cv::Mat &grayImage, const uchar lowerGray, const uchar upperGray);
    static cv::Mat PathToMask(const Geom::PathVector &pv, const cv::Size &sz);
    static cv::Mat PathToMask(const Geom::PathVector &pv, const cv::Size &sz, std::vector<uint8_t> &buf);
    static void Transform(const cv::Mat &grayImage, cv::Mat &dst, const cv::Mat &transMat, const cv::Rect &mask);
    static void TrackCurves(const ScalablePointSequence &points, const cv::Point &minPoint, const cv::Point &maxPoint, std::vector<std::vector<int>> &curves, std::vector<bool> &closed);
    static void SplitCurvesToSegments(const std::vector<bool> &closed, std::vector<std::vector<int>> &curves);

public:
    static int16_t getGrayScaleSubpix(const cv::Mat& grayImage, const cv::Point2f &pt);
};

inline int16_t BasicImgProc::getGrayScaleSubpix(const cv::Mat& img, const cv::Point2f &pt)
{
    cv::Point tlPt{ cvFloor(pt.x), cvFloor(pt.y) };
    cv::Point trPt{ tlPt.x + 1, tlPt.y };
    cv::Point blPt{ tlPt.x, tlPt.y + 1 };
    cv::Point brPt{ tlPt.x + 1, tlPt.y + 1 };

    cv::Rect imageBox(0, 0, img.cols, img.rows);
    if (imageBox.contains(tlPt) && imageBox.contains(brPt))
    {
        float rx = pt.x - tlPt.x;
        float tx = 1 - rx;
        float ry = pt.y - tlPt.y;
        float ty = 1 - ry;

        return static_cast<int16_t>(cvRound(img.at<uint8_t>(tlPt)*tx*ty + img.at<uint8_t>(trPt)*rx*ty + img.at<uint8_t>(blPt)*tx*ry + img.at<uint8_t>(brPt)*rx*ry));
    }
    else
    {
        return -1;
    }
}

#endif //SPAM_UI_PROC_BASIC_H