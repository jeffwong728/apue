#ifndef SPAM_UI_PROC_REGION_H
#define SPAM_UI_PROC_REGION_H
#include <vector>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#include <boost/container/small_vector.hpp>

struct SpamRun
{
    int l;  // line number (row) of run
    int cb; // column index of beginning(include) of run
    int ce; // column index of ending(exclude) of run
};

class SpamRgn;
using VertexList = boost::container::small_vector<int, 5>;
using AdjacencyList = std::vector<VertexList>;
using SpamRgnVector = std::vector<SpamRgn>;
using SPSpamRgn = std::shared_ptr<SpamRgn>;
using SPSpamRgnVector = std::shared_ptr<SpamRgnVector>;
using RgnBufferZone = std::unordered_map<std::string, SPSpamRgnVector>;

class SpamRgn
{

public:
    SpamRgn() : color_(0xFFFF0000) {}
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
    SPSpamRgnVector Connect() const;
    cv::Rect BoundingBox() const;
    bool Contain(const int r, const int c) const;
    AdjacencyList GetAdjacencyList() const;

private:
    std::vector<SpamRun> data_;
    uint32_t             color_;
};

inline bool IsRunColumnIntersection(const SpamRun &r1, const SpamRun &r2)
{
    return !(r1.ce < r2.cb || r2.ce < r1.cb);
}

#endif //SPAM_UI_PROC_REGION_H