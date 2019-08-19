#ifndef SPAM_UI_PROC_PIXEL_TEMPLATE_H
#define SPAM_UI_PROC_PIXEL_TEMPLATE_H
#include "rgn.h"
#include <ui/errdef.h>
#include <array>
#include <vector>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
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

using PixelValueSequence = std::vector<int16_t>;
using NCCValueSequence = std::vector<uint8_t>;
using Point3iSet = std::vector<cv::Point3i>;
using NCCValuePairSequence = std::vector<std::pair<uint8_t, uint8_t>>;
using PointPairSet = std::vector<std::pair<cv::Point, cv::Point>>;

enum TemplPart
{
    kTP_WaveBack = 0,
    kTP_WaveMiddle = 1,
    kTP_WaveFront = 2,
    kTP_WaveGuard = 3
};

struct PixelTemplData
{
    PixelTemplData(const float a, const float s) : angle(a), scale(s) {}
    PixelValueSequence pixlVals;
    PointSet           pixlLocs;
    std::vector<int>   mindices;
    cv::Point          minPoint;
    cv::Point          maxPoint;
    float              angle;
    float              scale;
};

struct NCCTemplData
{
    NCCTemplData(const float a, const float s) : angle(a), scale(s) {}
    Point3iSet           partialLocs;
    NCCValueSequence     partialVals;
    Point3iSet           residualLocs;
    NCCValueSequence     residualVals;
    PointPairSet         residualBoundaries;
    NCCValuePairSequence residualBoundaryVals;
    std::vector<int>     mindices;
    cv::Point            minPoint;
    cv::Point            maxPoint;
    int                  cPartialBack;
    int                  cPartialFront;
    float                angle;
    float                scale;
};

using PixelTemplDatas = std::vector<PixelTemplData>;
using NCCTemplDatas = std::vector<NCCTemplData>;
using TemplDataSequence = boost::variant<PixelTemplDatas, NCCTemplDatas>;
struct LayerTemplData
{
    LayerTemplData(const float as, const float ss) : angleStep(as), scaleStep(ss) {}
    float angleStep;
    float scaleStep;
    TemplDataSequence tmplDatas;
};

struct PixelTmplCreateData
{
    const cv::Mat &srcImg;
    const Geom::PathVector &tmplRgn;
    const Geom::PathVector &roi;
    const int angleStart;
    const int angleExtent;
    const int pyramidLevel;
    const cv::TemplateMatchModes matchMode;
};

struct AngleRange
{
    AngleRange(const float s, const float e)
        : start(normalize(s)), end(normalize(e))
    {
    }

    float normalize(const float a)
    {
        float angle = a;
        while (angle < -180) angle += 360;
        while (angle > 180) angle -= 360;
        return angle;
    }

    bool contains(const float a)
    {
        float na = normalize(a);
        if (start < end)
        {
            return !(na > end || na < start);
        }
        else
        {
            return !(na > end && na < start);
        }
    }

    float start;
    float end;
};

class PixelTemplate
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

    friend struct SADTopLayerScaner;
    friend struct SADCandidateScaner;

public:
    PixelTemplate();
    ~PixelTemplate();

public:
    SpamResult matchTemplate(const cv::Mat &img, const int sad, cv::Point2f &pos, float &angle);
    SpamResult CreatePixelTemplate(const PixelTmplCreateData &createData);
    const std::vector<LayerTemplData> &GetTmplDatas() const { return pyramid_tmpl_datas_; }
    cv::Mat GetTopScoreMat() const;
    cv::Point2f GetCenter() const { return cfs_.front(); }

private:
    void destroyData();
    void clearCacheMatchData();
    SpamResult verifyCreateData(const PixelTmplCreateData &createData);
    SpamResult calcCentreOfGravity(const PixelTmplCreateData &createData);
    void linkTemplatesBetweenLayers();
    static uint8_t getMinMaxGrayScale(const cv::Mat &img, const PointSet &maskPoints, const cv::Point &point);
    void supressNoneMaximum();
    void changeToNCCTemplate();

private:
    int pyramid_level_;
    cv::TemplateMatchModes match_mode_;
    std::vector<cv::Point2f> cfs_; // centre of referance
    std::vector<SpamRgn>     tmpl_rgns_;
    std::vector<SpamRgn>     search_rois_;
    std::vector<LayerTemplData> pyramid_tmpl_datas_;
    std::vector<cv::Mat> pyrs_;
    std::vector<const uint8_t *> row_ptrs_;
    CandidateList candidates_;
    CandidateList top_candidates_;
    CandidateList final_candidates_;
    CandidateGroup candidate_runs_;
    CandidateGroupList candidate_groups_;
};

#endif //SPAM_UI_PROC_PIXEL_TEMPLATE_H