#ifndef __OPENCV_MVLAB_CONTOUR_IMPL_HPP__
#define __OPENCV_MVLAB_CONTOUR_IMPL_HPP__

#include <opencv2/mvlab/contour.hpp>

namespace cv {
namespace mvlab {

class ContourImpl : public Contour, public std::enable_shared_from_this<ContourImpl>
{
    friend class boost::serialization::access;
public:
    ContourImpl() : is_self_intersect_(K_UNKNOWN), is_convex_(K_UNKNOWN) {}
    ContourImpl(const std::vector<Point2f> &vertexes, const int is_self_intersect);
    ContourImpl(ScalablePoint2fSequence *vertexes, const int is_self_intersect);
    ContourImpl(ScalablePoint2fSequenceSequence *curves, const int is_self_intersect);
    ContourImpl(const std::string &bytes);

public:
    int Draw(Mat &img, const Scalar& color, const float thickness, const int style) const CV_OVERRIDE;
    int Draw(InputOutputArray img, const Scalar& color, float thickness, const int style) const CV_OVERRIDE;

public:
    bool Empty() const CV_OVERRIDE;
    int Count() const CV_OVERRIDE;
    void GetCountPoints(std::vector<int> &cPoints) const CV_OVERRIDE;
    void GetArea(std::vector<double> &areas) const CV_OVERRIDE;
    void GetLength(std::vector<double> &lengthes) const CV_OVERRIDE;
    void GetCentroid(std::vector<cv::Point2f> &centroids) const CV_OVERRIDE;
    void GetBoundingBox(std::vector<cv::Rect> &boundingBoxes) const CV_OVERRIDE;
    void GetSmallestCircle(std::vector<cv::Point3d> &miniCircles) const CV_OVERRIDE;
    void GetSmallestRectangle(std::vector< cv::RotatedRect> &miniRects) const CV_OVERRIDE;
    void GetCircularity(std::vector<double> &circularities) const CV_OVERRIDE;
    cv::Ptr<Contour> Simplify(const float tolerance) const CV_OVERRIDE;
    cv::Ptr<Contour> GetConvex() const CV_OVERRIDE;
    //Access
    void GetPoints(std::vector<Point2f> &vertexes) const CV_OVERRIDE;
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
    void Feed(Cairo::RefPtr<Cairo::Context> &cr) const;
    const ScalablePoint2fSequenceSequence &Curves() const { return curves_; };
    static const cv::String &TypeGUID() { return type_guid_s; }

private:
    void DrawVerified(Mat &img, const Scalar& color, const float thickness, const int style) const;
    void AreaCenter() const;
    void PointsCloudMoments() const;
    cv::Ptr<Contour> GetConvexImpl() const;
    static bool IsCurveClosed(const ScalablePoint2fSequence &crv);

private:
    const ScalablePoint2fSequenceSequence curves_;
    mutable cv::Ptr<Contour> convex_hull_;
    mutable boost::optional<double> length_;
    mutable boost::optional<double> area_;
    mutable boost::optional<double> circularity_;
    mutable boost::optional<double> convexity_;
    mutable boost::optional<cv::Point2d> centroid_;
    mutable boost::optional<cv::Point2d> point_cloud_center_;
    mutable boost::optional<cv::Point3d> mini_ball_;
    mutable boost::optional<cv::Point3d> moment_2rd_;
    mutable boost::optional<cv::Point3d> point_cloud_moment_2rd_;
    mutable boost::optional<cv::Rect> bbox_;
    mutable boost::optional<cv::Scalar> diameter_;
    mutable int is_self_intersect_;
    mutable int is_convex_;
    mutable cv::String err_msg_;
    static const cv::String type_guid_s;

    template<class Archive>
    void save(Archive & ar, const unsigned int /*version*/) const
    {
        boost::serialization::void_cast_register<ContourImpl, Contour>();
        ar & boost::serialization::make_nvp("curves",            curves_);
        ar & boost::serialization::make_nvp("is_self_intersect", is_self_intersect_);
        ar & boost::serialization::make_nvp("is_convex",         is_convex_);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int /*version*/)
    {
        ScalablePoint2fSequenceSequence curves;
        boost::serialization::void_cast_register<ContourImpl, Contour>();
        ar & boost::serialization::make_nvp("curves", curves);
        ar & boost::serialization::make_nvp("is_self_intersect", is_self_intersect_);
        ar & boost::serialization::make_nvp("is_convex", is_convex_);
        const_cast<ScalablePoint2fSequenceSequence&>(curves_).swap(curves);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

}
}

BOOST_CLASS_EXPORT_KEY(cv::mvlab::Contour)
BOOST_CLASS_EXPORT_KEY2(cv::mvlab::ContourImpl, "55F24E48-3E9E-4164-80E0-DAB56F7735B2")

#endif
