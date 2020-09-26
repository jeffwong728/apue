#ifndef __OPENCV_MVLAB_REGION_IMPL_HPP__
#define __OPENCV_MVLAB_REGION_IMPL_HPP__

#include <contour/contour_impl.hpp>
#include <contour/contour_array_impl.hpp>
#include <opencv2/mvlab/region.hpp>

namespace cv {
namespace mvlab {
struct PolyEdge
{
    int y0, y1;
    int x, dx;
    PolyEdge *next;
};

struct CmpEdges
{
    bool operator()(const PolyEdge& e1, const PolyEdge& e2)
    {
        return e1.y0 - e2.y0 ? e1.y0 < e2.y0 : e1.x - e2.x ? e1.x < e2.x : e1.dx < e2.dx;
    }
};

struct RunLength
{
    friend class boost::serialization::access;
private:
    RunLength() : RunLength(0, 0, 0) {}

public:
    RunLength(const RunLength &r) : row(r.row), colb(r.colb), cole(r.cole), label(r.label) {}
    RunLength(const int ll, const int bb, const int ee) : row(ll), colb(bb), cole(ee), label(0) {}
    RunLength(const int ll, const int bb, const int ee, const int lab) : row(ll), colb(bb), cole(ee), label(lab) {}
    RunLength(const cv::Point &start, const int len) : row(start.y), colb(start.x), cole(start.x+len), label(0) {}
    int len() const { return cole - colb; }
    cv::Point start() const { return { colb, row }; }
    cv::Point end() const { return { cole-1, row }; }

    int row;  // line number (row) of run
    int colb; // column index of beginning(include) of run
    int cole; // column index of ending(exclude) of run
    int label;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int /*version*/)
    {
        ar & BOOST_SERIALIZATION_NVP(row);
        ar & BOOST_SERIALIZATION_NVP(colb);
        ar & BOOST_SERIALIZATION_NVP(cole);
        ar & BOOST_SERIALIZATION_NVP(label);
    }

