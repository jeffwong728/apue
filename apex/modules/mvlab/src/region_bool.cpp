#include "precomp.hpp"
#include "region_bool.hpp"

namespace cv {
namespace mvlab {

RunSequence RegionComplementOp::Do(const RunSequence &srcRuns, const RowBeginSequence &rowBegs, const cv::Rect &rcUniverse)
{
    const int numLines = rcUniverse.height + 1;
    const int numRows = static_cast<int>(rowBegs.size()) - 1;

    int numDstRuns = numLines - numRows;
    const RowBeginSequence::const_pointer pRowBegEnd = rowBegs.data() + rowBegs.size();
    for (RowBeginSequence::const_pointer pRowBeg = rowBegs.data()+1; pRowBeg != pRowBegEnd; ++pRowBeg)
    {
        numDstRuns += *pRowBeg - *(pRowBeg-1) + 1;
    }

    RunSequence dstRuns(numDstRuns);
    RunSequence::pointer pDstRun = dstRuns.data();

    int rIdx = 0;
    RowBeginSequence::const_pointer pRowBeg = rowBegs.data();
    const int lBeg = rcUniverse.y;
    const int lEnd = rcUniverse.y + rcUniverse.height;
    const RunSequence::const_pointer pRunBeg = srcRuns.data();
    const int colMin = rcUniverse.x;
    const int colMax = rcUniverse.x + rcUniverse.width + 1;
    for (int l = lBeg; l <= lEnd; ++l)
    {
        if (rIdx < numRows)
        {
            const RunSequence::const_pointer pRowRunBeg = pRunBeg + *pRowBeg;
            if (l == pRowRunBeg->row)
            {
                const RunSequence::const_pointer pRowRunEnd = pRunBeg + *(pRowBeg+1);
                int colb = colMin;
                for (RunSequence::const_pointer pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
                {
                    pDstRun->row = l;
                    pDstRun->colb = colb;
                    pDstRun->cole = pRun->colb;
                    colb = pRun->cole;

                    pDstRun += 1;
                }

                pDstRun->row = l;
                pDstRun->colb = colb;
                pDstRun->cole = colMax;
                pDstRun += 1;

                rIdx += 1;
                pRowBeg += 1;
            }
            else
            {
                pDstRun->row = l;
                pDstRun->colb = colMin;
                pDstRun->cole = colMax;
                pDstRun += 1;
            }
        }
        else
        {
            pDstRun->row = l;
            pDstRun->colb = colMin;
            pDstRun->cole = colMax;
            pDstRun += 1;
        }
    }

    return dstRuns;
}

}
}
