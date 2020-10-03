#include "precomp.hpp"
#include "connection.hpp"
#include "region_impl.hpp"
#include "utility.hpp"

namespace cv {
namespace mvlab {
//Find the root of the tree of node i
template<typename LabelT>
inline static
    LabelT findRoot(const LabelT *P, LabelT i) {
    LabelT root = i;
    while (P[root] < root) {
        root = P[root];
    }
    return root;
}

//Make all nodes in the path of node i point to root
template<typename LabelT>
inline static
    void setRoot(LabelT *P, LabelT i, LabelT root) {
    while (P[i] < i) {
        LabelT j = P[i];
        P[i] = root;
        i = j;
    }
    P[i] = root;
}

//Find the root of the tree of the node i and compress the path in the process
template<typename LabelT>
inline static
    LabelT find(LabelT *P, LabelT i) {
    LabelT root = findRoot(P, i);
    setRoot(P, i, root);
    return root;
}

//unite the two trees containing nodes i and j and return the new root
template<typename LabelT>
inline static
    LabelT set_union(LabelT *P, LabelT i, LabelT j) {
    LabelT root = findRoot(P, i);
    if (i != j) {
        LabelT rootj = findRoot(P, j);
        if (root > rootj) {
            root = rootj;
        }
        setRoot(P, j, root);
    }
    setRoot(P, i, root);
    return root;
}

//Flatten the Union Find tree and relabel the components
template<typename LabelT>
inline static
    LabelT flattenL(LabelT *P, LabelT length) {
    LabelT k = 0;
    for (LabelT i = 0; i < length; ++i) {
        if (P[i] < i) {
            P[i] = P[P[i]];
        }
        else {
            P[i] = k; k = k + 1;
        }
    }
    return k;
}

template<typename LabelT>
inline static
    void flattenL(LabelT *P, const int start, const int nElem, LabelT& k) {
    for (int i = start; i < start + nElem; ++i) {
        if (P[i] < i) {//node that point to root
            P[i] = P[P[i]];
        }
        else { //for root node
            P[i] = k;
            k = k + 1;
        }
    }
}

void ConnectWuSerial::Connect(const Region *rgn, const int connectivity, std::vector<Ptr<Region>> &regions) const
{
    regions.clear();
    const RegionImpl *rgnImpl = dynamic_cast<const RegionImpl*>(rgn);
    if (!rgnImpl)
    {
        return;
    }

    ScalableIntSequence numRunsOfRgn;
    if (8 == connectivity)
    {
        ConnectCommon8(rgn, numRunsOfRgn);
    }
    else
    {
        ConnectCommon4(rgn, numRunsOfRgn);
    }

    RunSequenceSequence runSeqSeq(numRunsOfRgn.size());

    auto pNumRuns = numRunsOfRgn.data();
    auto pRunSeqSeqEnd = runSeqSeq.data() + runSeqSeq.size();
    for (auto pRunSeqSeq = runSeqSeq.data(); pRunSeqSeq != pRunSeqSeqEnd; ++pRunSeqSeq, ++pNumRuns)
    {
        pRunSeqSeq->reserve(*pNumRuns);
    }

    const RunSequence &allRuns = const_cast<RunSequence &>(rgnImpl->GetAllRuns());
    for (const auto &run : allRuns)
    {
        runSeqSeq[run.label].push_back(run);
    }

    regions.reserve(runSeqSeq.size());
    for (RunSequence &runSeq : runSeqSeq)
    {
        regions.push_back(cv::makePtr<RegionImpl>(&runSeq));
    }
}

void ConnectWuSerial::ConnectCommon8(const Region *rgn, ScalableIntSequence &numRunsOfRgn) const
{
    const RegionImpl *rgnImpl = dynamic_cast<const RegionImpl*>(rgn);
    RunSequence &allRuns = const_cast<RunSequence &>(rgnImpl->GetAllRuns());
    const RowBeginSequence &rowRanges = rgnImpl->GetRowBeginSequence();
    const cv::Rect bbox = rgnImpl->BoundingBox();
    const int maxWidth = bbox.width + 3;

    AdaptBuffer<RunLength *> rowRunPtrs(maxWidth);
    ScalableIntSequence  vecP(allRuns.size());
    RunLength **vRowRunPtrs = rowRunPtrs.data() + 1 - bbox.x;
    std::memset(rowRunPtrs.data(), 0, maxWidth * sizeof(RunLength *));

    LabelT lunique = 0;
    LabelT *P = vecP.data();
    int rowPrev = std::numeric_limits<int>::min();

    RunLength *pRuns = allRuns.data();
    const int *pRowRunStarts = rowRanges.data();
    const int numRows = static_cast<int>(rowRanges.size() - 1);

    for (int row = 0; row < numRows; ++row)
    {
        RunLength *pRowRunBeg = pRuns;
        RunLength *pRowRunEnd = pRuns + *(pRowRunStarts + 1) - *pRowRunStarts;

        if ((rowPrev + 1) == pRuns->row)
        {
            for (RunLength *pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
            {
                int col = pRun->colb - 1;
                while (!vRowRunPtrs[col] && col <= pRun->cole)
                {
                    col += 1;
                }

                if (col > pRun->cole)
                {
                    pRun->label = lunique; P[lunique] = lunique; lunique = lunique + 1;
                }
                else
                {
                    LabelT lab = vRowRunPtrs[col]->label;
                    col = vRowRunPtrs[col]->cole + 1;

                    for (; col <= pRun->cole; ++col)
                    {
                        if (vRowRunPtrs[col])
                        {
                            lab = set_union(P, lab, vRowRunPtrs[col]->label);
                            col = vRowRunPtrs[col]->cole;
                        }
                    }

                    pRun->label = lab;
                }
            }
        }
        else
        {
            for (RunLength *pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
            {
                pRun->label = lunique; P[lunique] = lunique; lunique = lunique + 1;
            }
        }

        std::memset(rowRunPtrs.data(), 0, maxWidth * sizeof(RunLength *));
        for (RunLength *pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
        {
            for (int col = pRun->colb; col < pRun->cole; ++col)
            {
                vRowRunPtrs[col] = pRun;
            }
        }

        rowPrev = pRuns->row;
        pRuns = pRowRunEnd;
        pRowRunStarts += 1;
    }

    LabelT nLabels = flattenL(P, lunique);

    numRunsOfRgn.resize(nLabels);
    RunLength *pRunsEnd = allRuns.data() + allRuns.size();
    for (RunLength *pRun = allRuns.data(); pRun != pRunsEnd; ++pRun)
    {
        pRun->label = P[pRun->label];
        numRunsOfRgn[pRun->label] += 1;
    }
}

void ConnectWuSerial::ConnectCommon4(const Region *rgn, ScalableIntSequence &numRunsOfRgn) const
{
    const RegionImpl *rgnImpl = dynamic_cast<const RegionImpl*>(rgn);
    RunSequence &allRuns = const_cast<RunSequence &>(rgnImpl->GetAllRuns());
    const RowBeginSequence &rowRanges = rgnImpl->GetRowBeginSequence();
    const cv::Rect bbox = rgnImpl->BoundingBox();
    const int maxWidth = bbox.width + 3;

    AdaptBuffer<RunLength *> rowRunPtrs(maxWidth);
    ScalableIntSequence  vecP(allRuns.size());
    RunLength **vRowRunPtrs = rowRunPtrs.data() + 1 - bbox.x;
    std::memset(rowRunPtrs.data(), 0, maxWidth * sizeof(RunLength *));

    LabelT lunique = 0;
    LabelT *P = vecP.data();
    int rowPrev = std::numeric_limits<int>::min();

    RunLength *pRuns = allRuns.data();
    const int *pRowRunStarts = rowRanges.data();
    const int numRows = static_cast<int>(rowRanges.size() - 1);

    for (int row = 0; row < numRows; ++row)
    {
        RunLength *pRowRunBeg = pRuns;
        RunLength *pRowRunEnd = pRuns + *(pRowRunStarts + 1) - *pRowRunStarts;

        if ((rowPrev + 1) == pRuns->row)
        {
            for (RunLength *pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
            {
                int col = pRun->colb;
                while (!vRowRunPtrs[col] && col < pRun->cole)
                {
                    col += 1;
                }

                if (col >= pRun->cole)
                {
                    pRun->label = lunique; P[lunique] = lunique; lunique = lunique + 1;
                }
                else
                {
                    LabelT lab = vRowRunPtrs[col]->label;
                    col = vRowRunPtrs[col]->cole + 1;

                    for (; col < pRun->cole; ++col)
                    {
                        if (vRowRunPtrs[col])
                        {
                            lab = set_union(P, lab, vRowRunPtrs[col]->label);
                            col = vRowRunPtrs[col]->cole;
                        }
                    }

                    pRun->label = lab;
                }
            }
        }
        else
        {
            for (RunLength *pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
            {
                pRun->label = lunique; P[lunique] = lunique; lunique = lunique + 1;
            }
        }

        std::memset(rowRunPtrs.data(), 0, maxWidth * sizeof(RunLength *));
        for (RunLength *pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
        {
            for (int col = pRun->colb; col < pRun->cole; ++col)
            {
                vRowRunPtrs[col] = pRun;
            }
        }

        rowPrev = pRuns->row;
        pRuns = pRowRunEnd;
        pRowRunStarts += 1;
    }

    LabelT nLabels = flattenL(P, lunique);

    numRunsOfRgn.resize(nLabels);
    RunLength *pRunsEnd = allRuns.data() + allRuns.size();
    for (RunLength *pRun = allRuns.data(); pRun != pRunsEnd; ++pRun)
    {
        pRun->label = P[pRun->label];
        numRunsOfRgn[pRun->label] += 1;
    }
}

ConnectWuParallel::FirstScan8Connectivity::FirstScan8Connectivity(LabelT *P,
    int *chunksSizeAndLabels,
    const cv::Rect &bbox,
    const RowBeginSequence &rowRunBegs,
    RunSequence &data)
    : P_(P)
    , chunksSizeAndLabels_(chunksSizeAndLabels)
    , bbox_(bbox)
    , rowRunBegs_(rowRunBegs)
    , data_(data)
{}

void ConnectWuParallel::FirstScan8Connectivity::operator()(const tbb::blocked_range<int>& br) const
{
    const int rowBeg = br.begin();
    const int rowEnd = br.end();
    chunksSizeAndLabels_[rowBeg] = br.end();

    LabelT label = rowRunBegs_[rowBeg];
    const LabelT firstLabel = label;
    const int maxWidth = bbox_.width + 3;

    AdaptBuffer<RunLength *> rowRunPtrs(maxWidth);
    RunLength **vRowRunPtrs = rowRunPtrs.data() + 1 - bbox_.x;
    int rowPrev = std::numeric_limits<int>::min();
    RunLength *pRuns = data_.data() + rowRunBegs_[rowBeg];
    const int *pRowRunStarts = rowRunBegs_.data() + rowBeg;
    std::memset(rowRunPtrs.data(), 0, maxWidth * sizeof(RunLength *));

    for (int row = rowBeg; row != rowEnd; ++row)
    {
        RunLength *pRowRunBeg = pRuns;
        RunLength *pRowRunEnd = pRuns + *(pRowRunStarts + 1) - *pRowRunStarts;

        if ((rowPrev + 1) == pRuns->row) {
            for (RunLength *pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
            {
                int col = pRun->colb - 1;
                while (!vRowRunPtrs[col] && col <= pRun->cole) {
                    col += 1;
                }

                if (col > pRun->cole) {
                    pRun->label = label; P_[label] = label; label = label + 1;
                }
                else {
                    LabelT lab = vRowRunPtrs[col]->label;
                    col = vRowRunPtrs[col]->cole + 1;
                    for (; col <= pRun->cole; ++col)
                    {
                        if (vRowRunPtrs[col]) {
                            lab = set_union(P_, lab, vRowRunPtrs[col]->label);
                            col = vRowRunPtrs[col]->cole;
                        }
                    }
                    pRun->label = lab;
                }
            }
        }
        else {
            for (RunLength *pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
            {
                pRun->label = label; P_[label] = label; label = label + 1;
            }
        }

        std::memset(rowRunPtrs.data(), 0, maxWidth * sizeof(RunLength *));
        for (RunLength *pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
        {
            for (int col = pRun->colb; col < pRun->cole; ++col)
            {
                vRowRunPtrs[col] = pRun;
            }
        }

        rowPrev = pRuns->row; pRuns = pRowRunEnd; pRowRunStarts += 1;
    }

    chunksSizeAndLabels_[rowBeg + 1] = label - firstLabel;
}

ConnectWuParallel::FirstScan4Connectivity::FirstScan4Connectivity(LabelT *P,
    int *chunksSizeAndLabels,
    const cv::Rect &bbox,
    const RowBeginSequence &rowRunBegs,
    RunSequence &data)
    : P_(P)
    , chunksSizeAndLabels_(chunksSizeAndLabels)
    , bbox_(bbox)
    , rowRunBegs_(rowRunBegs)
    , data_(data)
{}

void ConnectWuParallel::FirstScan4Connectivity::operator()(const tbb::blocked_range<int>& br) const
{
    const int rowBeg = br.begin();
    const int rowEnd = br.end();
    chunksSizeAndLabels_[rowBeg] = br.end();

    LabelT label = rowRunBegs_[rowBeg];
    const LabelT firstLabel = label;
    const int maxWidth = bbox_.width + 3;

    AdaptBuffer<RunLength *> rowRunPtrs(maxWidth);
    RunLength **vRowRunPtrs = rowRunPtrs.data() + 1 - bbox_.x;
    int rowPrev = std::numeric_limits<int>::min();
    RunLength *pRuns = data_.data() + rowRunBegs_[rowBeg];
    const int *pRowRunStarts = rowRunBegs_.data() + rowBeg;
    std::memset(rowRunPtrs.data(), 0, maxWidth * sizeof(RunLength *));

    for (int row = rowBeg; row != rowEnd; ++row)
    {
        RunLength *pRowRunBeg = pRuns;
        RunLength *pRowRunEnd = pRuns + *(pRowRunStarts + 1) - *pRowRunStarts;

        if ((rowPrev + 1) == pRuns->row) {
            for (RunLength *pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
            {
                int col = pRun->colb;
                while (!vRowRunPtrs[col] && col < pRun->cole) {
                    col += 1;
                }

                if (col >= pRun->cole) {
                    pRun->label = label; P_[label] = label; label = label + 1;
                }
                else {
                    LabelT lab = vRowRunPtrs[col]->label;
                    col = vRowRunPtrs[col]->cole + 1;
                    for (; col < pRun->cole; ++col)
                    {
                        if (vRowRunPtrs[col]) {
                            lab = set_union(P_, lab, vRowRunPtrs[col]->label);
                            col = vRowRunPtrs[col]->cole;
                        }
                    }
                    pRun->label = lab;
                }
            }
        }
        else {
            for (RunLength *pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
            {
                pRun->label = label; P_[label] = label; label = label + 1;
            }
        }

        std::memset(rowRunPtrs.data(), 0, maxWidth * sizeof(RunLength *));
        for (RunLength *pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
        {
            for (int col = pRun->colb; col < pRun->cole; ++col)
            {
                vRowRunPtrs[col] = pRun;
            }
        }

        rowPrev = pRuns->row; pRuns = pRowRunEnd; pRowRunStarts += 1;
    }

    chunksSizeAndLabels_[rowBeg + 1] = label - firstLabel;
}

void ConnectWuParallel::Connect(const Region *rgn, const int connectivity, std::vector<Ptr<Region>> &regions) const
{
    regions.clear();
    const RegionImpl *rgnImpl = dynamic_cast<const RegionImpl*>(rgn);
    if (!rgnImpl)
    {
        return;
    }

    ScalableIntSequence numRunsOfRgn;
    ConnectCommon(rgn, connectivity, numRunsOfRgn);

    RunSequenceSequence runSeqSeq(numRunsOfRgn.size());

    auto pNumRuns = numRunsOfRgn.data();
    auto pRunSeqSeqEnd = runSeqSeq.data() + runSeqSeq.size();
    for (auto pRunSeqSeq = runSeqSeq.data(); pRunSeqSeq != pRunSeqSeqEnd; ++pRunSeqSeq, ++pNumRuns)
    {
        pRunSeqSeq->reserve(*pNumRuns);
    }

    const RunSequence &allRuns = const_cast<RunSequence &>(rgnImpl->GetAllRuns());
    for (const auto &run : allRuns)
    {
        runSeqSeq[run.label].push_back(run);
    }

    regions.reserve(runSeqSeq.size());
    for (RunSequence &runSeq : runSeqSeq)
    {
        regions.push_back(cv::makePtr<RegionImpl>(&runSeq));
    }
}

void ConnectWuParallel::mergeLabels8Connectivity(const RegionImpl &rgn,
    LabelT *P,
    const int *chunksSizeAndLabels) const
{
    RunSequence &allRuns = const_cast<RunSequence &>(rgn.GetAllRuns());
    const RowBeginSequence &rowRunBegs = rgn.GetRowBeginSequence();
    const int numRows = static_cast<int>(rowRunBegs.size() - 1);
    const cv::Rect bbox = rgn.BoundingBox();
    const int maxWidth = bbox.width + 3;

    AdaptBuffer<RunLength *> rowRunPtrs(maxWidth);
    RunLength **vRowRunPtrs = rowRunPtrs.data() + 1 - bbox.x;

    for (int row = chunksSizeAndLabels[0]; row < numRows; row = chunksSizeAndLabels[row])
    {
        RunLength *pRowRunBeg = allRuns.data() + rowRunBegs[row];
        RunLength *pRowRunEnd = pRowRunBeg + rowRunBegs[row + 1] - rowRunBegs[row];
        int rowPrev = row == 0 ? std::numeric_limits<int>::min() : allRuns[rowRunBegs[row - 1]].row;
        if ((rowPrev + 1) == pRowRunBeg->row)
        {
            std::memset(rowRunPtrs.data(), 0, maxWidth * sizeof(const RunLength *));
            RunLength *pPrevRowRunBeg = allRuns.data() + rowRunBegs[row - 1];
            RunLength *pPrevRowRunEnd = pPrevRowRunBeg + rowRunBegs[row] - rowRunBegs[row - 1];

            for (RunLength *pRun = pPrevRowRunBeg; pRun != pPrevRowRunEnd; ++pRun)
            {
                for (int col = pRun->colb; col < pRun->cole; ++col)
                {
                    vRowRunPtrs[col] = pRun;
                }
            }

            for (RunLength *pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
            {
                int col = pRun->colb - 1;
                while (!vRowRunPtrs[col] && col <= pRun->cole) {
                    col += 1;
                }

                if (col <= pRun->cole) {
                    LabelT lab = vRowRunPtrs[col]->label;
                    col = vRowRunPtrs[col]->cole + 1;
                    for (; col <= pRun->cole; ++col)
                    {
                        if (vRowRunPtrs[col]) {
                            lab = set_union(P, lab, vRowRunPtrs[col]->label);
                            col = vRowRunPtrs[col]->cole;
                        }
                    }
                    pRun->label = set_union(P, lab, pRun->label);
                }
            }
        }
    }
}

void ConnectWuParallel::mergeLabels4Connectivity(const RegionImpl &rgn,
    LabelT *P,
    const int *chunksSizeAndLabels) const
{
    RunSequence &allRuns = const_cast<RunSequence &>(rgn.GetAllRuns());
    const RowBeginSequence &rowRunBegs = rgn.GetRowBeginSequence();
    const int numRows = static_cast<int>(rowRunBegs.size() - 1);
    const cv::Rect bbox = rgn.BoundingBox();
    const int maxWidth = bbox.width + 3;

    AdaptBuffer<RunLength *> rowRunPtrs(maxWidth);
    RunLength **vRowRunPtrs = rowRunPtrs.data() + 1 - bbox.x;

    for (int row = chunksSizeAndLabels[0]; row < numRows; row = chunksSizeAndLabels[row])
    {
        RunLength *pRowRunBeg = allRuns.data() + rowRunBegs[row];
        RunLength *pRowRunEnd = pRowRunBeg + rowRunBegs[row + 1] - rowRunBegs[row];
        int rowPrev = row == 0 ? std::numeric_limits<int>::min() : allRuns[rowRunBegs[row - 1]].row;
        if ((rowPrev + 1) == pRowRunBeg->row)
        {
            std::memset(rowRunPtrs.data(), 0, maxWidth * sizeof(const RunLength *));
            RunLength *pPrevRowRunBeg = allRuns.data() + rowRunBegs[row - 1];
            RunLength *pPrevRowRunEnd = pPrevRowRunBeg + rowRunBegs[row] - rowRunBegs[row - 1];

            for (RunLength *pRun = pPrevRowRunBeg; pRun != pPrevRowRunEnd; ++pRun)
            {
                for (int col = pRun->colb; col < pRun->cole; ++col)
                {
                    vRowRunPtrs[col] = pRun;
                }
            }

            for (RunLength *pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
            {
                int col = pRun->colb;
                while (!vRowRunPtrs[col] && col < pRun->cole) {
                    col += 1;
                }

                if (col < pRun->cole) {
                    LabelT lab = vRowRunPtrs[col]->label;
                    col = vRowRunPtrs[col]->cole + 1;
                    for (; col < pRun->cole; ++col)
                    {
                        if (vRowRunPtrs[col]) {
                            lab = set_union(P, lab, vRowRunPtrs[col]->label);
                            col = vRowRunPtrs[col]->cole;
                        }
                    }
                    pRun->label = set_union(P, lab, pRun->label);
                }
            }
        }
    }
}

void ConnectWuParallel::ConnectCommon(const Region *rgn,
    const int connectivity,
    ScalableIntSequence &numRunsOfRgn) const
{
    const RegionImpl *rgnImpl = dynamic_cast<const RegionImpl*>(rgn);
    RunSequence &allRuns = const_cast<RunSequence &>(rgnImpl->GetAllRuns());
    const RowBeginSequence &rowRunBegs = rgnImpl->GetRowBeginSequence();
    const cv::Rect bbox = rgnImpl->BoundingBox();

    const auto Plength = allRuns.size();
    ScalableIntSequence  vecP(Plength);
    LabelT *P = vecP.data();

    ScalableIntSequence vecChunksSizeAndLabels(rowRunBegs.size());
    int *chunksSizeAndLabels = vecChunksSizeAndLabels.data();
    const int numRows = static_cast<int>(rowRunBegs.size() - 1);

    const int nThreads = static_cast<int>(tbb::global_control::active_value(tbb::global_control::max_allowed_parallelism));
    tbb::blocked_range<int> range(0, numRows, std::max(2, std::min(numRows / 2, nThreads * 4)));

    if (8 == connectivity)
    {
        tbb::parallel_for(range, FirstScan8Connectivity(P, chunksSizeAndLabels, bbox, rowRunBegs, allRuns));
        mergeLabels8Connectivity(*rgnImpl, P, chunksSizeAndLabels);
    }
    else
    {
        tbb::parallel_for(range, FirstScan4Connectivity(P, chunksSizeAndLabels, bbox, rowRunBegs, allRuns));
        mergeLabels4Connectivity(*rgnImpl, P, chunksSizeAndLabels);
    }

    LabelT nLabels = 0;
    for (int i = 0; i < numRows; i = chunksSizeAndLabels[i])
    {
        flattenL(P, rowRunBegs[i], chunksSizeAndLabels[i + 1], nLabels);
    }

    numRunsOfRgn.resize(nLabels);
    RunLength *pRunsEnd = allRuns.data() + allRuns.size();
    for (RunLength *pRun = allRuns.data(); pRun != pRunsEnd; ++pRun)
    {
        pRun->label = P[pRun->label];
        numRunsOfRgn[pRun->label] += 1;
    }
}

}
}
