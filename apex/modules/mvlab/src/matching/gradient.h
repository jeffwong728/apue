#ifndef SPAM_UI_PROC_GRADIENT_H
#define SPAM_UI_PROC_GRADIENT_H
#include <vector>
#include <forward_list>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#include <boost/core/noncopyable.hpp>
#include <tbb/scalable_allocator.h>

class SpamGradient : private boost::noncopyable
{
public:
    SpamGradient() = delete;

public:
    static void Simple(const cv::Mat &img, cv::Mat &dx, cv::Mat &dy, const cv::Rect &roi);
    static void SimpleNormalize(const cv::Mat &img, cv::Mat &dx, cv::Mat &dy, const cv::Rect &roi);
    static void Sobel(const cv::Mat &img, cv::Mat &dx, cv::Mat &dy, const cv::Rect &roi);
    static void SobelNormalize(const cv::Mat &img, cv::Mat &dx, cv::Mat &dy, const cv::Rect &roi, const int minContrast);
};

#endif //SPAM_UI_PROC_BASIC_H