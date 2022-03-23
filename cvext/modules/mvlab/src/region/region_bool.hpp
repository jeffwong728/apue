#ifndef __OPENCV_MVLAB_REGION_BOOL_HPP__
#define __OPENCV_MVLAB_REGION_BOOL_HPP__

#include "region_impl.hpp"

namespace cv {
namespace mvlab {
struct RegionBoolOp
{
    static void GetRows(const RunSequence &srcRuns, const RowBeginSequence &rowBegs, UScalableIntSequence &rows);
    static int  IntersectRows(UScalableIntSequence &rows1, UScalableIntSequence &rows2);
};

struct RegionComplementOp : public RegionBoolOp
{
    RunSequence Do(const RunSequence &srcRuns, const cv::Rect &rcUniverse);
    RunSequence Do1(const RunSequence &srcRuns, const cv::Point &tlPoint, const cv::Point &brPoint);
    RunSequence Do2(const RunSequence &srcRuns);
};

struct RegionDifferenceOp : public RegionBoolOp
{
    RunSequence Do(const RunSequence &srcRuns1, const RunSequence &srcRuns2);
};

struct RegionIntersectionOp : public RegionBoolOp
{
    RunSequence Do(const RunSequence &srcRuns1, const RunSequence &srcRuns2);
};

struct RegionSymmDifferenceOp : public RegionBoolOp
{
    RunSequence Do(const RunSequence &srcRuns1, const RunSequence &srcRuns2);
};

struct RegionUnion2Op : public RegionBoolOp
{
    void Do(const RunSequence &srcRuns1, const RunSequence &srcRuns2, RunSequence &dstRuns);
};

}
}

#endif //__OPENCV_MVLAB_REGION_BOOL_HPP__
