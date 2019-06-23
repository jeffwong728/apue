#ifndef SPAM_UI_PROC_REGION_H
#define SPAM_UI_PROC_REGION_H
#include <array>
#include <vector>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#include <boost/container/small_vector.hpp>

struct SpamRun
{
    SpamRun() : l(0), cb(0), ce(0) {}
    SpamRun(const int ll, const int bb, const int ee) : l(ll), cb(bb), ce(ee) {}
    int l;  // line number (row) of run
    int cb; // column index of beginning(include) of run
    int ce; // column index of ending(exclude) of run
};

struct RD_LIST_ENTRY
{
    RD_LIST_ENTRY(const int x, const int y, const int code, const int qi) : X(x), Y(y), CODE(code), LINK(0), W_LINK(0), QI(qi), FLAG(0) {}
    int X;
    int Y;
    int CODE;
    int LINK;
    int W_LINK;
    int QI;
    int FLAG;
};

class SpamRgn;
using VertexList = boost::container::small_vector<int, 5>;
using AdjacencyList = std::vector<VertexList>;
using SpamRgnVector = std::vector<SpamRgn>;
using SPSpamRgn = std::shared_ptr<SpamRgn>;
using SPSpamRgnVector = std::shared_ptr<SpamRgnVector>;
using RgnBufferZone = std::unordered_map<std::string, SPSpamRgnVector>;
using RD_LIST = std::vector<RD_LIST_ENTRY>;
using RD_CONTOUR = std::vector<cv::Point>;

class SpamRgn
{
    friend class RunTypeDirectionEncoder;
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
    int GetNumRuns() const { return static_cast<int>(data_.size()); }

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

class RunTypeDirectionEncoder
{
public:
    RunTypeDirectionEncoder(SpamRgn &rgn) : rgn_(rgn) {}

public:
    RD_LIST encode() const;
    RD_CONTOUR track(std::vector<RD_CONTOUR> &holes) const;

private:
    SpamRgn &rgn_;
    const int qis_[11]{ 0, 2, 1, 1, 1, 0, 1, 1, 1, 2, 0 };
    const int count_[11]{1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1};
    const int downLink_[11][11]{ {0}, {0}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0}, {0}, {0}, {0}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1} };
    const int upLink_[11][11]{ {0}, {0}, {0}, {0}, {0}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1} };
};

inline bool IsRunColumnIntersection(const SpamRun &r1, const SpamRun &r2)
{
    return !(r1.ce < r2.cb || r2.ce < r1.cb);
}

#endif //SPAM_UI_PROC_REGION_H