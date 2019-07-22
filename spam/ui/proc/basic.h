#ifndef SPAM_UI_PROC_BASIC_H
#define SPAM_UI_PROC_BASIC_H
#include "rgn.h"
#include <vector>
#include <forward_list>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#include <boost/core/noncopyable.hpp>
#include <tbb/scalable_allocator.h>

using SpamRunListTBB = std::vector<SpamRun, tbb::scalable_allocator<SpamRun>>;
using SpamRunListPool = std::list<SpamRunListTBB, tbb::scalable_allocator<SpamRunListTBB>>;
using UPSpamRunListTBB = std::unique_ptr<SpamRunListTBB>;
using SpamRunListList = std::vector<std::unique_ptr<SpamRunListTBB>, tbb::scalable_allocator<std::unique_ptr<SpamRunListTBB>>>;

class BasicImgProc : private boost::noncopyable
{

public:
    BasicImgProc() = delete;

public:
    static void Initialize(const int numWorkerThread);
    static void ReturnRegion(SpamRunList &&rgn);

public:
    static SPSpamRgn Threshold(const cv::Mat &grayImage, const uchar lowerGray, const uchar upperGray);
    static SPSpamRgnVector Connect(const cv::Mat &labels, const int numLabels);

private:
    static std::vector<SpamRunListPool> s_runList_pools_;
    static std::vector<SpamRunList> s_rgn_pool_;
};
#endif //SPAM_UI_PROC_BASIC_H