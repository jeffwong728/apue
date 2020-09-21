#ifndef SPAM_UI_PROC_PIXEL_TEMPLATE_H
#define SPAM_UI_PROC_PIXEL_TEMPLATE_H

#include "base_template.h"

namespace cv {
namespace mvlab {

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
    friend class boost::serialization::access;
    PixelTemplData(const float a, const float s) : angle(a), scale(s) {}
    PixelValueSequence    pixlVals;
    ScalablePointSequence pixlLocs;
    std::vector<int>      mindices;
    cv::Point             minPoint;
    cv::Point             maxPoint;
    float                 angle;
    float                 scale;

private:
    PixelTemplData() : PixelTemplData(0.f, 0.f) {}

    template<class Archive>
    void serialize(Archive & ar, const unsigned int /*version*/)
    {
        ar & BOOST_SERIALIZATION_NVP(pixlVals);
        ar & BOOST_SERIALIZATION_NVP(pixlLocs);
        ar & BOOST_SERIALIZATION_NVP(mindices);
        ar & BOOST_SERIALIZATION_NVP(minPoint);
        ar & BOOST_SERIALIZATION_NVP(maxPoint);
        ar & BOOST_SERIALIZATION_NVP(angle);
        ar & BOOST_SERIALIZATION_NVP(scale);
    }
};

struct NCCTemplData
{
    friend class boost::serialization::access;
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

    Point3iSet       partALocs;
    NCCValueSequence partAVals;
    Point3iSet       partBLocs;
    NCCValueSequence partBVals;
    PointPairSet     partBBoundaries;
    std::vector<int> mindices;
    cv::Point        minPoint;
    cv::Point        maxPoint;
    int64_t          partASum;
    int64_t          partBSum;
    int64_t          partASqrSum;
    int64_t          partBSqrSum;
    double           betaz;
    double           norm;
    int              cPartABoundaries;
    float            angle;
    float            scale;

private:
    NCCTemplData() : NCCTemplData(0.f, 0.f) {}

    template<class Archive>
    void serialize(Archive & ar, const unsigned int /*version*/)
    {
        ar & BOOST_SERIALIZATION_NVP(partALocs);
        ar & BOOST_SERIALIZATION_NVP(partAVals);
        ar & BOOST_SERIALIZATION_NVP(partBLocs);
        ar & BOOST_SERIALIZATION_NVP(partBVals);
        ar & BOOST_SERIALIZATION_NVP(partBBoundaries);
        ar & BOOST_SERIALIZATION_NVP(mindices);
        ar & BOOST_SERIALIZATION_NVP(minPoint);
        ar & BOOST_SERIALIZATION_NVP(maxPoint);
        ar & BOOST_SERIALIZATION_NVP(partASum);
        ar & BOOST_SERIALIZATION_NVP(partBSum);
        ar & BOOST_SERIALIZATION_NVP(partASqrSum);
        ar & BOOST_SERIALIZATION_NVP(partBSqrSum);
        ar & BOOST_SERIALIZATION_NVP(betaz);
        ar & BOOST_SERIALIZATION_NVP(norm);
        ar & BOOST_SERIALIZATION_NVP(cPartABoundaries);
        ar & BOOST_SERIALIZATION_NVP(angle);
        ar & BOOST_SERIALIZATION_NVP(scale);
    }
};

struct BruteForceNCCTemplData
{
    friend class boost::serialization::access;
    BruteForceNCCTemplData(const float a, const float s)
        : angle(a)
        , scale(s)
        , sum(0)
        , sqrSum(0)
        , norm(0) {}

    ScalablePointSequence locs;
    NCCValueSequence      vals;
    RegularNCCValues      regVals;
    std::vector<int>      mindices;
    cv::Point             minPoint;
    cv::Point             maxPoint;
    int64_t               sum;
    int64_t               sqrSum;
    double                norm;
    float                 angle;
    float                 scale;

private:
    BruteForceNCCTemplData() : BruteForceNCCTemplData(0.f, 0.f) {}

