#ifndef SPAM_UI_PROC_PIXEL_TEMPLATE_H
#define SPAM_UI_PROC_PIXEL_TEMPLATE_H
#include "rgn.h"
#include "basetmpl.h"

using PixelValueSequence = std::vector<int16_t>;
using NCCValueSequence = std::vector<uint8_t>;
using Point3iSet = std::vector<cv::Point3i>;
using NCCValuePairSequence = std::vector<std::pair<uint8_t, uint8_t>>;
using PointPairSet = std::vector<std::pair<cv::Point, cv::Point>>;
using RegularNCCValues = std::vector<uint32_t, tbb::scalable_allocator<uint32_t>>;

enum TemplPart
{
    kTP_WaveMiddle = 0,
    kTP_WaveBack = 1,
    kTP_WaveFront = 2,
    kTP_WaveGuard = 3
};

struct PixelTemplData
{
    PixelTemplData(const float a, const float s) : angle(a), scale(s) {}
    PixelValueSequence pixlVals;
    PointSet           pixlLocs;
    std::vector<int>   mindices;
    cv::Point          minPoint;
    cv::Point          maxPoint;
    float              angle;
    float              scale;
};

struct NCCTemplData
{
    NCCTemplData(const float a, const float s)
        : angle(a)
        , scale(s)
        , partASum(0)
        , partBSum(0)
        , partASqrSum(0)
        , partBSqrSum(0)
        , cPartABoundaries(0)
        , betaz(0)
        , norm(0) {}

    Point3iSet           partALocs;
    NCCValueSequence     partAVals;
    Point3iSet           partBLocs;
    NCCValueSequence     partBVals;
    PointPairSet         partBBoundaries;
    std::vector<int>     mindices;
    cv::Point            minPoint;
    cv::Point            maxPoint;
    int64_t              partASum;
    int64_t              partBSum;
    int64_t              partASqrSum;
    int64_t              partBSqrSum;
    double               betaz;
    double               norm;
    int                  cPartABoundaries;
    float                angle;
    float                scale;
};

struct BruteForceNCCTemplData
{
    BruteForceNCCTemplData(const float a, const float s)
        : angle(a)
        , scale(s)
        , sum(0)
        , sqrSum(0)
        , norm(0) {}

    PointSet         locs;
    NCCValueSequence vals;
    RegularNCCValues regVals;
    std::vector<int> mindices;
    cv::Point        minPoint;
    cv::Point        maxPoint;
    int64_t          sum;
    int64_t          sqrSum;
    double           norm;
    float            angle;
    float            scale;
};

using PixelTemplDatas = std::vector<PixelTemplData>;
using NCCTemplDatas = std::vector<NCCTemplData>;
using BFNCCTemplDatas = std::vector<BruteForceNCCTemplData>;
using TemplDataSequence = boost::variant<PixelTemplDatas, NCCTemplDatas, BFNCCTemplDatas>;
struct LayerTemplData
{
    LayerTemplData(const float as, const float ss) : angleStep(as), scaleStep(ss) {}
    float angleStep;
    float scaleStep;
    TemplDataSequence tmplDatas;
};

struct PixelTmplCreateData
{
    const BaseTmplCreateData baseData;
    const cv::TemplateMatchModes matchMode;
};

class PixelTemplate : public BaseTemplate
{
public:
    friend struct SADTopLayerScaner;
    friend struct SADCandidateScaner;
    friend struct NCCTopLayerScaner;
    template<bool TouchBorder> friend struct BFNCCTopLayerScaner;
    template<bool TouchBorder> friend struct BFNCCCandidateScaner;

public:
    PixelTemplate();
    ~PixelTemplate();

public:
    SpamResult matchPixelTemplate(const cv::Mat &img, const int sad, cv::Point2f &pos, float &angle);
    SpamResult matchNCCTemplate(const cv::Mat &img, const float minScore, cv::Point2f &pos, float &angle, float &score);
    SpamResult CreateTemplate(const PixelTmplCreateData &createData);
    const std::vector<LayerTemplData> &GetTmplDatas() const { return pyramid_tmpl_datas_; }
    cv::Mat GetTopScoreMat() const;
    cv::Point2f GetCenter() const { return cfs_.front(); }

private:
    void destroyData();
    SpamResult calcCreateTemplate(const PixelTmplCreateData &createData);
    SpamResult fastCreateTemplate(const PixelTmplCreateData &createData);
    void linkTemplatesBetweenLayers();
    static uint8_t getMinMaxGrayScale(const cv::Mat &img, const PointSet &maskPoints, const cv::Point &point);
    SpamResult changeToNCCTemplate();
    SpamResult changeToBruteForceNCCTemplate();
    static double calcValue1(const int64_t T, const int64_t S, const int64_t n);
    static double calcValue2(const int64_t T, const int64_t S, const int64_t Sp, const int64_t n, const int64_t np);

private:
    cv::TemplateMatchModes match_mode_;
    std::vector<LayerTemplData> pyramid_tmpl_datas_;
};

inline double PixelTemplate::calcValue1(const int64_t T, const int64_t S, const int64_t n)
{
    int64_t A = n * T;
    int64_t B = S * S;
    if (A <= B)
    {
        return -1.0;
    }
    else
    {
        return std::sqrt(static_cast<double>(A - B)) / std::sqrt(static_cast<double>(n));
    }
}

inline double PixelTemplate::calcValue2(const int64_t Tp, const int64_t S, const int64_t Sp, const int64_t n, const int64_t np)
{
    int64_t A = n * n * Tp;
    int64_t B = np * S * S;
    int64_t C = 2 * n * S * Sp;
    int64_t D = A + B - C;
    if (D <= 0)
    {
        return -1.0;
    }
    else
    {
        return std::sqrt(D) / static_cast<double>(n);
    }
}
#endif //SPAM_UI_PROC_PIXEL_TEMPLATE_H