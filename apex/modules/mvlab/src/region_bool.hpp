#ifndef __OPENCV_MVLAB_REGION_BOOL_HPP__
#define __OPENCV_MVLAB_REGION_BOOL_HPP__

#include "region_impl.hpp"

namespace cv {
namespace mvlab {
struct RegionBoolOp
{
    static void GetRows(const RunSequence &srcRuns, const RowBeginSequence &rowBegs, UScalableIntSequence &rows);
};

struct RegionComplementOp : public RegionBoolOp
{
    RunSequence Do(const RunSequence &srcRuns, const RowBeginSequence &rowBegs, const cv::Rect &rcUniverse);
};

struct RegionDifferenceOp : public RegionBoolOp
{
    RunSequence Do(const RunSequence &srcRuns1, const RowBeginSequence &rowBegs1, const RunSequence &srcRuns2, const RowBeginSequence &rowBegs2);
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
