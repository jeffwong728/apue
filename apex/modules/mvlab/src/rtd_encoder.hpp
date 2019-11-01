#ifndef __OPENCV_MVLAB_RTD_ENCODER_HPP__
#define __OPENCV_MVLAB_RTD_ENCODER_HPP__

#include "region_impl.hpp"

namespace cv {
namespace mvlab {

struct RDEntry
{
    RDEntry(const int x, const int y, const int code, const int qi)
        : X(x), Y(y), CODE(code), LINK(0), W_LINK(0), QI(qi), FLAG(0) {}
    int X, Y;
    int CODE;
    int LINK;
    int W_LINK;
    int QI;
    int FLAG;
};

using RDList = std::vector<RDEntry>;
using RDListList = std::vector<RDList>;

class RDEncoder
{
public:
    RDEncoder() {}
    void Link();

protected:
    inline void GenerateRDCodes(const std::vector<int> &P_BUFFER, const std::vector<int> &C_BUFFER, const int l);

protected:
    RDList rd_list_;
};

class RunLengthRDEncoder : public RDEncoder
{
public:
    RunLengthRDEncoder() : RDEncoder() {}

public:
    void Encode(const RunList &rgn_runs, const RowRunStartList &rranges);
};

}
}

#endif //__OPENCV_MVLAB_RTD_ENCODER_HPP__
