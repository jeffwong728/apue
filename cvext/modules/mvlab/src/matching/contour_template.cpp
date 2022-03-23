#include "precomp.hpp"
#include "contour_template_impl.hpp"
#include <utility.hpp>

namespace cv {
namespace mvlab {

cv::Ptr<ContourTemplate> ContourTemplate::GenEmpty()
{
    return makePtr<ContourTemplateImpl>();
}

cv::Ptr<ContourTemplate> ContourTemplate::GenTemplate(cv::InputArray img, const cv::Ptr<Region> &rgn, const cv::Ptr<Dict> &opts)
{
    cv::Ptr<ContourTemplate> tmpl = ContourTemplate::GenEmpty();
    tmpl->Create(img, rgn, opts);
    return tmpl;
}

cv::Ptr<ContourTemplate> ContourTemplate::Load(const cv::String &fileName, const cv::Ptr<Dict> &opts)
{
    cv::Ptr<ContourTemplateImpl> tmpl = makePtr<ContourTemplateImpl>();
    tmpl->Load(fileName, opts);
    return tmpl;
}

}
}
