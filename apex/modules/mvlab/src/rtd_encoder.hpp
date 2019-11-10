#ifndef __OPENCV_MVLAB_RTD_ENCODER_HPP__
#define __OPENCV_MVLAB_RTD_ENCODER_HPP__

#include "region_impl.hpp"
#include <opencv2/mvlab/contour.hpp>

namespace cv {
namespace mvlab {

struct RDEntry
{
    float X, Y;
    int LINK;
    int W_LINK;
    int8_t CODE;
    int8_t QI;
    int8_t FLAG;
};

class RDEncoder
{
public:
    RDEncoder();
    ~RDEncoder();
    void Link();
    void Track(std::vector<Ptr<Contour>> &outers, std::vector<Ptr<Contour>> &holes);
    void Track(Ptr<Contour> &outer);

public:
    static inline void GenerateRDCodes(const ScalableIntSequence &P_BUFFER, const ScalableIntSequence &C_BUFFER, const int l, RDEntry *&pRDList);

protected:
    RDEntry *rd_list_;
    RDEntry *rd_list_end_;
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
    void Track(std::vector<Ptr<Contour>> &outers, std::vector<Ptr<Contour>> &holes);
    void Track(Ptr<Contour> &outer);
    void Encode(const RunSequence::const_pointer pRunBeg, const RunSequence::const_pointer pRunEnd, const RowBeginSequence &rranges);
};

}
}

#endif //__OPENCV_MVLAB_RTD_ENCODER_HPP__
