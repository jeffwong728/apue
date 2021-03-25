#include "precomp.hpp"
#include "opencv2/mvlab.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <map>

namespace cv {
namespace mvlab {

static std::map<cv::String, cv::String> g_GlobalOptions;

FitLineParameters::FitLineParameters() 
    : algorithm("tukey")
    , maxNumPoints(-1)
    , clippingEndPoints(0)
    , iterations(5)
    , clippingFactor(2.0) 
{}

FitLineResults::FitLineResults() 
    : xBegin(0)
    , yBegin(0)
    , xEnd(0)
    , yEnd(0)
    , nx(0)
    , ny(0)
    , dist(0) 
{}

int Initialize(const cv::String& /*fileName*/)
{
    int inset = vcl::instrset_detect();
    if (inset < 8)
    {
        return MLR_CPU_UNSUPPORTED;
    }

    return MLR_SUCCESS;
}

int SetGlobalOption(const cv::String& optName, const cv::String& optVal)
{
    g_GlobalOptions[optName] = optVal;
    return 0;
}

int GetGlobalOption(const cv::String& optName, cv::String& optVal)
{
    const auto it = g_GlobalOptions.find(optName);
    if (it != g_GlobalOptions.cend())
    {
        optVal = it->second;
        return 0;
    }

    return -1;
}

cv::Rect2f BoundingBox(const std::vector<cv::Point2f> &points)
{
    if (points.empty())
    {
        return cv::Rect2f();
    }

    const int numPoints = static_cast<int>(points.size());
    constexpr int simdSize = 8;
    const int regularNumPoints = numPoints & (-simdSize);

    vcl::Vec8f top(std::numeric_limits<float>::max());
    vcl::Vec8f left(std::numeric_limits<float>::max());
    vcl::Vec8f bot(std::numeric_limits<float>::lowest());
    vcl::Vec8f right(std::numeric_limits<float>::lowest());

    int n = 0;
    const cv::Point2f *pt = points.data();
    for (; n < regularNumPoints; n += simdSize)
    {
        vcl::Vec8f v1, v2;
        v1.load(reinterpret_cast<const float *>(pt));
        v2.load(reinterpret_cast<const float *>(pt + simdSize / 2));
        vcl::Vec8f x = vcl::blend8<0, 2, 4, 6, 8, 10, 12, 14>(v1, v2);
        vcl::Vec8f y = vcl::blend8<1, 3, 5, 7, 9, 11, 13, 15>(v1, v2);

        top = vcl::min(top, y);
        left = vcl::min(left, x);
        bot = vcl::max(bot, y);
        right = vcl::max(right, x);

        pt += simdSize;
    }

    float xmin = vcl::horizontal_min(left);
    float xmax = vcl::horizontal_max(right);
    float ymin = vcl::horizontal_min(top);
    float ymax = vcl::horizontal_max(bot);
    for (; n < numPoints; ++n, ++pt)
    {
        xmin = std::min(xmin, pt->x);
        ymin = std::min(ymin, pt->y);
        xmax = std::max(xmax, pt->x);
        ymax = std::max(ymax, pt->y);
    }

    return cv::Rect2f(xmin, ymin, xmax - xmin, ymax - ymin);
}

}
}
