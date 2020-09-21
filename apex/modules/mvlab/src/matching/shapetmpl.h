#ifndef SPAM_UI_PROC_SHAPE_TEMPLATE_H
#define SPAM_UI_PROC_SHAPE_TEMPLATE_H

#include "base_template.h"

namespace cv {
namespace mvlab {

struct ShapeClusterData
{
    ShapeClusterData() : label(0) {}
    int label;
    cv::Point2f center;
    cv::Point2f direction;

    template<class Archive>
    void save(Archive & ar, const unsigned int /*version*/) const
    {
        AnyMap fields;
        SET_ANY_FIELD(fields, label);
        SET_ANY_FIELD(fields, center);
        SET_ANY_FIELD(fields, direction);
        ar & BOOST_SERIALIZATION_NVP(fields);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int /*version*/)
    {
        AnyMap fields;
        ar & BOOST_SERIALIZATION_NVP(fields);

        GET_ANY_FIELD(fields, label);
        GET_ANY_FIELD(fields, center);
        GET_ANY_FIELD(fields, direction);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

using GradientSequence = std::vector<float>;
using ClusterSequence  = std::vector<ShapeClusterData>;

struct ShapeTemplData
{
    ShapeTemplData() : ShapeTemplData(0.f, 0.f) {}
    ShapeTemplData(const float a, const float s) : angle(a), scale(s) {}
    ScalableFloatSequence   gNXVals;
    ScalableFloatSequence   gNYVals;
    ScalablePointSequence   edgeLocs;
    ClusterSequence         clusters;
    ScalableIntSequence     mindices;
    cv::Point               minPoint;
    cv::Point               maxPoint;
    float                   angle;
    float                   scale;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int /*version*/)
    {
        ar & BOOST_SERIALIZATION_NVP(gNXVals);
        ar & BOOST_SERIALIZATION_NVP(gNYVals);
        ar & BOOST_SERIALIZATION_NVP(edgeLocs);
        ar & BOOST_SERIALIZATION_NVP(clusters);
        ar & BOOST_SERIALIZATION_NVP(mindices);
        ar & BOOST_SERIALIZATION_NVP(minPoint);
        ar & BOOST_SERIALIZATION_NVP(maxPoint);
        ar & BOOST_SERIALIZATION_NVP(angle);
        ar & BOOST_SERIALIZATION_NVP(scale);
    }
};

struct ShapeTmplCreateData : public BaseTmplCreateData
{
    ShapeTmplCreateData() {}
    int lowContrast;
    int highContrast;
};

struct ShapeTmplMatchOption
{
    float minScore;
    float greediness;
    int minContrast;
    bool touchBorder;
    cv::Ptr<Region> searchRgn;
};

struct LayerShapeData
{
    LayerShapeData() : LayerShapeData(0.f, 0.f) {}
    LayerShapeData(const float as, const float ss) : angleStep(as), scaleStep(ss) {}
    std::vector<ShapeTemplData> tmplDatas;
    float angleStep;
    float scaleStep;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int /*version*/)
    {
        ar & BOOST_SERIALIZATION_NVP(tmplDatas);
        ar & BOOST_SERIALIZATION_NVP(angleStep);
        ar & BOOST_SERIALIZATION_NVP(scaleStep);
    }
};

class ShapeTemplate : public BaseTemplate
{
public:
    template<bool TouchBorder> friend struct ShapeTopLayerScaner;
    template<bool TouchBorder> friend struct ShapeCandidateScaner;
    template<bool TouchBorder> friend struct DeformCandidateScaner;

public:
    ShapeTemplate();
    ~ShapeTemplate();

public:
    int matchShapeTemplate(const cv::Mat &img, const ShapeTmplMatchOption &smo, cv::Point2f &pos, float &angle, float &score);
    int CreateTemplate(const ShapeTmplCreateData &createData);
    cv::Mat GetTopScoreMat() const;
    void DumpTemplate(std::ostream &oss) const;
    void DrawInstance(cv::Mat &img, const cv::Point2f &pos, const float angle, const DrawTmplOptions &opts) const;

protected:
    void destroyData();
    int createShapeTemplate(const ShapeTmplCreateData &createData);
    void linkTemplatesBetweenLayers();
    float estimateSubPixelPose(const Candidate &bestCandidate, const float minScore, const int minContrast, const float greediness, cv::Point2f &pos, float &angle);
    void groupEdgePoints(ShapeTemplData &shptd) const;

protected:
    int low_contrast_;
    int high_contrast_;
    std::vector<LayerShapeData> pyramid_tmpl_datas_;
    cv::Mat top_layer_dx_;
    cv::Mat top_layer_dy_;
    std::vector<const float *> dx_row_ptrs_;
    std::vector<const float *> dy_row_ptrs_;
};
} 
}

#endif //SPAM_UI_PROC_SHAPE_TEMPLATE_H