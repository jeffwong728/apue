#include "precomp.hpp"
#include "rtd_encoder.hpp"
#include "contour_impl.hpp"

namespace cv {
namespace mvlab {

void RDEncoder::Link()
{
    const int downLink_g[11][11]{ {0}, {0}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0}, {0}, {0}, {0}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1} };
    const int upLink_g[11][11]{ {0}, {0}, {0}, {0}, {0}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1} };

    int P3{ -1 };
    int P4{ 0 };
    int P5{ 0 };

    for (const RDEntry &rdEntry : rd_list_)
    {
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
    for (RDEntry &e : rd_list_)
    {
        if (!e.FLAG)
        {
            e.FLAG = 1;
            if (1 == e.CODE || 9 == e.CODE)
            {
                ScalablePoint2fSequence contour;
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

inline void RDEncoder::GenerateRDCodes(const ScalableIntSequence &P_BUFFER, const ScalableIntSequence &C_BUFFER, const int l)
{
    int P1 = 0;
    int P2 = 0;
    int State = 0;
    int X1 = P_BUFFER[P1];
    int X2 = C_BUFFER[P2];
    int X = X2;
    constexpr int Infinity = std::numeric_limits<int>::max();
    const int qis_g[11]{ 0, 2, 1, 1, 1, 0, 1, 1, 1, 2, 0 };

    bool stay = true;
    while (stay)
    {
        int RD_CODE = 0;
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
            rd_list_.emplace_back(X-0.5f, l-0.5f, RD_CODE, qis_g[RD_CODE]);
        }
    }
}

void RunLengthRDSerialEncoder::Encode(const RunSequence::const_pointer pRunBeg,
    const RunSequence::const_pointer pRunEnd,
    const RowBeginSequence &rranges)
{
    constexpr int Infinity = std::numeric_limits<int>::max();
    const int numRows = static_cast<int>(rranges.size()) - 1;
    const int lBeg = pRunBeg->row;
    const int lEnd = (pRunEnd-1)->row + 1;
    rd_list_.reserve(std::distance(pRunBeg, pRunEnd)*2);

    ScalableIntSequence P_BUFFER;
    ScalableIntSequence C_BUFFER;
    P_BUFFER.push_back(Infinity);

    int rIdx = 0;
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
        RDEncoder::GenerateRDCodes(P_BUFFER, C_BUFFER, l);
        P_BUFFER.swap(C_BUFFER);
        C_BUFFER.resize(0);
    }
}

class ParallelRDEncoder
{
    const RunSequence::const_pointer runBeg_;
    const RunSequence::const_pointer runEnd_;
    const RowBeginSequence &rranges_;
    const ScalableIntSequence::pointer numCodes_;

public:
    ParallelRDEncoder(const RunSequence::const_pointer runBeg,
        const RunSequence::const_pointer runEnd,
        const RowBeginSequence &rranges,
        const ScalableIntSequence::pointer numCodes)
        : runBeg_(runBeg)
        , runEnd_(runEnd)
        , rranges_(rranges) {}

public:
    void operator()(const tbb::blocked_range<int>& br) const;
};

void RunLengthRDParallelEncoder::Encode(const RunSequence::const_pointer pRunBeg,
    const RunSequence::const_pointer pRunEnd,
    const RowBeginSequence &rranges)
{
    const int numRows = static_cast<int>(rranges.size()) - 1;
    rd_list_.reserve(std::distance(pRunBeg, pRunEnd) * 2);
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
}

}
}
