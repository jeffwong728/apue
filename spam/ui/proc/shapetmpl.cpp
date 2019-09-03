#include "shapetmpl.h"
#include "basic.h"
#include <limits>
#include <stack>
#include <vectorclass/vectorclass.h>
#include <boost/container/static_vector.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 4267 4244 )
#include <2geom/path-sink.h>
#include <2geom/path-intersection.h>
#pragma warning( pop )
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>

ShapeTemplate::ShapeTemplate()
{
}

ShapeTemplate::~ShapeTemplate()
{ 
}

SpamResult ShapeTemplate::matchShapeTemplate(const cv::Mat &img, const float minScore, cv::Point2f &pos, float &angle, float &score)
{
    return SpamResult::kSR_TM_INSTANCE_NOT_FOUND;
}

SpamResult ShapeTemplate::CreateTemplate(const ShapeTmplCreateData &createData)
{
    destroyData();
    SpamResult sr = verifyCreateData(createData.baseData);
    if (SpamResult::kSR_SUCCESS != sr)
    {
        destroyData();
        return sr;
    }

    sr = createShapeTemplate(createData);
    if (SpamResult::kSR_SUCCESS != sr)
    {
        destroyData();
        return sr;
    }

    linkTemplatesBetweenLayers();

    pyramid_level_ = createData.baseData.pyramidLevel;
    processToplayerSearchROI(createData.baseData.roi, createData.baseData.pyramidLevel);

    return SpamResult::kSR_SUCCESS;
}

cv::Mat ShapeTemplate::GetTopScoreMat() const
{
    cv::Mat scoreMat(pyrs_.back().rows, pyrs_.back().cols, CV_8UC1, cv::Scalar());
    return scoreMat;
}

void ShapeTemplate::destroyData()
{
    destroyBaseData();
    pyramid_tmpl_datas_.clear();
}

SpamResult ShapeTemplate::createShapeTemplate(const ShapeTmplCreateData &createData)
{
    return SpamResult::kSR_OK;
}

void ShapeTemplate::linkTemplatesBetweenLayers()
{
    const int topLayerIndex = static_cast<int>(pyramid_tmpl_datas_.size() - 1);
    for (int layer = topLayerIndex; layer > 0; --layer)
    {
        LayerShapeData &ltd = pyramid_tmpl_datas_[layer];
        LayerShapeData &belowLtd = pyramid_tmpl_datas_[layer - 1];
        auto &tmplDatas = ltd.tmplDatas;
        const auto &belowTmplDatas = belowLtd.tmplDatas;
        for (ShapeTemplData &ptd : tmplDatas)
        {
            AngleRange<float> angleRange(ptd.angle - ltd.angleStep, ptd.angle + ltd.angleStep);
            for (int t = 0; t < belowTmplDatas.size(); ++t)
            {
                if (angleRange.contains(belowTmplDatas[t].angle))
                {
                    ptd.mindices.push_back(t);
                }
            }
        }
    }
}