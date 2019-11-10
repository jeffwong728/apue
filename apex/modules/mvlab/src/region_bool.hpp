#ifndef __OPENCV_MVLAB_REGION_BOOL_HPP__
#define __OPENCV_MVLAB_REGION_BOOL_HPP__

#include "region_impl.hpp"

namespace cv {
namespace mvlab {
struct RegionBoolOp
{
};

struct RegionComplementOp : public RegionBoolOp
{
    RunSequence Do(const RunSequence &srcRuns, const RowBeginSequence &rowBegs, const cv::Rect &rcUniverse);
};

struct RegionDifferenceOp : public RegionBoolOp
{
};

struct RegionIntersectionOp : public RegionBoolOp
{
};

struct RegionSymmDifferenceOp : public RegionBoolOp
{
};

struct RegionUnion2Op : public RegionBoolOp
{
};

}
}

#endif //__OPENCV_MVLAB_REGION_BOOL_HPP__
