#ifndef __OPENCV_MVLAB_CONTOUR_ARRAY_IMPL_HPP__
#define __OPENCV_MVLAB_CONTOUR_ARRAY_IMPL_HPP__

#include "contour_impl.hpp"

namespace cv {
namespace mvlab {

class ContourArrayImpl : public Contour
{
    friend class boost::serialization::access;
public:
    ContourArrayImpl() {}
    ContourArrayImpl(std::vector<cv::Ptr<Contour>> *contours) : contours_(std::move(*contours)) {}
    ContourArrayImpl(const std::string &bytes);

public:
    int Draw(Mat &img, const Scalar& color, const float thickness, const int style) const CV_OVERRIDE;
    int Draw(InputOutputArray img, const Scalar& color, float thickness, const int style) const CV_OVERRIDE;

public:
    bool Empty() const CV_OVERRIDE;
    int Count() const CV_OVERRIDE;
    int CountCurves() const CV_OVERRIDE;
    void GetCountPoints(std::vector<int> &cPoints) const CV_OVERRIDE;
    void GetArea(std::vector<double> &areas) const CV_OVERRIDE;
    void GetLength(std::vector<double> &lengthes) const CV_OVERRIDE;
    void GetCentroid(std::vector<cv::Point2f> &centroids) const CV_OVERRIDE;
    void GetBoundingBox(std::vector<cv::Rect> &boundingBoxes) const CV_OVERRIDE;
    void GetSmallestCircle(std::vector< cv::Point3d> &miniCircles) const CV_OVERRIDE;
    void GetSmallestRectangle(std::vector< cv::RotatedRect> &miniRects) const CV_OVERRIDE;
    void GetCircularity(std::vector<double> &circularities) const CV_OVERRIDE;
    cv::Ptr<Contour> Simplify(const float tolerance) const CV_OVERRIDE;
    cv::Ptr<Contour> GetConvex() const CV_OVERRIDE;
    //Access
    void GetPoints(std::vector<Point2f> &vertexes) const CV_OVERRIDE;
    void SelectPoints(const int index, CV_OUT std::vector<cv::Point2f> &vertexes) const CV_OVERRIDE;
    cv::Ptr<Contour> SelectObj(const int index) const CV_OVERRIDE;
    //Geometric Transformations
    cv::Ptr<Contour> Move(const cv::Point2f &delta) const CV_OVERRIDE;
    cv::Ptr<Contour> Zoom(const cv::Size2f &scale) const CV_OVERRIDE;
    cv::Ptr<Contour> AffineTrans(const cv::Matx33d &homoMat2D) const CV_OVERRIDE;
    //Features
    void GetTestClosed(std::vector<int> &isClosed) const CV_OVERRIDE;
    void GetTestConvex(std::vector<int> &isConvex) const CV_OVERRIDE;
    void GetTestPoint(const cv::Point2f &point, std::vector<int> &isInside) const CV_OVERRIDE;
    void GetTestSelfIntersection(const cv::String &closeContour, std::vector<int> &doesIntersect) const CV_OVERRIDE;

public:
    int CountPoints() const CV_OVERRIDE;
    double Area() const CV_OVERRIDE;
    double Length() const CV_OVERRIDE;
    cv::Point2d Centroid() const CV_OVERRIDE;
    cv::Point2d PointsCenter() const CV_OVERRIDE;
    cv::Point3d Moments() const CV_OVERRIDE;
    cv::Point3d PointsMoments() const CV_OVERRIDE;
    cv::Rect BoundingBox() const CV_OVERRIDE;
    cv::Point3d SmallestCircle() const CV_OVERRIDE;
    double Circularity() const CV_OVERRIDE;
    double Convexity() const CV_OVERRIDE;
    cv::Scalar Diameter() const CV_OVERRIDE;
    cv::RotatedRect SmallestRectangle() const CV_OVERRIDE;
    cv::Point3d EllipticAxis() const CV_OVERRIDE;
    double Anisometry() const CV_OVERRIDE;
    double Bulkiness() const CV_OVERRIDE;
    double StructureFactor() const CV_OVERRIDE;
    bool TestClosed() const CV_OVERRIDE;
    bool TestConvex() const CV_OVERRIDE;
    bool TestPoint(const cv::Point2f &point) const CV_OVERRIDE;
    bool TestSelfIntersection(const cv::String &closeContour) const CV_OVERRIDE;
    FitLineResults FitLine(const FitLineParameters &parameters) const CV_OVERRIDE;
    cv::String GetErrorStatus() const CV_OVERRIDE;
    int Save(const cv::String &fileName, const cv::Ptr<Dict> &opts) const CV_OVERRIDE;

public:
    int Load(const cv::String &fileName, const cv::Ptr<Dict> &opts);
    int Serialize(const cv::String &name, H5Group *g) const;
    static const cv::String &TypeGUID() { return type_guid_s; }

private:
    std::vector<cv::Ptr<Contour>> contours_;
    mutable cv::String err_msg_;
    static const cv::String type_guid_s;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int /*version*/)
    {
        boost::serialization::void_cast_register<ContourArrayImpl, Contour>();
        ar & boost::serialization::make_nvp("contours", contours_);
    }
};

}
}

BOOST_CLASS_EXPORT_KEY2(cv::mvlab::ContourArrayImpl, "E1CCE12C-1CB9-4715-B347-32EAA5B0B47A")

#endif //__OPENCV_MVLAB_CONTOUR_ARRAY_IMPL_HPP__
