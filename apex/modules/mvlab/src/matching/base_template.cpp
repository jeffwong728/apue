#include "precomp.hpp"
#include "base_template.h"
#include "basic.h"
#include <opencv2/mvlab/cmndef.hpp>
#include <opencv2/mvlab/region.hpp>

namespace cv {
namespace mvlab {

ScalablePointSet::ScalablePointSet(const cv::Ptr<Region> &rgn)
{
    cv::Ptr<RegionImpl> rgnImpl = rgn.dynamicCast<RegionImpl>();
    for (const RunLength &run : rgnImpl->GetAllRuns())
    {
        for (int16_t col = run.colb; col < run.cole; ++col)
        {
            emplace_back(col, run.row);
        }
    }
}

ScalablePointSet::ScalablePointSet(const cv::Ptr<Region> &rgn, const cv::Point &offset)
{
    cv::Ptr<RegionImpl> rgnImpl = rgn.dynamicCast<RegionImpl>();
    for (const RunLength &run : rgnImpl->GetAllRuns())
    {
        for (int16_t col = run.colb; col < run.cole; ++col)
        {
            emplace_back(col + offset.x, run.row + offset.y);
        }
    }
}

std::pair<cv::Point, cv::Point> ScalablePointSet::MinMax() const
{
    cv::Point minPoint{ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
    cv::Point maxPoint{ std::numeric_limits<int>::min(), std::numeric_limits<int>::min() };

    for (const cv::Point &pt : *this)
    {
        if (pt.y < minPoint.y) {
            minPoint.y = pt.y;
        }

        if (pt.y > maxPoint.y) {
            maxPoint.y = pt.y;
        }

        if (pt.x < minPoint.x) {
            minPoint.x = pt.x;
        }

        if (pt.x > maxPoint.x) {
            maxPoint.x = pt.x;
        }
    }

    if (empty()) {
        return std::make_pair(cv::Point(), cv::Point());
    }
    else {
        return std::make_pair(minPoint, maxPoint);
    }
}

bool ScalablePointSet::IsInsideImage(const cv::Size &imgSize) const
{
    for (const cv::Point &point : *this)
    {
        if (point.x < 0 || point.x >= imgSize.width || point.y < 0 || point.y >= imgSize.height)
        {
            return false;
        }
    }

    return true;
}

BaseTemplate::BaseTemplate()
    : pyramid_level_(4)
    , angle_start_(-30)
    , angle_extent_(60)
{
    int i = 0;
    cv::Mat A(27, 10, CV_64FC1);
    for (int r = -1; r < 2; ++r)
    {
        for (int c = -1; c < 2; ++c)
        {
            for (int a = -1; a < 2; ++a)
            {
                double *R = A.ptr<double>(i++);
                R[0] = 1;
                R[1] = r; R[2] = c; R[3] = a;
                R[4] = r * r; R[5] = c * c; R[6] = a * a;
                R[7] = r * c; R[8] = c * a; R[9] = r * a;
            }
        }
    }

    cv::Mat TA, IA;
    cv::transpose(A, TA);
    cv::invert(TA * A, IA);
    score_fitting_mask_ = IA * TA;
}

BaseTemplate::~BaseTemplate()
{ 
}

BaseTemplate::CandidateGroup::CandidateGroup(CandidateList &candidates)
{
    RLEncodeCandidates(candidates);
}

void BaseTemplate::CandidateGroup::RLEncodeCandidates(CandidateList &candidates)
{
    resize(0);
    std::sort(candidates.begin(), candidates.end(), [](const Candidate &l, const Candidate &r)
    {
        return (l.row < r.row) || (l.row == r.row && l.col < r.col);
    });

    if (!candidates.empty())
    {
        int colb = candidates.front().col;
        int preRow = candidates.front().row;
        int preCol = candidates.front().col - 1;
        Candidate bestCandidate = candidates.front();
        for (const Candidate &candidate : candidates)
        {
            if (candidate.row != preRow)
            {
                emplace_back(preRow, colb, preCol + 1).best = bestCandidate;
                colb = candidate.col;
                bestCandidate = candidate;
            }
            else
            {
                if (candidate.col != (preCol + 1))
                {
                    emplace_back(preRow, colb, preCol + 1).best = bestCandidate;
                    colb = candidate.col;
                    bestCandidate = candidate;
                }
            }

            preRow = candidate.row;
            preCol = candidate.col;

            if (candidate.score > bestCandidate.score)
            {
                bestCandidate = candidate;
            }
        }

        emplace_back(preRow, colb, preCol + 1).best = bestCandidate;
    }
}

void BaseTemplate::CandidateGroup::Connect(CandidateGroupList &candidateGroups)
{
    row_ranges.resize(0);
    adjacency_list.resize(0);
    adjacency_list.resize(size());
    candidateGroups.resize(0);

    if (!empty())
    {
        int begIdx = 0;
        int16_t currentRow = front().row;
        row_ranges.reserve(size());

        const CandidateRun *runs = data();
        const int numRuns = static_cast<int>(size());
        for (int run = 0; run < numRuns; ++run)
        {
            if (runs[run].row != currentRow)
            {
                row_ranges.emplace_back(currentRow, begIdx, run);
                begIdx = run;
                currentRow = runs[run].row;
            }
        }
        row_ranges.emplace_back(currentRow, begIdx, numRuns);

        const int endRow = static_cast<int>(row_ranges.size() - 1);
        for (auto row = 0; row < endRow; ++row)
        {
            const RowRange &rowRange = row_ranges[row];
            const RowRange &nextRowRange = row_ranges[row + 1];

            if ((rowRange.row + 1) == nextRowRange.row)
            {
                int nextBegIdx = nextRowRange.beg;
                const int nextEndIdx = nextRowRange.end;
                for (int runIdx = rowRange.beg; runIdx != rowRange.end; ++runIdx)
                {
                    bool metInter = false;
                    for (int nextRunIdx = nextBegIdx; nextRunIdx != nextEndIdx; ++nextRunIdx)
                    {
                        if (runs[runIdx].IsColumnIntersection(runs[nextRunIdx]))
                        {
                            metInter = true;
                            nextBegIdx = nextRunIdx;
                            adjacency_list[runIdx].push_back(nextRunIdx);
                            adjacency_list[nextRunIdx].push_back(runIdx);
                        }
                        else if (metInter)
                        {
                            break;
                        }
                    }
                }
            }
        }

        run_stack.resize(0);
        rgn_idxs.resize(0);
        std::vector<uint8_t> met(adjacency_list.size());
        for (int n = 0; n < numRuns; ++n)
        {
            if (!met[n])
            {
                rgn_idxs.emplace_back();
                run_stack.push_back(n);
                while (!run_stack.empty())
                {
                    const int seedRun = run_stack.back();
                    run_stack.pop_back();
                    if (!met[seedRun])
                    {
                        met[seedRun] = 1;
                        rgn_idxs.back().push_back(seedRun);
                        for (const int a : adjacency_list[seedRun])
                        {
                            run_stack.push_back(a);
                        }
                    }
                }

                std::sort(rgn_idxs.back().begin(), rgn_idxs.back().end());
            }
        }

        candidateGroups.resize(rgn_idxs.size());
        const int numRgns = static_cast<int>(rgn_idxs.size());
        for (int r = 0; r < numRgns; ++r)
        {
            CandidateGroup &rgn = candidateGroups[r];
            const std::vector<int> &rgnIdx = rgn_idxs[r];
            const int numRgnRuns = static_cast<int>(rgnIdx.size());
            rgn.resize(numRgnRuns);
            for (int rr = 0; rr < numRgnRuns; ++rr)
            {
                rgn[rr] = runs[rgnIdx[rr]];
            }
        }
    }
}

int BaseTemplate::verifyCreateData(const BaseTmplCreateData &createData)
{
    if (createData.pyramidLevel < 1)
    {
        err_msg_ = "invalid pyramid level";
        return MLR_TEMPLATE_PYRAMID_LEVEL_INVALID;
    }

    if (createData.img.empty())
    {
        err_msg_ = "empty image";
        return MLR_IMAGE_EMPTY;
    }

    int topWidth = createData.img.cols;
    int topHeight = createData.img.rows;
    for (int n = 0; n < (createData.pyramidLevel - 1); ++n)
    {
        topWidth /= 2;
        topHeight /= 2;
    }

    if (0 == topWidth || 0 == topHeight)
    {
        err_msg_ = "pyramid level too large";
        return MLR_TEMPLATE_PYRAMID_LEVEL_TOO_LARGE;
    }

    if (createData.angleExtent > 360 || createData.angleExtent < 0)
    {
        err_msg_ = "invalid angle range";
        return MLR_TEMPLATE_ANGLE_RANGE_INVALID;
    }

    return MLR_SUCCESS;
}

void BaseTemplate::destroyBaseData()
{
    pyramid_level_ = 1;
    angle_start_ = -30;
    angle_extent_ = 60;
    cfs_.clear();
    top_layer_full_domain_.release();
    top_layer_search_roi_.release();
    pyrs_.clear();
    tmpl_rgn_.release();
    tmpl_cont_.release();
    tmpl_img_ = cv::Mat();
    err_msg_.resize(0);
}

void BaseTemplate::clearCacheMatchData()
{
    row_ptrs_.resize(0);
    candidates_.resize(0);
    top_candidates_.resize(0);
    final_candidates_.resize(0);
    candidate_runs_.resize(0);
    candidate_groups_.resize(0);
}

void BaseTemplate::supressNoneMaximum()
{
    candidate_runs_.RLEncodeCandidates(candidates_);
    candidate_runs_.Connect(candidate_groups_);
    candidates_.resize(candidate_groups_.size());

    BaseTemplate::Candidate *candidates = candidates_.data();
    for (const BaseTemplate::CandidateGroup &cg : candidate_groups_)
    {
        *candidates = cg.front().best;
        for (const BaseTemplate::CandidateRun &cr : cg)
        {
            if (cr.best.score > candidates->score)
            {
                *candidates = cr.best;
            }
        }
        candidates += 1;
    }

    top_candidates_.assign(candidates_.cbegin(), candidates_.cend());
}

void BaseTemplate::processToplayerSearchROI(const cv::Ptr<Region> &roi, const int pyramidLevel)
{
    if (roi && !roi->Empty())
    {
        const float s = std::pow(0.5f, pyramidLevel - 1);
        top_layer_search_roi_ = roi->Zoom(cv::Size2f(s, s));
    }
    else
    {
        top_layer_search_roi_.release();
    }
}

float BaseTemplate::maxScoreInterpolate(const cv::Mat &scores, cv::Point2f &pos, float &angle) const
{
    pos.x = 0.f;
    pos.y = 0.f;
    angle = 0.f;

    cv::Mat A(3, 3, CV_64FC1);
    cv::Mat b(3, 1, CV_64FC1);
    cv::Mat C = score_fitting_mask_ * scores;
    A.ptr<double>(0)[0] = 2 * C.ptr<double>(4)[0];
    A.ptr<double>(0)[1] = C.ptr<double>(7)[0];
    A.ptr<double>(0)[2] = C.ptr<double>(9)[0];
    A.ptr<double>(1)[0] = C.ptr<double>(7)[0];
    A.ptr<double>(1)[1] = 2 * C.ptr<double>(5)[0];
    A.ptr<double>(1)[2] = C.ptr<double>(8)[0];
    A.ptr<double>(2)[0] = C.ptr<double>(9)[0];
    A.ptr<double>(2)[1] = C.ptr<double>(8)[0];
    A.ptr<double>(2)[2] = 2 * C.ptr<double>(6)[0];
    b.ptr<double>(0)[0] = -C.ptr<double>(1)[0];
    b.ptr<double>(1)[0] = -C.ptr<double>(2)[0];
    b.ptr<double>(2)[0] = -C.ptr<double>(3)[0];

    cv::Mat x;
    if (cv::solve(A, b, x))
    {
        const double r = x.ptr<double>(0)[0];
        const double c = x.ptr<double>(1)[0];
        const double a = x.ptr<double>(2)[0];
        pos.x = static_cast<float>(c);
        pos.y = static_cast<float>(r);
        angle = static_cast<float>(a);

        return static_cast<float>(C.ptr<double>(0)[0] + C.ptr<double>(1)[0] * r + C.ptr<double>(2)[0] * c + C.ptr<double>(3)[0] * a +
            C.ptr<double>(4)[0] * r * r + C.ptr<double>(5)[0] * c * c + C.ptr<double>(6)[0] * a * a +
            C.ptr<double>(7)[0] * r * c + C.ptr<double>(8)[0] * c * a + C.ptr<double>(9)[0] * r * a);
    }
    else
    {
        return 0.f;
    }
}

void BaseTemplate::moveCandidatesToLowerLayer(const int layer)
{
    for (BaseTemplate::Candidate &candidate : candidates_)
    {
        candidate.score = 0.f;
    }

    for (const BaseTemplate::Candidate &candidate : final_candidates_)
    {
        if (candidate.mindex >= 0)
        {
            if (candidate.score > candidates_[candidate.label].score)
            {
                candidates_[candidate.label] = std::move(candidate);
            }
        }
    }

    final_candidates_.resize(0);
    for (const BaseTemplate::Candidate &candidate : candidates_)
    {
        if (candidate.score > 0.f)
        {
            final_candidates_.push_back(std::move(candidate));
        }
    }

    if (layer > 0)
    {
        candidates_.swap(final_candidates_);

        final_candidates_.resize(0);
        const int numLayerCandidates = static_cast<int>(candidates_.size());
        for (int cc = 0; cc < numLayerCandidates; ++cc)
        {
            const BaseTemplate::Candidate &candidate = candidates_[cc];
            for (int row = -2; row < 3; ++row)
            {
                for (int col = -2; col < 3; ++col)
                {
                    final_candidates_.emplace_back(candidate.row * 2 + row, candidate.col * 2 + col, candidate.mindex, cc);
                }
            }
        }
    }
}

void BaseTemplate::startMoveCandidatesToLowerLayer()
{
    final_candidates_.resize(0);
    const int numTopLayerCandidates = static_cast<int>(candidates_.size());
    for (int cc = 0; cc < numTopLayerCandidates; ++cc)
    {
        const BaseTemplate::Candidate &candidate = candidates_[cc];
        for (int row = -2; row < 3; ++row)
        {
            for (int col = -2; col < 3; ++col)
            {
                final_candidates_.emplace_back(candidate.row * 2 + row, candidate.col * 2 + col, candidate.mindex, cc);
            }
        }
    }
}

std::tuple<const int, const RunLength *> BaseTemplate::getSearchRegion(const cv::Ptr<Region> &searchRgn, const int nTopRows, const int nTopCols)
{
    int numSearchRuns = 0;
    const RunLength *searchRuns = nullptr;

    if (searchRgn && !searchRgn->Empty())
    {
        const float s = std::pow(0.5f, pyramid_level_ - 1);
        top_layer_search_roi_ = searchRgn->Shrink(cv::Size2f(s, s));
    }

    if (top_layer_search_roi_.empty() || top_layer_search_roi_->Empty())
    {
        if (top_layer_full_domain_.empty() ||
            top_layer_full_domain_->BoundingBox() != cv::Rect(0, 0, nTopCols, nTopRows))
        {
            top_layer_full_domain_ = Region::GenRectangle(cv::Rect2f(0.f, 0.f, static_cast<float>(nTopCols), static_cast<float>(nTopRows)));
        }

        cv::Ptr<RegionImpl> rgn = top_layer_full_domain_.dynamicCast<RegionImpl>();
        numSearchRuns           = rgn->CountRuns();
        searchRuns              = rgn->GetAllRuns().data();
    }
    else
    {
        cv::Ptr<RegionImpl> rgn = top_layer_search_roi_.dynamicCast<RegionImpl>();
        numSearchRuns = rgn->CountRuns();
        searchRuns = rgn->GetAllRuns().data();
    }

    return { numSearchRuns , searchRuns };
}

}
}