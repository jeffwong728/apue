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
    void AddRun(const int l, const int cb, const int ce) { data_.push_back({ l, cb, ce }); }
    void AddRun(const cv::Mat &binaryImage);
    void AddRunParallel(const cv::Mat &binaryImage);

public:
    double Area() const;
    void Connect() const;

private:
    std::vector<SpamRun> data_;
};
#endif //SPAM_UI_PROC_REGION_H