    bool operator<(const RunLength &right) const
    {
        if (row < right.row ||
            (row == right.row && colb < right.colb) ||
            (row == right.row && colb == right.colb && cole < right.cole))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

using RunSequence               = ao::uvector<RunLength, MyAlloc<RunLength>>;
using RunSequenceSequence       = std::vector<RunSequence, MyAlloc<RunSequence>>;
using RunPtrSequence            = std::vector<RunLength *, MyAlloc<RunLength*>>;
using RunConstPtrSequence       = std::vector<const RunLength *, MyAlloc<const RunLength*>>;
using RowBeginSequence          = ScalableIntSequence;
using RowBeginSequenceSequence  = ScalableIntSequenceSequence;
using UScalablePolyEdgeSequence = ao::uvector<PolyEdge, MyAlloc<PolyEdge>>;

class RegionImpl : public Region, public std::enable_shared_from_this<RegionImpl>
{
    friend class boost::serialization::access;
public:
    RegionImpl() { }
    RegionImpl(RunSequence *const runs);
    RegionImpl(const cv::Mat &runs);
    RegionImpl(const std::string &bytes);

public:
    int Draw(Mat &img, const Scalar& fillColor) const CV_OVERRIDE;
    int Draw(InputOutputArray img, const Scalar& fillColor) const CV_OVERRIDE;

public:
    bool Empty() const CV_OVERRIDE;
    double Area() const CV_OVERRIDE;
    Point2d Centroid() const CV_OVERRIDE;
    Rect BoundingBox() const CV_OVERRIDE;
    cv::Point3d SmallestCircle() const CV_OVERRIDE;
    double AreaHoles() const CV_OVERRIDE;
    double Contlength() const CV_OVERRIDE;
    int Count() const CV_OVERRIDE;
    int CountRuns() const CV_OVERRIDE;
    int CountRows() const CV_OVERRIDE;
    int CountConnect() const CV_OVERRIDE;
    int CountHoles() const CV_OVERRIDE;
    // Access
    cv::Ptr<Region> SelectObj(const int index) const CV_OVERRIDE;
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
    cv::Ptr<Region> Dilation(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts = nullptr) const CV_OVERRIDE;
    cv::Ptr<Region> Erosion(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts = nullptr) const CV_OVERRIDE;
    cv::Ptr<Region> Opening(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts = nullptr) const CV_OVERRIDE;
    cv::Ptr<Region> Closing(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts = nullptr) const CV_OVERRIDE;
    // Miscellaneous
    cv::Ptr<Region> Clone() const;
    cv::String GetErrorStatus() const CV_OVERRIDE;
    int Save(const cv::String &fileName, const cv::Ptr<Dict> &opts) const CV_OVERRIDE;

public:
    const RunSequence &GetAllRuns() const { return rgn_runs_; }
    const RowBeginSequence &GetRowBeginSequence() const;
    static const cv::String &TypeGUID() { return type_guid_s; }
    int Load(const cv::String &fileName, const cv::Ptr<Dict> &opts);
    int Serialize(const cv::String &name, H5Group *g) const;

private:
    void FromMask(const cv::Mat &mask);
    void FromPathVector(const Geom::PathVector &pv);
    void DrawVerifiedRGBA(Mat &img, const Scalar& fillColor) const;
    void DrawVerifiedRGB(Mat &img, const Scalar& fillColor) const;
    void DrawVerifiedGray(Mat &img, const Scalar& fillColor) const;
    void TraceContour() const;
    void GatherBasicFeatures() const;
    void GetContourInfo(const cv::Ptr<Contour> &contour, int &numEdges, int &maxNumPoints) const;
    void ZoomContourToEdges(const cv::Ptr<Contour> &contour, const cv::Size2f &scale, UScalablePointSequence &v, UScalablePolyEdgeSequence::pointer &pEdge) const;
    void AffineContourToEdges(const cv::Ptr<Contour> &contour, const cv::Matx33d &m, UScalablePointSequence &v, UScalablePolyEdgeSequence::pointer &pEdge) const;

private:
    struct ErosTransPoint;
    struct RetGenerateSkeletonB;
    struct RetGenerateErosionTransformX;
    struct RetGenerateErosionTransformX2;
    static RetGenerateSkeletonB const generateSkeletonBtrans(const RegionImpl *SE, const cv::Point &transVector);
    static RetGenerateSkeletonB const generateSkeletonB(const RegionImpl *SE, const cv::Point &transVector);
    RetGenerateErosionTransformX const generateErosionTransformX(const RegionImpl *SE, const int lmin) const;
    RetGenerateErosionTransformX const generateErosionTransformX2(const RegionImpl *SE, const int lmin) const;
    RetGenerateErosionTransformX const generateErosionTransformX2cut(const RegionImpl *SE, const int lmin, const int lmax) const;
    RetGenerateErosionTransformX const generateErosionTransformXcomp(const RegionImpl *SE, const int lmin) const;
    RetGenerateErosionTransformX const generateErosionTransformXcompcut(const RegionImpl *SE, const int lmin, const int lmax) const;
    RetGenerateErosionTransformX const generateExtErosionTransformX(const RegionImpl *SE, const int lmin) const;
    cv::Ptr<Region> erode1(const cv::Ptr<Region> &structElement) const;
    cv::Ptr<Region> erode2(const cv::Ptr<Region> &structElement) const;
    cv::Ptr<Region> erode2cut(const cv::Ptr<Region> &structElement) const;
    cv::Ptr<Region> erode3(const cv::Ptr<Region> &structElement) const;
    cv::Ptr<Region> dilate(const cv::Ptr<Region> &structElement) const;
    cv::Ptr<Region> dilatecut(const cv::Ptr<Region> &structElement) const;
    cv::Ptr<Region> rleErosion(const cv::Ptr<Region> &structElement) const;
    cv::Ptr<Region> rleDilation(const cv::Ptr<Region> &structElement) const;
    cv::Ptr<Region> rleCompDilation(const cv::Ptr<Region> &structElement) const;

private:
    const RunSequence                    rgn_runs_;
    mutable RowBeginSequence             row_begs_;
    mutable boost::optional<double>      area_;
    mutable boost::optional<double>      hole_area_;
    mutable boost::optional<double>      cont_length_;
    mutable boost::optional<cv::Point2d> centroid_;
    mutable boost::optional<cv::Rect>    bbox_;
    mutable boost::optional<cv::Point3d> min_circle_;
    mutable cv::Ptr<Contour> contour_;
    mutable cv::Ptr<Contour> hole_;
    mutable cv::String err_msg_;
    static const cv::String type_guid_s;

    template<class Archive>
    void save(Archive & ar, const unsigned int /*version*/) const
    {
        std::vector<RunLength> runs(rgn_runs_.cbegin(), rgn_runs_.cend());
        boost::serialization::base_object<Region>(*this);
        ar & boost::serialization::make_nvp("runs", runs);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int /*version*/)
    {
        std::vector<RunLength> runs;
        // method 1 : invoke empty base class serialization
        // method 2 : explicitly register base/derived relationship
        // boost::serialization::void_cast_register<base, derived>();
        boost::serialization::base_object<Region>(*this);
        ar & boost::serialization::make_nvp("runs", runs);
        const_cast<RunSequence&>(rgn_runs_).assign(runs.cbegin(), runs.cend());
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

extern void CollectPolyEdges_(const cv::Point* v, const int count, UScalablePolyEdgeSequence::pointer &pEdge);
extern RunSequence FillEdgeCollection_(UScalablePolyEdgeSequence &edges);

}
}

BOOST_CLASS_EXPORT_KEY(cv::mvlab::Region)
BOOST_CLASS_EXPORT_KEY2(cv::mvlab::RegionImpl, "FFF61A27-41E0-4ED8-9C08-8A6B124E9008")

namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, cv::mvlab::Region &v, const unsigned int version)
{
}

}
}

#endif
