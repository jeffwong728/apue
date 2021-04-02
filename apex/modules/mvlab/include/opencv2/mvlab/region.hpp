#ifndef __OPENCV_MVLAB_REGION_HPP__
#define __OPENCV_MVLAB_REGION_HPP__

#include "forward.hpp"
#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W Region {

public:
    Region() {}
    virtual ~Region() {}

    CV_WRAP static cv::Ptr<Region> GenEmpty();
    CV_WRAP static cv::Ptr<Region> GenChecker(const cv::Size &sizeRegion, const cv::Size &sizePattern);
    CV_WRAP static cv::Ptr<Region> GenTriangle(const cv::Point2f &v1, const cv::Point2f &v2, const cv::Point2f &v3);
    CV_WRAP static cv::Ptr<Region> GenQuadrangle(const cv::Point2f &v1, const cv::Point2f &v2, const cv::Point2f &v3, const cv::Point2f &v4);
    CV_WRAP static cv::Ptr<Region> GenRectangle(const cv::Rect2f &rect);
    CV_WRAP static cv::Ptr<Region> GenRotatedRectangle(const cv::RotatedRect &rotatedRect);
    CV_WRAP static cv::Ptr<Region> GenCircle(const cv::Point2f &center, const float radius);
    CV_WRAP static cv::Ptr<Region> GenCircleSector(const cv::Point2f &center, const float radius, const float startAngle, const float endAngle);
    CV_WRAP static cv::Ptr<Region> GenEllipse(const cv::Point2f &center, const cv::Size2f &size);
    CV_WRAP static cv::Ptr<Region> GenRotatedEllipse(const cv::Point2f &center, const cv::Size2f &size, const float phi);
    CV_WRAP static cv::Ptr<Region> GenEllipseSector(const cv::Point2f &center, const cv::Size2f &size, const float phi, const float startAngle, const float endAngle);
    CV_WRAP static cv::Ptr<Region> GenPolygon(const std::vector<cv::Point2f> &vertexes);
    /** @brief Generates the structuring element of choice.

     Generates a structuring element
     @param choice the type of structuring element to be generated.
        "square"  | square-shaped structuring element
        "circle"  | circle-shaped structuring element
        "diamond" | diamond-shaped structuring element
        "hline"   | horizontal line-shaped structuring element
        "vline"   | vertical line-shaped structuring element
     @param sz the size of the structuring element to be generated (square-shaped SE: width equals
        @f$(2 \cdot size + 1)@f$, circle-shaped SE: diameter equals
        @f$(2 \cdot size + 1)@f$, diamond-shaped SE: width equals
        @f$(2 \cdot size + 1)@f$).
     @return the generated structuring element as region.
     */
    CV_WRAP static cv::Ptr<Region> GenStructuringElement(const cv::String &choice, const int sz);
    /** @brief Load region from a file.

    The function try to load a region from a file given by fileName using specified format hint.
    @param  fileName File full path name load from.
    @param  opts Options used to load this region.
            Option Name   | Option Type   | Description
            ------------- | ------------- | -------------------------------
            FormatHint    | cv::String    | Load file format hint: "xml", "text", "binary"
    */
    CV_WRAP static cv::Ptr<Region> Load(const cv::String &fileName, const cv::Ptr<Dict> &opts = nullptr);

public:
    virtual int Draw(Mat &img, const Scalar& fillColor) const = 0;

public:
    CV_WRAP virtual bool Empty() const = 0;
    CV_WRAP virtual int Count() const = 0;
    CV_WRAP virtual int CountRuns() const = 0;
    CV_WRAP virtual int CountRows() const = 0;
    CV_WRAP virtual int CountConnect() const = 0;
    CV_WRAP virtual int CountHoles() const = 0;
    CV_WRAP virtual double Area() const = 0;
    CV_WRAP virtual cv::Point2d Centroid() const = 0;
    CV_WRAP virtual cv::Rect BoundingBox() const = 0;
    CV_WRAP virtual cv::Point3d SmallestCircle() const = 0;
    CV_WRAP virtual double AreaHoles() const = 0;
    CV_WRAP virtual double Contlength() const = 0;
    CV_WRAP virtual double Circularity() const = 0;
    CV_WRAP virtual double Compactness() const = 0;
    CV_WRAP virtual double Convexity() const = 0;
    CV_WRAP virtual cv::Scalar Diameter() const = 0;
    CV_WRAP virtual cv::Point3d EllipticAxis() const = 0;
    CV_WRAP virtual double Anisometry() const = 0;
    CV_WRAP virtual double Bulkiness() const = 0;
    CV_WRAP virtual double StructureFactor() const = 0;
    CV_WRAP virtual double Orientation() const = 0;
    CV_WRAP virtual cv::Ptr<Region> SelectObj(const int index) const = 0;
    CV_WRAP virtual cv::Ptr<Region> SelectArea(const double minArea, const double maxArea) const = 0;
    CV_WRAP virtual cv::Ptr<Contour> GetContour() const = 0;
    CV_WRAP virtual cv::Ptr<Contour> GetHole() const = 0;
    CV_WRAP virtual cv::Ptr<Contour> GetConvex() const = 0;
    CV_WRAP virtual cv::Ptr<Contour> GetPolygon(const float tolerance) const = 0;
    CV_WRAP virtual void GetPoints(CV_OUT std::vector<cv::Point> &points) const = 0;
    CV_WRAP virtual void GetRuns(CV_OUT std::vector<cv::Point3i> &runs) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Complement(const cv::Rect &universe) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Difference(const cv::Ptr<Region> &subRgn) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Intersection(const cv::Ptr<Region> &otherRgn) const = 0;
    CV_WRAP virtual cv::Ptr<Region> SymmDifference(const cv::Ptr<Region> &otherRgn) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Union1() const = 0;
    CV_WRAP virtual cv::Ptr<Region> Union2(const cv::Ptr<Region> &otherRgn) const = 0;
    CV_WRAP virtual bool TestEqual(const cv::Ptr<Region> &otherRgn) const = 0;
    CV_WRAP virtual bool TestPoint(const cv::Point &point) const = 0;
    CV_WRAP virtual bool TestSubset(const cv::Ptr<Region> &otherRgn) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Move(const cv::Point &delta) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Zoom(const cv::Size2f &scale) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Shrink(const cv::Size2f &ratio) const = 0;
    CV_WRAP virtual cv::Ptr<Region> AffineTrans(const cv::Matx33d &homoMat2D) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Connect() const = 0;
    CV_WRAP virtual int Draw(cv::InputOutputArray img, const cv::Scalar& fillColor) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Dilation(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts = nullptr) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Erosion(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts = nullptr) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Opening(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts = nullptr) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Closing(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts = nullptr) const = 0;

    /** @brief Get region error status message.
    */
    CV_WRAP virtual cv::String GetErrorStatus() const = 0;

    /** @brief Save region to a file.

    The function save a region to a file given by fileName using specified format.
    @param  fileName File full path name.
    @param  opts Options used to save this region.
            Option Name   | Option Type   | Description
            ------------- | ------------- | -------------------------------
            FileFormat    | cv::String    | Save file format: "xml", "text", "binary".
            Policy        | cv::String    | How to respond when file exists: "overwrite", "backup", "raise_error".
    */
    CV_WRAP virtual int Save(const cv::String &fileName, const cv::Ptr<Dict> &opts = nullptr) const = 0;
};

}
}

#endif
