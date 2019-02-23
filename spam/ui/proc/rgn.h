#ifndef SPAM_UI_PROC_REGION_H
#define SPAM_UI_PROC_REGION_H
#include <vector>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>

struct SpamRun
{
    int l;  // line number (row) of run
    int cb; // column index of beginning(include) of run
    int ce; // column index of ending(exclude) of run
};

class SpamRgn
{

public:
    SpamRgn() {}
    ~SpamRgn() {}

public:
    void swap(SpamRgn &o) { data_.swap(o.data_); }
    void clear() { data_.clear(); }

public:
    void AddRun(const int l, const int cb, const int ce) { data_.push_back({ l, cb, ce }); }
    void AddRun(const cv::Mat &binaryImage);
    void AddRunParallel(const cv::Mat &binaryImage);
    void Draw(const cv::Mat &dstImage, const double sx, const double sy) const;

public:
    double Area() const;
    void Connect() const;
    cv::Rect BoundingBox() const;
    bool Contain(const int r, const int c) const;

private:
    std::vector<SpamRun> data_;
};

using SpamRgnVector   = std::vector<SpamRgn>;
using SPSpamRgn       = std::shared_ptr<SpamRgn>;
using SPSpamRgnVector = std::shared_ptr<SpamRgnVector>;
using RgnBufferZone   = std::unordered_map<std::string, SPSpamRgnVector>;

#endif //SPAM_UI_PROC_REGION_H