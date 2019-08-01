#ifndef SPAM_UI_PROC_PIXEL_TEMPLATE_H
#define SPAM_UI_PROC_PIXEL_TEMPLATE_H
#include "rgn.h"
#include <ui/cmndef.h>
#include <array>
#include <vector>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#include <boost/optional.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <tbb/scalable_allocator.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 4267 4244)
#include <2geom/path.h>
#include <2geom/pathvector.h>
#pragma warning( pop )

struct PixelTmplData
{
    std::vector<cv::Point> pixlLocs;
    std::vector<float>     pixlVals;
};

struct PixelTmplCreateData
{
    const cv::Mat &srcImg;
    const Geom::PathVector &tmplRgn;
    const Geom::PathVector &roi;
    const cv::Range &angleRange;
    const int pyramidLevel;
    const cv::TemplateMatchModes matchMode;
};

class PixelTemplate
{
public:
    PixelTemplate();
    ~PixelTemplate();

public:
    SpamResult CreatePixelTemplate(const PixelTmplCreateData &createData);

private:
    SpamResult calcCentreOfGravity(const PixelTmplCreateData &createData);

private:
    int pyramid_level_;
    cv::TemplateMatchModes match_mode_;
    std::vector<cv::Point2f> cgs_; // centre of gravity
    std::vector<SpamRgn>     tmpl_rgns_;
    std::vector<SpamRgn>     search_rois_;
    std::vector<PixelTmplData> tmpl_datas_;
    cv::Range angle_range_;
};

#endif //SPAM_UI_PROC_PIXEL_TEMPLATE_H