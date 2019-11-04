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
    cv::Ptr<RegionCollection> operator()(const Region *rgn, const int connectivity) const;
};

class ConnectWuParallel
{
    class FirstScan8Connectivity
    {
        LabelT *P_;
        int *chunksSizeAndLabels_;
        const cv::Rect &bbox_;
        const RowRunStartList &rowRunBegs_;
        RunList &data_;

    public:
        FirstScan8Connectivity(LabelT *P, int *chunksSizeAndLabels, const cv::Rect &bbox, const RowRunStartList &rowRunBegs, RunList &data);
        void operator()(const tbb::blocked_range<int>& br) const;
    };

public:
    ConnectWuParallel() {}

public:
    cv::Ptr<RegionCollection> operator() (const Region *rgn, const int connectivity);
    void mergeLabels8Connectivity(const RegionImpl &rgn, std::vector<RunLength *> &rowRunPtrs, LabelT *P, const int *chunksSizeAndLabels);
    void mergeLabels4Connectivity(const RegionImpl &rgn, LabelT *P, const int *chunksSizeAndLabels);
};

}
}

#endif //__OPENCV_MVLAB_CONNECTION_HPP__
