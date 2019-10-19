#include "precomp.hpp"
#include "utility.hpp"
#include "region_impl.hpp"

namespace cv {
namespace mvlab {
int Threshold(InputArray src, const int minGray, const int maxGray, Ptr<Region> &region)
{
    region = makePtr<RegionImpl>();

    return MLR_SUCCESS;
}

}
}
