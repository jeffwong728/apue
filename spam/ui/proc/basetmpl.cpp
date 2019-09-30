#include "basetmpl.h"
#include "basic.h"
#include <limits>
#include <stack>
#include <vectorclass/vectorclass.h>
#include <boost/container/static_vector.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 4267 4244 )
#include <2geom/path-sink.h>
#include <2geom/path-intersection.h>
#pragma warning( pop )
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>

BaseTemplate::BaseTemplate()
    : pyramid_level_(4)
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
        constexpr int negInf = std::numeric_limits<int>::min();
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

SpamResult BaseTemplate::verifyCreateData(const BaseTmplCreateData &createData)
{
    if (createData.pyramidLevel < 1)
    {
        return SpamResult::kSR_TM_PYRAMID_LEVEL_INVALID;
    }

    if (createData.srcImg.empty())
    {
        return SpamResult::kSR_IMG_EMPTY;
    }

    int topWidth = createData.srcImg.cols;
    int topHeight = createData.srcImg.rows;
    for (int n = 0; n < (createData.pyramidLevel - 1); ++n)
    {
        topWidth /= 2;
        topHeight /= 2;
    }

    if (0 == topWidth || 0 == topHeight)
    {
        return SpamResult::kSR_TM_PYRAMID_LEVEL_TOO_LARGE;
    }

    if (createData.angleExtent > 360 || createData.angleExtent < 0)
    {
        return SpamResult::kSR_TM_ANGLE_RANGE_INVALID;
    }

    return SpamResult::kSR_OK;
}

void BaseTemplate::destroyBaseData()
{
    pyramid_level_ = 1;
    cfs_.clear();
    tmpl_rgns_.clear();
    top_layer_search_roi_.clear();
    top_layer_search_roi_g_.clear();
    pyrs_.clear();
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

void BaseTemplate::processToplayerSearchROI(const Geom::PathVector &roi, const int pyramidLevel)
{
    if (roi.empty())
    {
        top_layer_search_roi_.clear();
        top_layer_search_roi_g_.clear();
    }
    else
    {
        double s = std::pow(0.5, pyramidLevel - 1);
        top_layer_search_roi_g_ = roi * Geom::Scale(s, s);
        top_layer_search_roi_.SetRegion(top_layer_search_roi_g_, std::vector<uint8_t>());
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

std::tuple<const int, const SpamRun *> BaseTemplate::getSearchRegion(const int nTopRows, const int nTopCols)
{
    int numSearchRuns = 0;
    const SpamRun *searchRuns = nullptr;
    if (top_layer_search_roi_.GetData().empty())
    {
        top_layer_full_domain_.SetRegion(cv::Rect(0, 0, nTopCols, nTopRows));
        numSearchRuns = static_cast<int>(top_layer_full_domain_.GetData().size());
        searchRuns = top_layer_full_domain_.GetData().data();
    }
    else
    {
        numSearchRuns = static_cast<int>(top_layer_search_roi_.GetData().size());
        searchRuns = top_layer_search_roi_.GetData().data();
    }

    return { numSearchRuns , searchRuns };
}