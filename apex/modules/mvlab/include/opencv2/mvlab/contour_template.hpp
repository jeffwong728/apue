#ifndef __OPENCV_MVLAB_CONTOUR_TEMPLATE_HPP__
#define __OPENCV_MVLAB_CONTOUR_TEMPLATE_HPP__

#include "forward.hpp"
#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W ContourTemplate
{
public:
    ContourTemplate() {}
    virtual ~ContourTemplate() {}

    /** @brief Create a empty contour template.

    The function generate a empty contour template. You need to call Create later create a valid template.
    */
    CV_WRAP static cv::Ptr<ContourTemplate> GenEmpty();

    /** @brief Generate a contour template.

    The function ContourTemplate::GenTemplate create a contour template by the input parameters and options.
    @param img Source image used to create this template
    @param rgn Template region used to create this template
    @param opts Options used to create this template
            Option Name   | Option Type   | Description
            ------------- | ------------- | -------------------------------
            AngleStart    | int           | Template start angle in degree.
            AngleExtent   | int           | Template angle extent in degree.
            PyramidLevel  | int           | Template pyramid level.
            LowContrast   | int           | Template lower contrast.
            HighContrast  | int           | Template upper contrast.
    */
    CV_WRAP static cv::Ptr<ContourTemplate> GenTemplate(cv::InputArray img, const cv::Ptr<Region> &rgn, const cv::Ptr<Dict> &opts);

    /** @brief Load template from a file.

    The function try to load a contour template from a file given by.fileName using specified format hint.
    @param  fileName File full path name load from.
    @param  opts Options used to load this template.
            Option Name   | Option Type   | Description
            ------------- | ------------- | -------------------------------
            FormatHint    | cv::String    | Load file format hint: "xml", "text", "binary"
    */
    CV_WRAP static cv::Ptr<ContourTemplate> Load(const cv::String &fileName, const cv::Ptr<Dict> &opts = nullptr);

public:
    /** @brief Test template emptiness.
    */
    CV_WRAP virtual bool Empty() const = 0;

    /** @brief Get template error status message.
    */
    CV_WRAP virtual cv::String GetErrorStatus() const = 0;
    CV_WRAP virtual int GetPyramidLevel() const = 0;

    /** @brief Create a contour template.

    The function ContourTemplate::GenTemplate create a contour template by the input parameters and options.
    @param  img  Source image used to create this template
    @param  rgn  Template region used to create this template
    @param  opts Options used to create this template
            Option Name   | Option Type   | Description
            ------------- | ------------- | -------------------------------
            AngleStart    | int           | Template start angle in degree.
            AngleExtent   | int           | Template angle extent in degree.
            PyramidLevel  | int           | Template pyramid level.
            LowContrast   | int           | Template lower contrast.
            HighContrast  | int           | Template upper contrast.
    */
    CV_WRAP virtual int Create(cv::InputArray img, const cv::Ptr<Region> &rgn, const cv::Ptr<Dict> &opts) = 0;

    /** @brief Match a contour template.

    The function ContourTemplate::Search match the best matches of a contour template in an image.
    @param  img  Input image in which the template should be found.
    @param  opts Options used to match this template
            Option Name   | Option Type   | Description
            ------------- | ------------- | -------------------------------
            SearchRegion  | Region        | Determine the search space for the reference point of the template.
            AngleStart    | int           | Determine start angle of the range of rotations for which the template is searched.
            AngleExtent   | int           | Determine angle extent of the range of rotations for which the template is searched.
            MinScore      | float         | Determines what score a potential match must at least have to be regarded as an instance of the template in the image.
            MinContrast   | int           | Min contrast be considered a contour point.
            TouchBorder   | int           | Determines if consider templates which touch search region.
            Greediness    | float         | Determines how greedily the search should be carried out.
            NumMatches    | int           | The maximum number of instances to be matched.
            MaxOverlap    | float         | Determines by what fraction two instances may at most overlap in order to consider them as different instances.
            SubPixel      | cv::String    | Determines whether the instances should be extracted with subpixel accuracy.
            NumLevels     | int           | The number of pyramid levels used during the search.
    */
    CV_WRAP virtual cv::Ptr<MatchResult> Search(cv::InputArray img, const cv::Ptr<Dict> &opts) const = 0;

    /** @brief Draw a contour template.

    The function draw a contour template on destination image.
    @param  img  Destination image draw into
    @param  opts Options used to draw this template
            Option Name       | Option Type   | Description
            -------------     | ------------- | -------------------------------
            DrawTemplate      | int           | If draw template contour: 0 - don't draw, 1 - draw
            ColorTemplate     | cv::Scalar    | Color used to draw template contour
            StyleTemplate     | int           | Style used to draw template contour
            ThicknessTemplate | float         | Line thickness used to draw template contour
            DrawRegion        | int           | If draw template region: 0 - don't draw, 1 - draw
            ColorRegion       | cv::Scalar    | Color used to draw template region
            StyleRegion       | int           | Style used to draw template region
            ThicknessRegion   | float         | Line thickness used to draw template region
    */
    CV_WRAP virtual int Draw(cv::InputOutputArray img, const cv::Ptr<Dict> &opts = nullptr) const = 0;

    /** @brief Save template to a file.

    The function save a contour template to a file given by.FileFullName using specified format.
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

#endif //__OPENCV_MVLAB_CONTOUR_TEMPLATE_HPP__
