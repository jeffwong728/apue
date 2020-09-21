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

}
}
