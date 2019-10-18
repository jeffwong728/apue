#include "precomp.hpp"
#include "opencv2/mvlab.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <map>

namespace cv {
namespace mvlab {

static std::map<cv::String, cv::String> g_GlobalOptions;

int SetGlobalOption(const String& optName, const String& optVal)
{
    g_GlobalOptions[optName] = optVal;
    return 0;
}

int GetGlobalOption(const String& optName, String& optVal)
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
