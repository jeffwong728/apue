#ifndef __OPENCV_MVLAB_CONNECTION_HPP__
#define __OPENCV_MVLAB_CONNECTION_HPP__

#include <opencv2/mvlab/region.hpp>
#include <opencv2/mvlab/region_collection.hpp>
#include "region_impl.hpp"

namespace cv {
namespace mvlab {
using LabelT = int;
class ConnectWuSerial
{
public:
    ConnectWuSerial() {}

public:
    cv::Ptr<RegionCollection> Connect(const Region *rgn, const int connectivity) const;
    void Connect(const Region *rgn, const int connectivity, std::vector<Ptr<Region>> &regions) const;

private:
    void ConnectCommon(const Region *rgn, const int connectivity, ScalableIntSequence &numRunsOfRgn, RunPtrSequence &rowRunPtrs) const;
};

class ConnectWuParallel
{
    class FirstScan8Connectivity
    {
        LabelT *P_;
        int *chunksSizeAndLabels_;
        const cv::Rect &bbox_;
        const RowBeginSequence &rowRunBegs_;
        RunSequence &data_;

    public:
        FirstScan8Connectivity(LabelT *P, int *chunksSizeAndLabels, const cv::Rect &bbox, const RowBeginSequence &rowRunBegs, RunSequence &data);
        void operator()(const tbb::blocked_range<int>& br) const;
    };

public:
    ConnectWuParallel() {}

public:
    cv::Ptr<RegionCollection> Connect(const Region *rgn, const int connectivity) const;
    void Connect(const Region *rgn, const int connectivity, std::vector<Ptr<Region>> &regions) const;
    void mergeLabels8Connectivity(const RegionImpl &rgn, RunPtrSequence &rowRunPtrs, LabelT *P, const int *chunksSizeAndLabels) const;
    void mergeLabels4Connectivity(const RegionImpl &rgn, LabelT *P, const int *chunksSizeAndLabels) const;

private:
    void ConnectCommon(const Region *rgn, const int connectivity, ScalableIntSequence &numRunsOfRgn, RunPtrSequence &rowRunPtrs) const;
};

}
}

#endif //__OPENCV_MVLAB_CONNECTION_HPP__
