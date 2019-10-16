#include "precomp.hpp"
#include "opencv2/mvlab.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace cv {
namespace mvlab {

int SetGlobalOption(const String& optName, const String& optVal)
{
    return 0;
}

int GetGlobalOption(const String& optName, String& optVal)
{
    optVal = "";
    return 0;
}

}
}
