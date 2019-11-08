#include "precomp.hpp"
#include "rtd_encoder.hpp"
#include "contour_impl.hpp"

namespace cv {
namespace mvlab {

RDEncoder::RDEncoder()
    : rd_list_(nullptr)
    , rd_list_end_(nullptr)
{
}

RDEncoder::~RDEncoder()
{
    ::mi_free(rd_list_);
}

void RDEncoder::Link()
{
    const int downLink_g[11][11]{ {0}, {0}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0}, {0}, {0}, {0}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1} };
    const int upLink_g[11][11]{ {0}, {0}, {0}, {0}, {0}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1} };

    int P3{ -1 };
    int P4{ 0 };
    int P5{ 0 };

    for (RDEntry *pRDEntry = rd_list_; pRDEntry != rd_list_end_; ++pRDEntry)
    {
        RDEntry &rdEntry = *pRDEntry;
        P3 += 1;
        const int RD_CODE = rdEntry.CODE;

        if (rdEntry.QI)
        {
            rd_list_[P5].W_LINK = P3;
            P5 = P3;
        }
        else
        {
            rd_list_[P5].W_LINK = P3 + 1;
        }

        if (5 == RD_CODE)
        {
            if (downLink_g[RD_CODE][rd_list_[P4].CODE])
            {
                rd_list_[P4].LINK = P3;
                rd_list_[P4].QI -= 1;
                if (1 > rd_list_[P4].QI)
                {
                    P4 = rd_list_[P4].W_LINK;
                }
            }

            if (upLink_g[RD_CODE][rd_list_[P4].CODE])
            {
                rd_list_[P3].LINK = P4;
                rd_list_[P4].QI -= 1;
                if (1 > rd_list_[P4].QI)
                {
                    P4 = rd_list_[P4].W_LINK;
                }
            }
        }
        else
        {
            if (upLink_g[RD_CODE][rd_list_[P4].CODE])
            {
                rd_list_[P3].LINK = P4;
                rd_list_[P4].QI -= 1;
                if (1 > rd_list_[P4].QI)
                {
                    P4 = rd_list_[P4].W_LINK;
                }
            }

            if (downLink_g[RD_CODE][rd_list_[P4].CODE])
            {
                rd_list_[P4].LINK = P3;
                rd_list_[P4].QI -= 1;
                if (1 > rd_list_[P4].QI)
                {
                    P4 = rd_list_[P4].W_LINK;
                }
            }
        }
    }
}

void RDEncoder::Track(std::vector<Ptr<Contour>> &outers, std::vector<Ptr<Contour>> &holes)
{
    const int count_g[11]{ 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1 };
    for (RDEntry *pRDEntry = rd_list_; pRDEntry != rd_list_end_; ++pRDEntry)
    {
        RDEntry &e = *pRDEntry;
        if (!e.FLAG)
        {
            e.FLAG = 1;
            if (1 == e.CODE || 9 == e.CODE)
            {
                ScalablePoint2fSequence contour;
                contour.reserve(64);
                contour.emplace_back(e.X, e.Y);
                int nextLink = e.LINK;
                while (!rd_list_[nextLink].FLAG)
                {
                    rd_list_[nextLink].FLAG = 1;
                    if (count_g[rd_list_[nextLink].CODE])
                    {
                        contour.emplace_back(contour.back().x, rd_list_[nextLink].Y);
                        contour.emplace_back(rd_list_[nextLink].X, rd_list_[nextLink].Y);
                    }
                    nextLink = rd_list_[nextLink].LINK;
                }

                if (!contour.empty())
                {
                    contour.emplace_back(contour.back().x, contour.front().y);

                    if (1 == e.CODE)
                    {
                        outers.push_back(cv::makePtr<ContourImpl>(&contour, true));
                    }
                    else
                    {
                        holes.push_back(cv::makePtr<ContourImpl>(&contour, true));
                    }
                }
            }
        }
    }
}

