#include "precomp.hpp"
#include "region_impl.hpp"

namespace cv {
namespace mvlab {

Ptr<Region> Region::CreateRectangle(const Rect &rectSize)
{
    return makePtr<RegionImpl>(rectSize);
}

}
}
