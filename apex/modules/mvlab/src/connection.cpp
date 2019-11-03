#include "precomp.hpp"
#include "connection.hpp"
#include "region_impl.hpp"
#include "region_collection_impl.hpp"

namespace cv {
namespace mvlab {
using LabelT = int;
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

cv::Ptr<RegionCollection> ConnectWuSerial::operator()(const Region *rgn, const int connectivity) const
{
    const RegionImpl *rgnImpl = dynamic_cast<const RegionImpl*>(rgn);
    if (!rgnImpl)
    {
        return makePtr<RegionCollectionImpl>();
    }

    RunList &allRuns = const_cast<RunList &>(rgnImpl->GetAllRuns());
    const RowRunStartList &rowRanges = rgnImpl->GetRowRunStartList();
    const cv::Rect bbox = rgnImpl->BoundingBox();
    const int maxWidth = bbox.width + 3;

    std::vector<RunLength *> rowRunPtrs(maxWidth);
    std::vector<LabelT>  vecP(allRuns.size());
    RunLength **vRowRunPtrs = rowRunPtrs.data() + 1 - bbox.x;

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

    std::vector<int> numRunsOfRgn(nLabels);
    RunLength *pRunsEnd = allRuns.data() + allRuns.size();
    for (RunLength *pRun = allRuns.data(); pRun != pRunsEnd; ++pRun)
    {
        pRun->label = P[pRun->label];
        numRunsOfRgn[pRun->label] += 1;
    }

    RunList collectionRuns(allRuns.size());
    rowRunPtrs.resize(nLabels);
    RunLength **pRunPtr = rowRunPtrs.data();
    RunLength *pFirstRun = collectionRuns.data();
    *(pRunPtr++) = pFirstRun;

    int *pNumEnd = numRunsOfRgn.data() + numRunsOfRgn.size();
    for (int *pNum = numRunsOfRgn.data()+1; pNum != pNumEnd; ++pNum, ++pRunPtr)
    {
        *pRunPtr = pFirstRun + pNum[-1];
        *pNum += pNum[-1];
    }

    for (const auto &run : allRuns)
    {
        auto &pRun = rowRunPtrs[run.label];
        *pRun = run;
        pRun += 1;
    }

    return makePtr<RegionCollectionImpl>(&collectionRuns, &numRunsOfRgn);
}

}
}
