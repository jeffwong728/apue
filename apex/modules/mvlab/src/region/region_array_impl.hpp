#ifndef __OPENCV_MVLAB_REGION_ARRAY_IMPL_HPP__
#define __OPENCV_MVLAB_REGION_ARRAY_IMPL_HPP__

#include <contour/contour_impl.hpp>
#include <contour/contour_array_impl.hpp>
#include <opencv2/mvlab/region.hpp>

namespace cv {
namespace mvlab {

class RegionArrayImpl : public Region
{
    friend class boost::serialization::access;
public:
    RegionArrayImpl() { }
    RegionArrayImpl(std::vector<cv::Ptr<Region>> *rgns) : rgns_(std::move(*rgns)) {}
    RegionArrayImpl(const cv::Mat &allRuns, const cv::Mat &delimits);
    RegionArrayImpl(const std::string &bytes);

public:
    int Draw(cv::Mat &img, const cv::Scalar& fillColor) const CV_OVERRIDE;
    int Draw(cv::InputOutputArray img, const cv::Scalar& fillColor) const CV_OVERRIDE;

public:
    bool Empty() const CV_OVERRIDE;
    double Area() const CV_OVERRIDE;
    cv::Point2d Centroid() const CV_OVERRIDE;
    cv::Rect BoundingBox() const CV_OVERRIDE;
    cv::Point3d SmallestCircle() const CV_OVERRIDE;
    double AreaHoles() const CV_OVERRIDE;
    double Contlength() const CV_OVERRIDE;
    double Circularity() const CV_OVERRIDE;
    double Compactness() const CV_OVERRIDE;
    double Convexity() const CV_OVERRIDE;
    cv::Scalar Diameter() const CV_OVERRIDE;
    cv::Point3d EllipticAxis() const CV_OVERRIDE;
    double Anisometry() const CV_OVERRIDE;
    double Bulkiness() const CV_OVERRIDE;
    double StructureFactor() const CV_OVERRIDE;
    double Orientation() const CV_OVERRIDE;
    int Count() const CV_OVERRIDE;
    int CountRuns() const CV_OVERRIDE;
    int CountRows() const CV_OVERRIDE;
    int CountConnect() const CV_OVERRIDE;
    int CountHoles() const CV_OVERRIDE;
    // Access
    cv::Ptr<Region> SelectObj(const int index) const CV_OVERRIDE;
    cv::Ptr<Region> SelectArea(const double minArea, const double maxArea) const CV_OVERRIDE;
    cv::Ptr<Contour> GetContour() const CV_OVERRIDE;
    cv::Ptr<Contour> GetHole() const CV_OVERRIDE;
    cv::Ptr<Contour> GetConvex() const CV_OVERRIDE;
    cv::Ptr<Contour> GetPolygon(const float tolerance) const CV_OVERRIDE;
    void GetPoints(std::vector<cv::Point> &points) const CV_OVERRIDE;
    void GetRuns(std::vector<cv::Point3i> &runs) const CV_OVERRIDE;
    // Sets
    cv::Ptr<Region> Complement(const cv::Rect &universe) const CV_OVERRIDE;
    cv::Ptr<Region> Difference(const cv::Ptr<Region> &subRgn) const CV_OVERRIDE;
    cv::Ptr<Region> Intersection(const cv::Ptr<Region> &otherRgn) const CV_OVERRIDE;
    cv::Ptr<Region> SymmDifference(const cv::Ptr<Region> &otherRgn) const CV_OVERRIDE;
    cv::Ptr<Region> Union1() const CV_OVERRIDE;
    cv::Ptr<Region> Union2(const cv::Ptr<Region> &otherRgn) const CV_OVERRIDE;
    // Tests
    bool TestEqual(const cv::Ptr<Region> &otherRgn) const CV_OVERRIDE;
    bool TestPoint(const cv::Point &point) const CV_OVERRIDE;
    bool TestSubset(const cv::Ptr<Region> &otherRgn) const CV_OVERRIDE;
    // Geometric Transformations
    cv::Ptr<Region> Move(const cv::Point &delta) const CV_OVERRIDE;
    cv::Ptr<Region> Zoom(const cv::Size2f &scale) const CV_OVERRIDE;
    cv::Ptr<Region> Shrink(const cv::Size2f &ratio) const CV_OVERRIDE;
    cv::Ptr<Region> AffineTrans(const cv::Matx33d &homoMat2D) const CV_OVERRIDE;
    cv::Ptr<Region> Connect() const CV_OVERRIDE;
    // Morphology
    cv::Ptr<Region> Dilation(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts) const CV_OVERRIDE;
    cv::Ptr<Region> Erosion(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts) const CV_OVERRIDE;
    cv::Ptr<Region> Opening(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts) const CV_OVERRIDE;
    cv::Ptr<Region> Closing(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts) const CV_OVERRIDE;
    cv::String GetErrorStatus() const CV_OVERRIDE;
    int Save(const cv::String &fileName, const cv::Ptr<Dict> &opts) const CV_OVERRIDE;

public:
    int Load(const cv::String &fileName, const cv::Ptr<Dict> &opts);
    int Serialize(const cv::String &name, H5Group *g) const;
    static const cv::String &TypeGUID() { return type_guid_s; }

private:
    bool IsNeedDoParallel() const;

private:
    std::vector<cv::Ptr<Region>> rgns_;
    mutable cv::String err_msg_;
    static const cv::String type_guid_s;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int /*version*/)
    {
        boost::serialization::base_object<Region>(*this);
        ar & boost::serialization::make_nvp("rgns", rgns_);
    }
};

}
}

BOOST_CLASS_EXPORT_KEY2(cv::mvlab::RegionArrayImpl, "A4278614-568E-4854-B0B2-A1875EDB1189")

#endif //__OPENCV_MVLAB_REGION_ARRAY_IMPL_HPP__
