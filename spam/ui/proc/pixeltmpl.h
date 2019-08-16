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

using PixelValueSequence = boost::variant<std::vector<int16_t>, std::vector<float>>;
struct PixelTmplData
{
    PixelTmplData(const float a, const float s) : angle(a), scale(s) {}
    PixelValueSequence     pixlVals;
    PointSet               pixlLocs;
    std::vector<int>       belowCandidates;
    cv::Point              minPoint;
    cv::Point              maxPoint;
    float                  angle;
    float                  scale;
};

struct LayerTmplData
{
    LayerTmplData(const float as, const float ss) : angleStep(as), scaleStep(ss) {}
    float angleStep;
    float scaleStep;
    std::vector<PixelTmplData> tmplDatas;
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
        Candidate() : row(0), col(0), score(0.f), angle(0.f), scale(1.f) {}
        Candidate(const int r, const int c) : row(r), col(c), score(0.f), angle(0.f), scale(1.f) {}
        Candidate(const int r, const int c, const float s) : row(r), col(c), score(s), angle(0.f), scale(1.f) {}
        Candidate(const int r, const int c, const float s, const float a) : row(r), col(c), score(s), angle(a), scale(1.f) {}
        int row;
        int col;
        float score;
        float angle;
        float scale;
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

public:
    PixelTemplate();
    ~PixelTemplate();

public:
    SpamResult matchTemplate(const cv::Mat &img, const int sad, cv::Point2f &pos, float &angle);
    SpamResult CreatePixelTemplate(const PixelTmplCreateData &createData);
    const std::vector<LayerTmplData> &GetTmplDatas() const { return pyramid_tmpl_datas_; }
    cv::Mat GetTopScoreMat() const;

private:
    void destroyData();
    SpamResult verifyCreateData(const PixelTmplCreateData &createData);
    SpamResult calcCentreOfGravity(const PixelTmplCreateData &createData);
    void linkTemplatesBetweenLayers();
    static uint8_t getMinMaxGrayScale(const cv::Mat &img, const PointSet &maskPoints, const cv::Point &point);
    void supressNoneMaximum();

private:
    int pyramid_level_;
    cv::TemplateMatchModes match_mode_;
    std::vector<cv::Point2f> cfs_; // centre of referance
    std::vector<SpamRgn>     tmpl_rgns_;
    std::vector<SpamRgn>     search_rois_;
    std::vector<LayerTmplData> pyramid_tmpl_datas_;
    std::vector<cv::Mat> pyrs_;
    std::vector<const uint8_t *> row_ptrs_;
    CandidateList candidates_;
    CandidateGroup candidate_runs_;
    CandidateGroupList candidate_groups_;
};

#endif //SPAM_UI_PROC_PIXEL_TEMPLATE_H