#ifndef __OPENCV_MVLAB_PIXEL_TEMPLATE_HPP__
#define __OPENCV_MVLAB_PIXEL_TEMPLATE_HPP__

#include "forward.hpp"
#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W PixelTemplate
{
public:
    PixelTemplate() {}
    virtual ~PixelTemplate() {}

    /** @brief Create a empty pixel template.

    The function generate a empty pixel template. You need to call Create later create a valid template.
    */
    CV_WRAP static cv::Ptr<PixelTemplate> GenEmpty();

    /** @brief Generate a pixel template.

    The function PixelTemplate::GenTemplate create a pixel template by the input parameters and options.
    @param img Source image used to create this template
    @param rgn Template region used to create this template
    @param opts Options used to create this template
            Option Name   | Option Type   | Description
            ------------- | ------------- | -------------------------------
            AngleStart    | int           | Template start angle in degree.
            AngleExtent   | int           | Template angle extent in degree.
            PyramidLevel  | int           | Template pyramid level.
    */
    CV_WRAP static cv::Ptr<PixelTemplate> GenTemplate(cv::InputArray img, const cv::Ptr<Region> &rgn, const cv::Ptr<Dict> &opts);

    /** @brief Load template from a file.

    The function try to load a pixel template from a file given by fileName using specified format hint.
    @param  fileName File full path name load from.
    @param  opts Options used to load this template.
            Option Name   | Option Type   | Description
            ------------- | ------------- | -------------------------------
            FormatHint    | cv::String    | Load file format hint: "xml", "text", "binary"
    */
    CV_WRAP static cv::Ptr<PixelTemplate> Load(const cv::String &fileName, const cv::Ptr<Dict> &opts = nullptr);

public:
    /** @brief Test template emptiness.
    */
    CV_WRAP virtual bool Empty() const = 0;

    /** @brief Get template error status message.
    */
    CV_WRAP virtual cv::String GetErrorStatus() const = 0;
    CV_WRAP virtual int GetPyramidLevel() const = 0;

    /** @brief Create a pixel template.

    The function PixelTemplate::GenTemplate create a pixel template by the input parameters and options.
    @param  img  Source image used to create this template
    @param  rgn  Template region used to create this template
    @param  opts Options used to create this template
            Option Name   | Option Type   | Description
            ------------- | ------------- | -------------------------------
            AngleStart    | int           | Template start angle in degree.
            AngleExtent   | int           | Template angle extent in degree.
            PyramidLevel  | int           | Template pyramid level.
            MatchMode     | cv::String    | "sad", "ncc"
    */
    CV_WRAP virtual int Create(cv::InputArray img, const cv::Ptr<Region> &rgn, const cv::Ptr<Dict> &opts) = 0;

    /** @brief Match a pixel template.

    The function PixelTemplate::Search match the best matches of a pixel template in an image.
    @param  img  Input image in which the template should be found.
    @param  opts Options used to match this template
            Option Name   | Option Type   | Description
            ------------- | ------------- | -------------------------------
            ROI           | Region        | Determine the search space for the reference point of the template.
            AngleStart    | int           | Determine start angle of the range of rotations for which the template is searched.
            AngleExtent   | int           | Determine angle extent of the range of rotations for which the template is searched.
            MinScore      | float         | Determines what score a potential match must at least have to be regarded as an instance of the template in the image.
            TouchBorder   | int           | Determines if consider templates which touch search region.
            NumMatches    | int           | The maximum number of instances to be matched.
            MaxOverlap    | float         | Determines by what fraction two instances may at most overlap in order to consider them as different instances.
            SubPixel      | cv::String    | Determines whether the instances should be extracted with subpixel accuracy.
            NumLevels     | int           | The number of pyramid levels used during the search.
    */
    CV_WRAP virtual cv::Ptr<MatchResult> Search(cv::InputArray img, const cv::Ptr<Dict> &opts) const = 0;

    /** @brief Draw a pixel template.

    The function draw a pixel template on destination image.
    @param  img  Destination image draw into
    @param  opts Options used to draw this template
            Option Name   | Option Type   | Description
            ------------- | ------------- | -------------------------------
            Color         | cv::Scalar    | Color used to draw pixel template
            Style         | int           | Style used to draw pixel template
    */
    CV_WRAP virtual int Draw(cv::InputOutputArray img, const cv::Ptr<Dict> &opts = nullptr) const = 0;

    /** @brief Save template to a file.

    The function save a pixel template to a file given by fileName using specified format.
    @param  fileName File full path name.
    @param  opts Options used to save this template.
            Option Name   | Option Type   | Description
            ------------- | ------------- | -------------------------------
            FileFormat    | cv::String    | Save file format: "xml", "text", "binary".
            Policy        | cv::String    | How to respond when file exists: "overwrite", "backup", "raise_error".
    */
    CV_WRAP virtual int Save(const cv::String &fileName, const cv::Ptr<Dict> &opts = nullptr) const = 0;
};

}
}

#endif //__OPENCV_MVLAB_PIXEL_TEMPLATE_HPP__