inline void RDEncoder::GenerateRDCodes(const ScalableIntSequence &P_BUFFER,
    const ScalableIntSequence &C_BUFFER,
    const int l,
    RDEntry *&pRDList)
{
    int P1 = 0;
    int P2 = 0;
    int State = 0;
    int X1 = P_BUFFER[P1];
    int X2 = C_BUFFER[P2];
    int X = X2;
    constexpr int Infinity = std::numeric_limits<int>::max();
    const int8_t qis_g[11]{ 0, 2, 1, 1, 1, 0, 1, 1, 1, 2, 0 };

    bool stay = true;
    while (stay)
    {
        int8_t RD_CODE = 0;
        switch (State)
        {
        case 0:
            if (X1 > X2) {
                State = 2; X = X2; P2 += 1; X2 = C_BUFFER[P2];
            }
            else if (X1 < X2) {
                State = 1; P1 += 1; X1 = P_BUFFER[P1];
            }
            else if (X1 < Infinity) {
                State = 3; RD_CODE = 2; P1 += 1; X1 = P_BUFFER[P1]; P2 += 1; X2 = C_BUFFER[P2];
            }
            else {
                stay = false;
            }
            break;

        case 1:
            if (X1 > X2) {
                State = 3; X = X2; RD_CODE = 4; P2 += 1; X2 = C_BUFFER[P2];
            }
            else if (X1 < X2) {
                State = 0; X = X1; RD_CODE = 5; P1 += 1; X1 = P_BUFFER[P1];
            }
            else {
                State = 4; X = X1; RD_CODE = 4; P1 += 1; X1 = P_BUFFER[P1]; P2 += 1; X2 = C_BUFFER[P2];
            }
            break;

        case 2:
            if (X1 > X2) {
                State = 0; RD_CODE = 1; P2 += 1; X2 = C_BUFFER[P2];
            }
            else if (X1 < X2) {
                State = 3; RD_CODE = 3; P1 += 1; X1 = P_BUFFER[P1];
            }
            else {
                State = 5; RD_CODE = 3; P1 += 1; X1 = P_BUFFER[P1]; P2 += 1; X2 = C_BUFFER[P2];
            }
            break;

        case 3:
            if (X1 > X2) {
                State = 5; P2 += 1; X2 = C_BUFFER[P2];
            }
            else if (X1 < X2) {
                State = 4; X = X1; P1 += 1; X1 = P_BUFFER[P1];
            }
            else {
                State = 0; RD_CODE = 6; P1 += 1; X1 = P_BUFFER[P1]; P2 += 1; X2 = C_BUFFER[P2];
            }
            break;

        case 4:
            if (X1 > X2) {
                State = 0; RD_CODE = 8; P2 += 1; X2 = C_BUFFER[P2];
            }
            else if (X1 < X2) {
                State = 3; RD_CODE = 10; P1 += 1; X1 = P_BUFFER[P1];
            }
            else {
                State = 5; RD_CODE = 10; P1 += 1; X1 = P_BUFFER[P1]; P2 += 1; X2 = C_BUFFER[P2];
            }
            break;

        case 5:
            if (X1 > X2) {
                State = 3; X = X2; RD_CODE = 9; P2 += 1; X2 = C_BUFFER[P2];
            }
            else if (X1 < X2) {
                State = 0; X = X1; RD_CODE = 7; P1 += 1; X1 = P_BUFFER[P1];
            }
            else {
                State = 4; X = X1; RD_CODE = 9; P1 += 1; X1 = P_BUFFER[P1]; P2 += 1; X2 = C_BUFFER[P2];
            }
            break;

        default:
            break;
        }

        if (RD_CODE)
        {
            pRDList->X = X - 0.5f;
            pRDList->Y = l - 0.5f;
            pRDList->CODE = RD_CODE;
            pRDList->QI = qis_g[RD_CODE];
            pRDList += 1;
        }
    }
}

void RunLengthRDSerialEncoder::Encode(const RunSequence::const_pointer pRunBeg,
    const RunSequence::const_pointer pRunEnd,
    const RowBeginSequence &rranges)
{
    constexpr int Infinity = std::numeric_limits<int>::max();
    const int numRows = static_cast<int>(rranges.size()) - 1;
    rd_list_ = static_cast<RDEntry*>(::mi_malloc(std::distance(pRunBeg, pRunEnd) * 2 * sizeof(RDEntry)));
    rd_list_end_ = rd_list_ + std::distance(pRunBeg, pRunEnd) * 2;
    std::memset(rd_list_, 0, std::distance(pRunBeg, pRunEnd) * 2 * sizeof(RDEntry));

    const int lBeg = pRunBeg->row;
    const int lEnd = (pRunEnd - 1)->row + 1;

    ScalableIntSequence P_BUFFER;
    ScalableIntSequence C_BUFFER;
    P_BUFFER.push_back(Infinity);

    int rIdx = 0;
    RDEntry* pRDList = rd_list_;
    for (int l = lBeg; l <= lEnd; ++l)
    {
        if (rIdx < numRows)
        {
            const RunSequence::const_pointer pRowRunBeg = pRunBeg + rranges[rIdx];
            const RunSequence::const_pointer pRowRunEnd = pRunBeg + rranges[rIdx+1];
            if (l == pRowRunBeg->row)
            {
                for (RunSequence::const_pointer pRun = pRowRunBeg; pRun != pRowRunEnd; ++pRun)
                {
                    C_BUFFER.push_back(pRun->colb);
                    C_BUFFER.push_back(pRun->cole);
                }
                rIdx += 1;
            }
        }

        C_BUFFER.push_back(Infinity);
        RDEncoder::GenerateRDCodes(P_BUFFER, C_BUFFER, l, pRDList);
        P_BUFFER.swap(C_BUFFER);
        C_BUFFER.resize(0);
    }
}