    template<class Archive>
    void serialize(Archive & ar, const unsigned int /*version*/)
    {
        ar & BOOST_SERIALIZATION_NVP(locs);
        ar & BOOST_SERIALIZATION_NVP(vals);
        ar & BOOST_SERIALIZATION_NVP(regVals);
        ar & BOOST_SERIALIZATION_NVP(mindices);
        ar & BOOST_SERIALIZATION_NVP(minPoint);
        ar & BOOST_SERIALIZATION_NVP(maxPoint);
        ar & BOOST_SERIALIZATION_NVP(sum);
        ar & BOOST_SERIALIZATION_NVP(sqrSum);
        ar & BOOST_SERIALIZATION_NVP(norm);
        ar & BOOST_SERIALIZATION_NVP(angle);
        ar & BOOST_SERIALIZATION_NVP(scale);
    }
};

using PixelTemplDatas = std::vector<PixelTemplData>;
using NCCTemplDatas = std::vector<NCCTemplData>;
using BFNCCTemplDatas = std::vector<BruteForceNCCTemplData>;
using TemplDataSequence = boost::variant<PixelTemplDatas, NCCTemplDatas, BFNCCTemplDatas>;
struct LayerTemplData
{
    friend class boost::serialization::access;
    LayerTemplData(const float as, const float ss) : angleStep(as), scaleStep(ss) {}
    float angleStep;
    float scaleStep;
    TemplDataSequence tmplDatas;

private:
    LayerTemplData() : LayerTemplData(0.f, 0.f) {}

    template<class Archive>
    void serialize(Archive & ar, const unsigned int /*version*/)
    {
        ar & BOOST_SERIALIZATION_NVP(angleStep);
        ar & BOOST_SERIALIZATION_NVP(scaleStep);
        ar & BOOST_SERIALIZATION_NVP(tmplDatas);
    }
};

struct PixelTmplCreateData : public BaseTmplCreateData
{
    PixelTmplCreateData() : matchMode("sad") {}
    cv::String matchMode;
};

class PixelTmpl : public BaseTemplate
{
public:
    friend struct SADTopLayerScaner;
    friend struct SADCandidateScaner;
    friend struct NCCTopLayerScaner;
    template<bool TouchBorder> friend struct BFNCCTopLayerScaner;
    template<bool TouchBorder> friend struct BFNCCCandidateScaner;

public:
    PixelTmpl();
    ~PixelTmpl();

public:
    int matchPixelTemplate(const cv::Mat &img, const int sad, cv::Point2f &pos, float &angle, float &score);
    int matchNCCTemplate(const cv::Mat &img, const float minScore, cv::Point2f &pos, float &angle, float &score);
    int CreateTemplate(const PixelTmplCreateData &createData);
    const std::vector<LayerTemplData> &GetTmplDatas() const { return pyramid_tmpl_datas_; }
    cv::Mat GetTopScoreMat() const;
    cv::Point2f GetCenter() const { return cfs_.front(); }
    void DrawInstance(cv::Mat &img, const cv::Point2f &pos, const float angle, const DrawTmplOptions &opts) const;

protected:
    void destroyData();
    int calcCreateTemplate(const PixelTmplCreateData &createData);
    int fastCreateTemplate(const PixelTmplCreateData &createData);
    void linkTemplatesBetweenLayers();
    static uint8_t getMinMaxGrayScale(const cv::Mat &img, const ScalablePointSet &maskPoints, const cv::Point &point);
    int changeToNCCTemplate();
    int changeToBruteForceNCCTemplate();
    static double calcValue1(const int64_t T, const int64_t S, const int64_t n);
    static double calcValue2(const int64_t T, const int64_t S, const int64_t Sp, const int64_t n, const int64_t np);

protected:
    cv::String match_mode_;
    std::vector<LayerTemplData> pyramid_tmpl_datas_;
};

inline double PixelTmpl::calcValue1(const int64_t T, const int64_t S, const int64_t n)
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

inline double PixelTmpl::calcValue2(const int64_t Tp, const int64_t S, const int64_t Sp, const int64_t n, const int64_t np)
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
}
}

#endif //SPAM_UI_PROC_PIXEL_TEMPLATE_H