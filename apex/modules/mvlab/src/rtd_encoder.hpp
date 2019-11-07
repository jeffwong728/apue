#ifndef __OPENCV_MVLAB_RTD_ENCODER_HPP__
#define __OPENCV_MVLAB_RTD_ENCODER_HPP__

#include "region_impl.hpp"
#include <opencv2/mvlab/contour.hpp>

namespace cv {
namespace mvlab {

struct RDEntry
{
    RDEntry(const float x, const float y, const int code, const int qi)
        : X(x), Y(y), CODE(code), LINK(0), W_LINK(0), QI(qi), FLAG(0) {}
    float X, Y;
    int CODE;
    int LINK;
    int W_LINK;
    int QI;
    int FLAG;
};

using RDList = std::vector<RDEntry, MyAlloc<RDEntry>>;

class RDEncoder
{
public:
    RDEncoder() {}
    void Link();
    void Track(std::vector<Ptr<Contour>> &outers, std::vector<Ptr<Contour>> &holes);

protected:
    inline void GenerateRDCodes(const ScalableIntSequence &P_BUFFER, const ScalableIntSequence &C_BUFFER, const int l);

protected:
    RDList rd_list_;
};

class RunLengthRDSerialEncoder : public RDEncoder
{
public:
    RunLengthRDSerialEncoder() : RDEncoder() {}

public:
    void Encode(const RunSequence::const_pointer pRunBeg, const RunSequence::const_pointer pRunEnd, const RowBeginSequence &rranges);
};

class RunLengthRDParallelEncoder : public RDEncoder
{
public:
    RunLengthRDParallelEncoder() : RDEncoder() {}

public:
    void Encode(const RunSequence::const_pointer pRunBeg, const RunSequence::const_pointer pRunEnd, const RowBeginSequence &rranges);
};

}
}

#endif //__OPENCV_MVLAB_RTD_ENCODER_HPP__