class ParallelRDEncoder
{
    const RunSequence::const_pointer runBeg_;
    const RunSequence::const_pointer runEnd_;
    const RowBeginSequence &rranges_;
    const ScalableIntSequence::const_pointer numCodes_;
    RDEntry * const rdList_;

public:
    ParallelRDEncoder(const RunSequence::const_pointer runBeg,
        const RunSequence::const_pointer runEnd,
        const RowBeginSequence &rranges,
        const ScalableIntSequence::pointer numCodes,
        RDEntry * const rdList)
        : runBeg_(runBeg)
        , runEnd_(runEnd)
        , rranges_(rranges)
        , numCodes_(numCodes)
        , rdList_(rdList)
    {}

public:
    void operator()(const tbb::blocked_range<int>& br) const;
};

void ParallelRDEncoder::operator()(const tbb::blocked_range<int>& br) const
{
    constexpr int Infinity = std::numeric_limits<int>::max();
    const int numRuns = static_cast<int>(rranges_.size()) - 1;
    const int lBeg = runBeg_[rranges_[br.begin()]].row;
    const int lEnd = (br.end() < numRuns) ? runBeg_[rranges_[br.end()]].row - 1 : runBeg_[rranges_[br.end() - 1]].row + 1;
    RDEntry* pRDList = rdList_ + numCodes_[lBeg - runBeg_->row];

    ScalableIntSequence P_BUFFER;
    ScalableIntSequence C_BUFFER;

    if (br.begin() > 0)
    {
        const int rBeg = rranges_[br.begin() - 1];
        const int rEnd = rranges_[br.begin()];
        if (runBeg_[rBeg].row == (lBeg - 1))
        {
            P_BUFFER.reserve((rEnd- rBeg)*2+1);
            for (int rIdx = rBeg; rIdx < rEnd; ++rIdx)
            {
                P_BUFFER.push_back(runBeg_[rIdx].colb);
                P_BUFFER.push_back(runBeg_[rIdx].cole);
            }
        }
    }

    P_BUFFER.push_back(Infinity);

    int rIdx = br.begin();
    for (int l = lBeg; l <= lEnd; ++l)
    {
        if (rIdx < numRuns)
        {
            const int rBeg = rranges_[rIdx];
            const int rEnd = rranges_[rIdx + 1];
            if (l == runBeg_[rBeg].row)
            {
                C_BUFFER.reserve((rEnd - rBeg) * 2 + 1);
                for (int runIdx = rBeg; runIdx < rEnd; ++runIdx)
                {
                    C_BUFFER.push_back(runBeg_[runIdx].colb);
                    C_BUFFER.push_back(runBeg_[runIdx].cole);
                }
                rIdx += 1;
            }
        }

        C_BUFFER.push_back(Infinity);
        RDEncoder::GenerateRDCodes(P_BUFFER, C_BUFFER, l, pRDList);
        P_BUFFER.swap(C_BUFFER);
        C_BUFFER.resize(0);
    }
}

void RunLengthRDParallelEncoder::Track(std::vector<Ptr<Contour>> &outers, std::vector<Ptr<Contour>> &holes)
{
    RDEncoder::Track(outers, holes);
}

void RunLengthRDParallelEncoder::Encode(const RunSequence::const_pointer pRunBeg,
    const RunSequence::const_pointer pRunEnd,
    const RowBeginSequence &rranges)
{
    const int numRows = static_cast<int>(rranges.size()) - 1;
    rd_list_ = static_cast<RDEntry*>(::mi_malloc(std::distance(pRunBeg, pRunEnd) * 2 * sizeof(RDEntry)));
    rd_list_end_ = rd_list_ + std::distance(pRunBeg, pRunEnd) * 2;
    std::memset(rd_list_, 0, std::distance(pRunBeg, pRunEnd) * 2 * sizeof(RDEntry));
    const int lBeg = pRunBeg->row;
    const int lEnd = (pRunEnd - 1)->row + 1;

    ScalableIntSequence numCodes(lEnd - lBeg + 2);
    ScalableIntSequence::pointer pNumCodes = numCodes.data() + 1;

    int rIdx = 0, P_NUMRUNS = 0;
    for (int l = lBeg; l <= lEnd; ++l, ++pNumCodes)
    {
        int C_NUMRUNS = 0;
        if (rIdx < numRows)
        {
            const RunSequence::const_pointer pRowRunBeg = pRunBeg + rranges[rIdx];
            if (l == pRowRunBeg->row)
            {
                C_NUMRUNS = rranges[rIdx + 1] - rranges[rIdx];
                rIdx += 1;
            }
        }

        *pNumCodes = P_NUMRUNS + C_NUMRUNS + *(pNumCodes-1);
        P_NUMRUNS = C_NUMRUNS;
    }

    ParallelRDEncoder parallelRDEncoder(pRunBeg, pRunEnd, rranges, numCodes.data(), rd_list_);
    tbb::parallel_for(tbb::blocked_range<int>(0, numRows), parallelRDEncoder);
}

}
}
