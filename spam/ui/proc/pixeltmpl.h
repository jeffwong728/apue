#ifndef SPAM_UI_PROC_PIXEL_TEMPLATE_H
#define SPAM_UI_PROC_PIXEL_TEMPLATE_H
#include "rgn.h"
#include <ui/errdef.h>
#include <array>
#include <vector>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#include <boost/optional.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/variant.hpp>
#include <tbb/scalable_allocator.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 4267 4244)
#include <2geom/path.h>
#include <2geom/pathvector.h>
#pragma warning( pop )

using PixelValueSequence = boost::variant<std::vector<int16_t>, std::vector<float>>;
struct PixelTmplData
{
    PixelTmplData(const float a, const float s) : angle(a), scale(s) {}
    PixelValueSequence     pixlVals;
    PointSet               pixlLocs;
    cv::Point              minPoint;
    cv::Point              maxPoint;
    float                  angle;
    float                  scale;
};

struct LayerTmplData
{
    LayerTmplData(const double as, const double ss) : angleStep(as), scaleStep(ss) {}
    double angleStep;
    double scaleStep;
    std::vector<PixelTmplData> tmplDatas;
};

struct PixelTmplCreateData
{
    const cv::Mat &srcImg;
    const Geom::PathVector &tmplRgn;
    const Geom::PathVector &roi;
    const int angleStart;
    const int angleExtent;
    const int pyramidLevel;
    const cv::TemplateMatchModes matchMode;
};

class PixelTemplate
{
public:
    PixelTemplate();
    ~PixelTemplate();

public:
    SpamResult matchTemplate(const cv::Mat &img, const int sad, cv::Point2f &pos, float &angle);
    SpamResult CreatePixelTemplate(const PixelTmplCreateData &createData);
    const std::vector<LayerTmplData> &GetTmplDatas() const { return pyramid_tmpl_datas_; }
    const cv::Mat &TopScoreMat() const { return top_layer_score_; }

private:
    void destroyData();
    SpamResult verifyCreateData(const PixelTmplCreateData &createData);
    SpamResult calcCentreOfGravity(const PixelTmplCreateData &createData);
    static uint8_t getMinMaxGrayScale(const cv::Mat &img, const PointSet &maskPoints, const cv::Point &point);

private:
    cv::Mat top_layer_score_;
    cv::Mat top_layer_angle_;

private:
    int pyramid_level_;
    cv::TemplateMatchModes match_mode_;
    std::vector<cv::Point2f> cfs_; // centre of referance
    std::vector<SpamRgn>     tmpl_rgns_;
    std::vector<SpamRgn>     search_rois_;
    std::vector<LayerTmplData> pyramid_tmpl_datas_;
    std::vector<cv::Mat> pyrs_;
};

#endif //SPAM_UI_PROC_PIXEL_TEMPLATE_H