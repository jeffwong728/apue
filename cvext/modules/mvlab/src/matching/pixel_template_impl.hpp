#ifndef __OPENCV_MVLAB_PIXEL_TEMPLATE_IMPL_HPP__
#define __OPENCV_MVLAB_PIXEL_TEMPLATE_IMPL_HPP__

#include <opencv2/mvlab/pixel_template.hpp>
#include <matching/pixeltmpl.h>

namespace cv {
namespace mvlab {

class PixelTemplateImpl : public PixelTemplate, public PixelTmpl, public std::enable_shared_from_this<PixelTemplateImpl>
{
    friend class boost::serialization::access;
public:
    PixelTemplateImpl() {}
    PixelTemplateImpl(const std::string &bytes);

public:
    bool Empty() const CV_OVERRIDE;
    cv::String GetErrorStatus() const CV_OVERRIDE;
    int GetPyramidLevel() const CV_OVERRIDE;
    int Create(cv::InputArray img, const cv::Ptr<Region> &rgn, const cv::Ptr<Dict> &opts) CV_OVERRIDE;
    cv::Ptr<MatchResult> Search(cv::InputArray img, const cv::Ptr<Dict> &opts) const CV_OVERRIDE;
    int Draw(cv::InputOutputArray img, const cv::Ptr<Dict> &opts) const CV_OVERRIDE;
    int Save(const cv::String &fileName, const cv::Ptr<Dict> &opts) const CV_OVERRIDE;

public:
    int Load(const cv::String &fileName, const cv::Ptr<Dict> &opts);
    int Serialize(const cv::String &name, H5Group *g) const;
    static const cv::String &TypeGUID() { return type_guid_s; }

private:
    void GetTemplateOptions(const cv::Ptr<Dict> &opts, PixelTmplCreateData &stcd);

private:
    static const cv::String type_guid_s;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int /*version*/)
    {
        ar & boost::serialization::make_nvp("pyramid_level",       pyramid_level_);
        ar & boost::serialization::make_nvp("angle_start",         angle_start_);
        ar & boost::serialization::make_nvp("angle_extent",        angle_extent_);
        ar & boost::serialization::make_nvp("centre_of_referance", cfs_);
        ar & boost::serialization::make_nvp("search_roi",          top_layer_search_roi_);
        ar & boost::serialization::make_nvp("tmpl_datas",          pyramid_tmpl_datas_);
        ar & boost::serialization::make_nvp("match_mode",          match_mode_);
        ar & boost::serialization::make_nvp("tmpl_rgn",            tmpl_rgn_);
    }
};

}
}

#endif //__OPENCV_MVLAB_PIXEL_TEMPLATE_IMPL_HPP__
