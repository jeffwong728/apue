#ifndef SPAM_UI_PROC_BASE_TEMPLATE_H
#define SPAM_UI_PROC_BASE_TEMPLATE_H
#include "rgn.h"
#include <ui/errdef.h>
#include <array>
#include <vector>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/affine.hpp>
#include <boost/optional.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/variant.hpp>
#include <tbb/scalable_allocator.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 4267 4244)
#include <2geom/path.h>
#include <2geom/pathvector.h>
#pragma warning( pop )

template<typename TAngle>
struct AngleRange
{
    AngleRange(const TAngle s, const TAngle e)
        : start(normalize(s)), end(normalize(e))
    {
    }

    TAngle normalize(const TAngle a)
    {
        TAngle angle = a;
        while (angle < -180) angle += 360;
        while (angle > 180) angle -= 360;
        return angle;
    }

    bool contains(const TAngle a)
    {
        TAngle na = normalize(a);
        if (start < end) {
            return !(na > end || na < start);
        }
        else {
            return !(na > end && na < start);
        }
    }

    bool between(const TAngle a)
    {
        TAngle na = normalize(a);
        if (start < end) {
            return na < end && na > start;
        }
        else {
            return na < end || na > start;
        }
    }

    TAngle start;
    TAngle end;
};

struct OutsideImageBox
{
    OutsideImageBox(const int w, const int h) : width(w), height(h) {}
    bool operator()(const cv::Point &point)
    {
        if (point.x < 0 || point.x >= width || point.y < 0 || point.y >= height)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    const int width;
    const int height;
};

struct OutsideRectangle
{
    OutsideRectangle(const int l, const int r, const int t, const int b) : left(l), right(r), top(t), bottom(b) {}
    bool operator()(const cv::Point &point)
    {
        if (point.x < left || point.x > right || point.y < top || point.y > bottom)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    const int left;
    const int right;
    const int top;
    const int bottom;
};

struct BaseTmplCreateData
{
    const cv::Mat &srcImg;
    const Geom::PathVector &tmplRgn;
    const Geom::PathVector &roi;
    const int angleStart;
    const int angleExtent;
    const int pyramidLevel;
};

class BaseTemplate
{
public:
    class Candidate
    {
    public:
        Candidate() : row(0), col(0), score(0.f), mindex(-1) {}
        Candidate(const int r, const int c) : row(r), col(c), score(0.f), mindex(-1), label(0) {}
        Candidate(const int r, const int c, const float s) : row(r), col(c), score(s), mindex(-1), label(0) {}
        Candidate(const int r, const int c, const int m, const float s) : row(r), col(c), mindex(m), score(s), label(0) {}
        Candidate(const int r, const int c, const int m, const int l) : row(r), col(c), mindex(m), score(0.f), label(l) {}
        int row;
        int col;
        int mindex;
        int label;
        float score;
    };

    class CandidateGroup;
    using CandidateList = std::vector<Candidate, tbb::scalable_allocator<Candidate>>;
    using CandidateGroupList = std::vector<CandidateGroup>;

    class CandidateRun
    {
    public:
        CandidateRun() : row(0), colb(0), cole(0) {}
        CandidateRun(const int r, const int cb, const int ce) : row(r), colb(cb), cole(ce) {}
        bool IsColumnIntersection(const CandidateRun &r) const { return !(cole < r.colb || r.cole < colb); }
        int row;
        int colb;
        int cole;
        Candidate best;
    };

    class CandidateGroup : public std::vector<CandidateRun>
    {
    public:
        CandidateGroup() {}
        CandidateGroup(CandidateList &candidates);
        void RLEncodeCandidates(CandidateList &candidates);
        void Connect(CandidateGroupList &candidateGroups);
        RowRangeList row_ranges;
        AdjacencyList adjacency_list;
        std::vector<int> run_stack;
        std::vector<std::vector<int>> rgn_idxs;
    };

protected:
    BaseTemplate();
    ~BaseTemplate();

public:
    cv::Point2f GetCenter() const { return cfs_.front(); }

protected:
    SpamResult verifyCreateData(const BaseTmplCreateData &createData);
    void destroyBaseData();
    void clearCacheMatchData();
    void supressNoneMaximum();
    void processToplayerSearchROI(const Geom::PathVector &roi, const int pyramidLevel);

protected:
    int pyramid_level_;
    std::vector<cv::Point2f> cfs_; // centre of referance
    std::vector<SpamRgn>     tmpl_rgns_;
    std::vector<cv::Mat>     pyrs_;
    std::vector<const uint8_t *> row_ptrs_;
    SpamRgn            top_layer_full_domain_;
    SpamRgn            top_layer_search_roi_;
    Geom::PathVector   top_layer_search_roi_g_;
    CandidateList      candidates_;
    CandidateList      top_candidates_;
    CandidateList      final_candidates_;
    CandidateGroup     candidate_runs_;
    CandidateGroupList candidate_groups_;
};
#endif //SPAM_UI_PROC_BASE_TEMPLATE_H