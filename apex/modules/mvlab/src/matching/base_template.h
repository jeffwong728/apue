#ifndef SPAM_UI_PROC_BASE_TEMPLATE_H
#define SPAM_UI_PROC_BASE_TEMPLATE_H

#include <region/region_impl.hpp>
#include <opencv2/mvlab/forward.hpp>
#include <opencv2/core.hpp>
#include <boost/optional.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <tbb/scalable_allocator.h>
#include <array>
#include <vector>

namespace cv {
namespace mvlab {

struct RowRange
{
    RowRange() : row(0), beg(0), end(0) {}
    RowRange(const int r, const int b, const int e) : row(r), beg(b), end(e) {}
    int row;
    int beg;
    int end;
};

struct BaseTmplCreateData
{
    BaseTmplCreateData() : angleStart(-30), angleExtent(60), pyramidLevel(5) {}
    cv::Mat img;
    cv::Ptr<Region> rgn;
    cv::Ptr<Region> roi;
    int angleStart;
    int angleExtent;
    int pyramidLevel;
};

struct DrawTmplOptions
{
    DrawTmplOptions()
        : drawTemplate(0)
        , drawRegion(1)
        , drawArrow(1)
        , styleTemplate(BOUNDARY_LINE_SOLID)
        , styleRegion(BOUNDARY_LINE_SOLID)
        , styleArrow(0)
        , thicknessTemplate(1.5f)
        , thicknessRegion(1.5f)
        , thicknessArrow(1.5f)
        , arrowSize(30.f)
        , arrowTip(0.2f)
        , colorTemplate(0, 0, 255, 255)
        , colorRegion(0, 255, 255, 255)
        , colorArrow(255, 255, 0, 255)
    {}
    int drawTemplate;
    int drawRegion;
    int drawArrow;
    int styleTemplate;
    int styleRegion;
    int styleArrow;
    float thicknessTemplate;
    float thicknessRegion;
    float thicknessArrow;
    float arrowSize;
    float arrowTip;
    cv::Scalar colorTemplate;
    cv::Scalar colorRegion;
    cv::Scalar colorArrow;
};


class ScalablePointSet : public ScalablePointSequence
{
public:
    ScalablePointSet() {}
    ScalablePointSet(const ScalablePointSequence &r) : ScalablePointSequence(r) {}
    ScalablePointSet(const cv::Ptr<Region> &rgn);
    ScalablePointSet(const cv::Ptr<Region> &rgn, const cv::Point &offset);
    std::pair<cv::Point, cv::Point> MinMax() const;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int /*version*/)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ScalablePointSequence);
    }

public:
    bool IsInsideImage(const cv::Size &imgSize) const;
};

using RowRangeList  = std::vector<RowRange>;
using VertexList    = boost::container::small_vector<int, 5>;
using AdjacencyList = std::vector<VertexList>;

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
        Candidate(const Candidate &other) = default;
        Candidate(Candidate &&other) = default;
        Candidate &operator=(const Candidate &other) = default;
        Candidate &operator=(Candidate &&other) = default;

        int row;
        int col;
        int mindex;
        int label;
        float score;
        ScalablePointSequence deforms;
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
    int verifyCreateData(const BaseTmplCreateData &createData);
    void destroyBaseData();
    void clearCacheMatchData();
    void supressNoneMaximum();
    void processToplayerSearchROI(const cv::Ptr<Region> &roi, const int pyramidLevel);
    float maxScoreInterpolate(const cv::Mat &scores, cv::Point2f &pos, float &angle) const;
    void moveCandidatesToLowerLayer(const int layer);
    void startMoveCandidatesToLowerLayer();
    std::tuple<const int, const RunLength *> getSearchRegion(const cv::Ptr<Region> &searchRgn, const int nTopRows, const int nTopCols);
    static void initMatchResult(cv::Point2f &pos, float &angle, float &score) { pos.x = 0.f; pos.y = 0.f; angle = 0.f; score = 0.f; }

protected:
    int pyramid_level_;
    int angle_start_;
    int angle_extent_;
    std::vector<cv::Point2f>        cfs_; // centre of referance
    std::vector<cv::Mat>            pyrs_;
    std::vector<const uint8_t *>    row_ptrs_;
    cv::Ptr<Region>                 top_layer_full_domain_;
    cv::Ptr<Region>                 top_layer_search_roi_;
    CandidateList                   candidates_;
    CandidateList                   top_candidates_;
    CandidateList                   final_candidates_;
    CandidateGroup                  candidate_runs_;
    CandidateGroupList              candidate_groups_;
    cv::Mat                         score_fitting_mask_;
    cv::Mat                         tmpl_img_;
    cv::Ptr<Region>                 tmpl_rgn_;
    cv::Ptr<Contour>                tmpl_cont_;
    mutable cv::String              err_msg_;
};
}
}
#endif //SPAM_UI_PROC_BASE_TEMPLATE_H