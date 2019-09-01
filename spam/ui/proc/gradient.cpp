#include "gradient.h"
#include <memory>
#include <vectorclass/vectorclass.h>
#include <boost/container/static_vector.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 4267 )
#include <2geom/2geom.h>
#include <2geom/path-intersection.h>
#include <2geom/cairo-path-sink.h>
#pragma warning( pop )
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#include <tbb/parallel_invoke.h>

struct SimpleGradient
{
    SimpleGradient(const cv::Mat *const i, cv::Mat *const x, cv::Mat *const y, const int cBeg, const int cEnd)
    : img(i), dx(x), dy(y), colBeg(cBeg), colEnd(cEnd) {}

    void operator()() const;
    const int colBeg;
    const int colEnd;
    cv::Mat *const dx;
    cv::Mat *const dy;
    const cv::Mat *const img;
};

void SimpleGradient::operator()() const
{

}

void SpamGradient::Simple(const cv::Mat &img, cv::Mat &dx, cv::Mat &dy, const cv::Rect &roi)
{
    dx.create(img.rows, img.cols, CV_16SC1);
    dy.create(img.rows, img.cols, CV_16SC1);
    dx = 0;
    dy = 0;

    if (img.rows<3 || img.cols<3)
    {
        return;
    }

    const int rowBeg = std::max(1, roi.y);
    const int colBeg = std::max(1, roi.x);
    const int rowEnd = std::min(img.rows-1, roi.y + roi.height);
    const int colEnd = std::min(img.cols-1, roi.x + roi.width);
}

void SpamGradient::SimpleNormalize(const cv::Mat &img, cv::Mat &dx, cv::Mat &dy, const cv::Rect &roi)
{
}

void SpamGradient::Sobel(const cv::Mat &img, cv::Mat &dx, cv::Mat &dy, const cv::Rect &roi)
{
}

void SpamGradient::SobelNormalize(const cv::Mat &img, cv::Mat &dx, cv::Mat &dy, const cv::Rect &roi)
{
}