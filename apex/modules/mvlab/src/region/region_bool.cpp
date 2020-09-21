#include "precomp.hpp"
#include "region_bool.hpp"

namespace cv {
namespace mvlab {

void RegionBoolOp::GetRows(const RunSequence &srcRuns, const RowBeginSequence &rowBegs, UScalableIntSequence &rows)
{
    UScalableIntSequence::pointer pRow = rows.data();
    const RowBeginSequence::const_pointer pRowBegEnd = rowBegs.data() + rowBegs.size() - 1;
    for (RowBeginSequence::const_pointer pRowBeg = rowBegs.data(); pRowBeg != pRowBegEnd; ++pRowBeg, ++pRow)
    {
        *pRow = srcRuns[*pRowBeg].row;
    }
}

int RegionBoolOp::IntersectRows(UScalableIntSequence &rows1, UScalableIntSequence &rows2)
{
    UScalableIntSequence::const_pointer pRow1 = rows1.begin();
    UScalableIntSequence::const_pointer pRow2 = rows2.begin();
    const UScalableIntSequence::const_pointer pRowEnd1 = rows1.end();
    const UScalableIntSequence::const_pointer pRowEnd2 = rows2.end();
    UScalableIntSequence::pointer pResRow1 = rows1.begin();
    UScalableIntSequence::pointer pResRow2 = rows2.begin();

    int row1 = 0, row2 = 0;
    while (pRow1 != pRowEnd1 && pRow2 != pRowEnd2)
    {
        if (*pRow1 < *pRow2)
        {
            ++pRow1;
            ++row1;
        }
        else if (*pRow2 < *pRow1)
        {
            ++pRow2;
            ++row2;
        }
        else
        {
            *pResRow1 = row1;
            *pResRow2 = row2;
            ++pRow1;
            ++pRow2;
            ++row1;
            ++row2;
            ++pResRow1;
            ++pResRow2;
        }
    }

    return static_cast<int>(std::distance(rows1.begin(), pResRow1));
}

RunSequence RegionComplementOp::Do(const RunSequence &srcRuns, const cv::Rect &rcUniverse)
{
    RunSequence dstRuns(srcRuns.size() + rcUniverse.height + 1);
    RunSequence::const_pointer cur2 = srcRuns.data();
    const RunSequence::const_pointer end2 = srcRuns.data() + srcRuns.size();

    int cur1 = rcUniverse.y;
    const int end1 = rcUniverse.y + rcUniverse.height + 1;

    const int colMin = rcUniverse.x;
    const int colMax = rcUniverse.x + rcUniverse.width + 1;

    RunSequence::pointer pResRun = dstRuns.data();
    while (cur1 != end1 && cur2 != end2)
    {
        if (cur1 < cur2->row)
        {
            pResRun->row = cur1; pResRun->colb = colMin; pResRun->cole = colMax; pResRun->label = 0;
            ++pResRun;
            ++cur1;
        }
        else if (cur2->row < cur1)
        {
            ++cur2;
        }
        else
        {
            const int curR = cur1;
            assert(curR == cur2->row);
            RunSequence::const_pointer lastIncluded = cur2;

            while (lastIncluded != end2 && lastIncluded->row == curR)
            {
                ++lastIncluded;
            }

            --lastIncluded;
            pResRun->row = curR; pResRun->colb = colMin; pResRun->cole = cur2->colb; pResRun->label = 0;
            ++pResRun;

            while (cur2 != lastIncluded)
            {
                pResRun->row = curR; pResRun->colb = cur2->cole; pResRun->cole = (cur2 + 1)->colb; pResRun->label = 0;
                assert(pResRun->colb < pResRun->cole);
                ++pResRun;
                ++cur2;
            }

            pResRun->row = curR; pResRun->colb = lastIncluded->cole; pResRun->cole = colMax; pResRun->label = 0;
            assert(pResRun->colb < pResRun->cole);
            ++pResRun;

            ++cur1;
        }
    }

    for (; cur1 != end1; ++cur1, ++pResRun)
    {
        pResRun->row = cur1; pResRun->colb = colMin; pResRun->cole = colMax; pResRun->label = 0;
    }

    assert(std::distance(dstRuns.data(), pResRun) == (std::ptrdiff_t)dstRuns.size());
    return dstRuns;
}

RunSequence RegionDifferenceOp::Do(const RunSequence &srcRuns1, const RunSequence &srcRuns2)
{
    if (srcRuns1.empty())
    {
        return RunSequence();
    }

    if (srcRuns2.empty())
    {
        return srcRuns1;
    }

    RunSequence dstRuns(srcRuns1.size() + srcRuns2.size());
    RunSequence::const_pointer cur1 = srcRuns1.data();
    RunSequence::const_pointer cur2 = srcRuns2.data();
    const RunSequence::const_pointer end1 = srcRuns1.data() + srcRuns1.size();
    const RunSequence::const_pointer end2 = srcRuns2.data() + srcRuns2.size();

    RunSequence::pointer pResRun = dstRuns.data();
    while (cur1 != end1 && cur2 != end2)
    {
        if (cur1->row < cur2->row || (cur1->row == cur2->row && cur1->cole <= cur2->colb))
        {
            pResRun->row = cur1->row; pResRun->colb = cur1->colb; pResRun->cole = cur1->cole; pResRun->label = 0;
            ++pResRun;
            ++cur1;
        }
        else if (cur2->row < cur1->row || (cur1->row == cur2->row && cur2->cole <= cur1->colb))
        {
            ++cur2;
        }
        else
        {
            const int curR = cur1->row;
            assert(curR == cur2->row);
            RunSequence::const_pointer lastIncluded = cur2;
            bool bIncremented = false;
            while (lastIncluded != end2 && lastIncluded->row == curR && lastIncluded->colb < cur1->cole)
            {
                ++lastIncluded;
                bIncremented = true;
            }

            if (bIncremented)
            {
                --lastIncluded;
            }

            // now all chords from cur2 to lastIncluded have an intersection with cur1
            if (cur1->colb < cur2->colb)
            {
                pResRun->row = curR; pResRun->colb = cur1->colb; pResRun->cole = cur2->colb; pResRun->label = 0;
                ++pResRun;
            }

            while (cur2 != lastIncluded)
            {
                pResRun->row = curR; pResRun->colb = cur2->cole; pResRun->cole = (cur2+1)->colb; pResRun->label = 0;
                assert(pResRun->colb < pResRun->cole);
                ++pResRun;
                ++cur2;
            }

            if (cur1->cole > lastIncluded->cole)
            {
                pResRun->row = curR; pResRun->colb = lastIncluded->cole; pResRun->cole = cur1->cole; pResRun->label = 0;
                assert(pResRun->colb < pResRun->cole);
                ++pResRun;
            }
            ++cur1;
        }
    }

    for (; cur1 != end1; ++cur1, ++pResRun)
    {
        pResRun->row = cur1->row; pResRun->colb = cur1->colb; pResRun->cole = cur1->cole; pResRun->label = 0;
    }

    assert(std::distance(dstRuns.data(), pResRun) <= (std::ptrdiff_t)dstRuns.size());
    dstRuns.resize(std::distance(dstRuns.data(), pResRun));

    return dstRuns;
}

RunSequence RegionSymmDifferenceOp::Do(const RunSequence &srcRuns1, const RunSequence &srcRuns2)
{
    if (srcRuns1.empty())
    {
        return srcRuns2;
    }

    if (srcRuns2.empty())
    {
        return srcRuns1;
    }

    RunSequence dstRuns(srcRuns1.size() + srcRuns2.size());
    RunSequence::const_pointer cur1 = srcRuns1.data();
    RunSequence::const_pointer cur2 = srcRuns2.data();
    const RunSequence::const_pointer end1 = srcRuns1.data() + srcRuns1.size();
    const RunSequence::const_pointer end2 = srcRuns2.data() + srcRuns2.size();
    constexpr int negInf = std::numeric_limits<int>::min();
    ScalableIntSequence cols(1024);

    RunLength negInfRun{ negInf, negInf, negInf, 0};
    RunSequence::pointer pResRun = dstRuns.data();
    RunSequence::pointer pPreRun = &negInfRun;
    while (cur1 != end1 && cur2 != end2)
    {
        if (cur1->row < cur2->row || (cur1->row == cur2->row && cur1->cole < cur2->colb))
        {
            pResRun->row = cur1->row; pResRun->colb = cur1->colb; pResRun->cole = cur1->cole; pResRun->label = 0;
            pPreRun = pResRun;
            ++pResRun;
            ++cur1;
        }
        else if (cur2->row < cur1->row || (cur1->row == cur2->row && cur2->cole < cur1->colb))
        {
            pResRun->row = cur2->row; pResRun->colb = cur2->colb; pResRun->cole = cur2->cole; pResRun->label = 0;
            pPreRun = pResRun; 
            ++pResRun;
            ++cur2;
        }
        else
        {
            assert(cur1->row == cur2->row);
            const int curR = cur1->row;
            RunSequence::const_pointer rowEnd1 = cur1;
            RunSequence::const_pointer rowEnd2 = cur2;
            while (rowEnd1 != end1 && rowEnd1->row == curR) ++rowEnd1;
            while (rowEnd2 != end2 && rowEnd2->row == curR) ++rowEnd2;

            cols.resize((std::distance(cur1, rowEnd1)+std::distance(cur2, rowEnd2))*2);
            UScalableIntSequence::pointer pCol = cols.data();

            for (; cur1 != rowEnd1; ++cur1)
            {
                *pCol = cur1->colb; ++pCol;
                *pCol = cur1->cole; ++pCol;
            }

            for (; cur2 != rowEnd2; ++cur2)
            {
                *pCol = cur2->colb; ++pCol;
                *pCol = cur2->cole; ++pCol;
            }

            std::sort(cols.begin(), cols.end());

            bool odd = true;
            UScalableIntSequence::pointer pColEnd = cols.data() + cols.size();
            for (pCol = cols.data()+1; pCol != pColEnd; ++pCol)
            {
                if (odd && *pCol > *(pCol - 1))
                {
                    if (pPreRun->row == curR && pPreRun->cole == *(pCol-1))
                    {
                        pPreRun->cole = *pCol;
                    }
                    else
                    {
                        pResRun->row = curR; pResRun->colb = *(pCol - 1); pResRun->cole = *pCol; pResRun->label = 0;
                        pPreRun = pResRun;
                        ++pResRun;
                    }
                }

                odd = !odd;
            }
        }
    }

    for (; cur1 != end1; ++cur1, ++pResRun)
    {
        pResRun->row = cur1->row; pResRun->colb = cur1->colb; pResRun->cole = cur1->cole; pResRun->label = 0;
    }

    for (; cur2 != end2; ++cur2, ++pResRun)
    {
        pResRun->row = cur2->row; pResRun->colb = cur2->colb; pResRun->cole = cur2->cole; pResRun->label = 0;
    }

    assert(std::distance(dstRuns.data(), pResRun) <= (std::ptrdiff_t)dstRuns.size());
    dstRuns.resize(std::distance(dstRuns.data(), pResRun));

    return dstRuns;
}

RunSequence RegionIntersectionOp::Do(const RunSequence &srcRuns1, const RunSequence &srcRuns2)
{
    if (srcRuns1.empty() || srcRuns2.empty())
    {
        return RunSequence();
    }

    RunSequence dstRuns(srcRuns1.size() + srcRuns2.size());
    RunSequence::const_pointer cur1 = srcRuns1.data();
    RunSequence::const_pointer cur2 = srcRuns2.data();
    const RunSequence::const_pointer end1 = srcRuns1.data() + srcRuns1.size();
    const RunSequence::const_pointer end2 = srcRuns2.data() + srcRuns2.size();

    RunSequence::pointer pResRun = dstRuns.data();
    while (cur1 != end1 && cur2 != end2)
    {
        if (cur1->row < cur2->row || (cur1->row == cur2->row && cur1->cole <= cur2->colb))
        {
            ++cur1;
        }
        else if (cur2->row < cur1->row || (cur1->row == cur2->row && cur2->cole <= cur1->colb))
        {
            ++cur2;
        }
        else
        {
            pResRun->row = cur1->row;
            pResRun->colb = std::max(cur1->colb, cur2->colb);
            pResRun->cole = std::min(cur1->cole, cur2->cole);
            pResRun->label = 0;
            ++pResRun;
            if (cur1->cole < cur2->cole)
            {
                ++cur1;
            }
            else
            {
                ++cur2;
            }
        }
    }

    assert(std::distance(dstRuns.data(), pResRun) <= (std::ptrdiff_t)dstRuns.size());
    dstRuns.resize(std::distance(dstRuns.data(), pResRun));

    return dstRuns;
}

void RegionUnion2Op::Do(const RunSequence &srcRuns1, const RunSequence &srcRuns2, RunSequence &dstRuns)
{
    if (srcRuns1.empty())
    {
        dstRuns = srcRuns2;
        return;
    }

    if (srcRuns2.empty())
    {
        dstRuns = srcRuns1;
        return;
    }

    dstRuns.resize(srcRuns1.size()+ srcRuns2.size());
    RunSequence::const_pointer cur1 = srcRuns1.data();
    RunSequence::const_pointer cur2 = srcRuns2.data();
    const RunSequence::const_pointer end1 = srcRuns1.data() + srcRuns1.size();
    const RunSequence::const_pointer end2 = srcRuns2.data() + srcRuns2.size();

    constexpr int negInf = std::numeric_limits<int>::min();
    RunLength negInfRun{ negInf, negInf, negInf, 0 };
    RunSequence::pointer pResRun = dstRuns.data();
    RunSequence::pointer pPreRun = &negInfRun;

    while (cur1 != end1 && cur2 != end2)
    {
        RunSequence::const_pointer pNewRun = cur1;
        if (cur1->row < cur2->row)
        {
            ++cur1;
        }
        else if (cur2->row < cur1->row)
        {
            pNewRun = cur2;
            ++cur2;
        }
        else if (cur1->colb < cur2->colb)
        {
            ++cur1;
        }
        else if (cur2->colb < cur1->colb)
        {
            pNewRun = cur2;
            ++cur2;
        }
        else if (cur1->cole < cur2->cole)
        {
            ++cur1;
        }
        else if (cur2->cole < cur1->cole)
        {
            pNewRun = cur2;
            ++cur2;
        }
        else
        {
            ++cur1;
            ++cur2;
        }

        if (pNewRun->row == pPreRun->row && pNewRun->colb <= pPreRun->cole)
        {
            pPreRun->cole = std::max(pNewRun->cole, pPreRun->cole);
        }
        else
        {
            pResRun->row = pNewRun->row; pResRun->colb = pNewRun->colb; pResRun->cole = pNewRun->cole; pResRun->label = 0;
            pPreRun = pResRun;
            ++pResRun;
        }
    }

    for (; cur1 != end1; ++cur1)
    {
        if (cur1->row == pPreRun->row && cur1->colb <= pPreRun->cole)
        {
            pPreRun->cole = std::max(cur1->cole, pPreRun->cole);
        }
        else
        {
            pResRun->row = cur1->row; pResRun->colb = cur1->colb; pResRun->cole = cur1->cole; pResRun->label = 0;
            pPreRun = pResRun;
            ++pResRun;
        }
    }

    for (; cur2 != end2; ++cur2)
    {
        pResRun->row = cur2->row; pResRun->colb = cur2->colb; pResRun->cole = cur2->cole; pResRun->label = 0;
        if (cur2->row == pPreRun->row && cur2->colb <= pPreRun->cole)
        {
            pPreRun->cole = std::max(cur2->cole, pPreRun->cole);
        }
        else
        {
            pResRun->row = cur2->row; pResRun->colb = cur2->colb; pResRun->cole = cur2->cole; pResRun->label = 0;
            pPreRun = pResRun;
            ++pResRun;
        }
    }

    assert(std::distance(dstRuns.data(), pResRun) <= (std::ptrdiff_t)dstRuns.size());
    dstRuns.resize(std::distance(dstRuns.data(), pResRun));
}
}
}
