#ifndef SPAM_UI_PROC_BASIC_H
#define SPAM_UI_PROC_BASIC_H
#include "rgn.h"
#include <vector>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#include <boost/core/noncopyable.hpp>

class BasicImgProc : private boost::noncopyable
{

public:
    BasicImgProc() = delete;

public:
    static SPSpamRgn Threshold(const cv::Mat &grayImage, const uchar lowerGray, const uchar upperGray);
};
#endif //SPAM_UI_PROC_BASIC_H