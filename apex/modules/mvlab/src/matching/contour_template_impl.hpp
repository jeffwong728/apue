#ifndef __OPENCV_MVLAB_CONTOUR_TEMPLATE_IMPL_HPP__
#define __OPENCV_MVLAB_CONTOUR_TEMPLATE_IMPL_HPP__

#include <opencv2/mvlab/contour_template.hpp>
#include <matching/shapetmpl.h>

namespace cv {
namespace mvlab {

class ContourTemplateImpl : public ContourTemplate, public ShapeTemplate, public std::enable_shared_from_this<ContourTemplateImpl>
{
    friend class boost::serialization::access;
public:
    ContourTemplateImpl() {}
    ContourTemplateImpl(const std::string &bytes);

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
    void GetTemplateOptions(const cv::Ptr<Dict> &opts, ShapeTmplCreateData &stcd);

private:
    static const cv::String type_guid_s;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int /*version*/)
    {
        ar & boost::serialization::make_nvp("pyramid_level",       pyramid_level_);
        ar & boost::serialization::make_nvp("angle_start",         angle_start_);
        ar & boost::serialization::make_nvp("angle_extent",        angle_extent_);
        ar & boost::serialization::make_nvp("low_contrast",        low_contrast_);
        ar & boost::serialization::make_nvp("high_contrast",       high_contrast_);
        ar & boost::serialization::make_nvp("centre_of_referance", cfs_);
        ar & boost::serialization::make_nvp("search_roi",          top_layer_search_roi_);
        ar & boost::serialization::make_nvp("tmpl_rgn",            tmpl_rgn_);
        ar & boost::serialization::make_nvp("tmpl_datas",          pyramid_tmpl_datas_);
    }
};

}
}

#endif //__OPENCV_MVLAB_CONTOUR_TEMPLATE_IMPL_HPP__
