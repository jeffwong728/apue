#ifndef SPAM_UI_PROC_SHAPE_TEMPLATE_H
#define SPAM_UI_PROC_SHAPE_TEMPLATE_H
#include "rgn.h"
#include "basetmpl.h"

using GradientSequence = std::vector<float>;
struct ShapeTemplData
{
    ShapeTemplData(const float a, const float s) : angle(a), scale(s) {}
    GradientSequence gNXVals;
    GradientSequence gNYVals;
    PointSet         edgeLocs;
    std::vector<int> mindices;
    cv::Point        minPoint;
    cv::Point        maxPoint;
    float            angle;
    float            scale;
};

struct ShapeTmplCreateData
{
    BaseTmplCreateData baseData;
    const int lowContrast;
    const int highContrast;
};

struct LayerShapeData
{
    LayerShapeData(const float as, const float ss) : angleStep(as), scaleStep(ss) {}
    float angleStep;
    float scaleStep;
    std::vector<ShapeTemplData> tmplDatas;
};

class ShapeTemplate : public BaseTemplate
{
public:
    template<bool TouchBorder> friend struct ShapeTopLayerScaner;
    template<bool TouchBorder> friend struct ShapeCandidateScaner;

public:
    ShapeTemplate();
    ~ShapeTemplate();

public:
    SpamResult matchShapeTemplate(const cv::Mat &img, const float minScore, cv::Point2f &pos, float &angle, float &score);
    SpamResult CreateTemplate(const ShapeTmplCreateData &createData);
    const std::vector<LayerShapeData> &GetTmplDatas() const { return pyramid_tmpl_datas_; }
    cv::Mat GetTopScoreMat() const;

private:
    void destroyData();
    SpamResult createShapeTemplate(const ShapeTmplCreateData &createData);
    void linkTemplatesBetweenLayers();

private:
    std::vector<LayerShapeData> pyramid_tmpl_datas_;
};
#endif //SPAM_UI_PROC_SHAPE_TEMPLATE_H