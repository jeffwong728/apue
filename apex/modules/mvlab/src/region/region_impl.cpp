#include "precomp.hpp"
#include "region_impl.hpp"
#include "region_array_impl.hpp"
#include "rtd_encoder.hpp"
#include "utility.hpp"
#include "connection.hpp"
#include "region_bool.hpp"
#include <convex_hull.hpp>
#include <hdf5/h5group_impl.hpp>
#include <opencv2/mvlab.hpp>

BOOST_CLASS_EXPORT_IMPLEMENT(cv::mvlab::Region)
BOOST_CLASS_EXPORT_IMPLEMENT(cv::mvlab::RegionImpl)

namespace cv {
namespace mvlab {

const cv::String RegionImpl::type_guid_s{"FFF61A27-41E0-4ED8-9C08-8A6B124E9008"};
extern RunSequence RunLengthEncode(const cv::Mat &imgMat, const int minGray, const int maxGray);

RegionImpl::RegionImpl(RunSequence *const runs)
    : rgn_runs_(std::move(*runs))
{
}

RegionImpl::RegionImpl(const cv::Mat &runs)
{
    if (runs.empty())
    {
        return;
    }

    if (CV_32S != runs.depth() || 4 != runs.channels() || 2 != runs.dims || 1 != runs.cols)
    {
        return;
    }

    RunSequence aRuns(runs.rows);
    for (int r=0; r<runs.rows; ++r)
    {
        const cv::Vec<int, 4> &elem = runs.at<cv::Vec<int, 4>>(cv::Vec<int, 2>(r, 0));
        RunLength &run = aRuns[r];
        run.row = elem[0];
        run.colb = elem[1];
        run.cole = elem[2];
        run.label = elem[3];
    }

    const_cast<RunSequence &>(rgn_runs_).swap(aRuns);
}

RegionImpl::RegionImpl(const std::string &bytes)
{
    try
    {
        std::istringstream iss(bytes, std::ios_base::in | std::ios_base::binary);
        boost::iostreams::filtering_istream fin;
        fin.push(boost::iostreams::lzma_decompressor());
        fin.push(iss);

        boost::archive::text_iarchive ia(fin);
        ia >> boost::serialization::make_nvp("region", *this);
    }
    catch (const std::exception &e)
    {
        err_msg_ = e.what();
    }
}

int RegionImpl::Draw(Mat &img,
    const Scalar& fillColor) const
{
    if (Empty())
    {
        return MLR_REGION_EMPTY;
    }

    if (img.empty())
    {
        const Rect bbox = RegionImpl::BoundingBox();
        if (bbox.width > 0 && bbox.height > 0)
        {
            img = Mat::zeros(bbox.br().y + 1, bbox.br().x + 1, CV_8UC4);
        }
        else
        {
            return MLR_REGION_EMPTY;
        }
    }

    if (img.empty())
    {
        return MLR_IMAGE_EMPTY;
    }

    int dph = img.depth();
    int cnl = img.channels();
    if (CV_8U == dph && 4 == cnl)
    {
        DrawVerifiedRGBA(img, fillColor);
    }
    else if (CV_8U == dph && 3 == cnl)
    {
        DrawVerifiedRGB(img, fillColor);
    }
    else if (CV_8U == dph && 1 == cnl)
    {
        DrawVerifiedGray(img, fillColor);
    }
    else
    {
        return MLR_IMAGE_FORMAT_ERROR;
    }

    return MLR_SUCCESS;
}

int RegionImpl::Draw(InputOutputArray img,
    const Scalar& fillColor) const
{
    if (Empty())
    {
        return MLR_REGION_EMPTY;
    }

    Mat imgMat = img.getMat();
    if (imgMat.empty())
    {
        int rest = RegionImpl::Draw(imgMat, fillColor);
        img.assign(imgMat);
        return rest;
    }
    else
    {
        int dph = img.depth();
        int cnl = img.channels();
        if (CV_8U == dph && (1 == cnl || 3 == cnl || 4 == cnl))
        {
            int rest = RegionImpl::Draw(imgMat, fillColor);
            img.assign(imgMat);
            return rest;
        }
        else
        {
            return MLR_IMAGE_FORMAT_ERROR;
        }
    }
}

bool RegionImpl::Empty() const
{
    return rgn_runs_.empty();
}

double RegionImpl::Area() const
{
    if (boost::none == area_)
    {
        GatherBasicFeatures();
    }

    return *area_;
}

cv::Point2d RegionImpl::Centroid() const
{
    if (boost::none == centroid_)
    {
        GatherBasicFeatures();
    }

    return *centroid_;
}

cv::Rect RegionImpl::BoundingBox() const
{
    if (boost::none == bbox_)
    {
        GatherBasicFeatures();
    }

    return *bbox_;
}

cv::Point3d RegionImpl::SmallestCircle() const
{
    if (min_circle_ == boost::none)
    {
        float radius = 0;
        cv::Point2f center;
        std::vector<cv::Point> points;
        points.reserve(rgn_runs_.size() * 2);

        for (const RunLength &r : rgn_runs_)
        {
            const auto n = r.cole - r.colb;
            if (1 == n)
            {
                points.emplace_back(r.colb, r.row);
            }
            else
            {
                points.emplace_back(r.colb, r.row);
                points.emplace_back(r.cole - 1, r.row);
            }
        }

        if (points.empty())
        {
            min_circle_.emplace(0.0, 0.0, 0.0);
        }
        else
        {
            cv::minEnclosingCircle(points, center, radius);
            min_circle_.emplace(center.x, center.y, radius);
        }
    }

    return *min_circle_;
}

double RegionImpl::AreaHoles() const
{
    if (boost::none == hole_area_)
    {
        hole_area_ = RegionImpl::GetHole()->Area();
    }

    return *hole_area_;
}

double RegionImpl::Contlength() const
{
    if (boost::none == cont_length_)
    {
        cont_length_ = RegionImpl::GetContour()->Length();
    }

    return *cont_length_;
}

double RegionImpl::Circularity() const
{
    if (boost::none == circularity_)
    {
        const double F = RegionImpl::Area();
        const double maxSqrRadius = MaxSqrRadius();
        if (maxSqrRadius > G_D_TOL)
        {
            const double CS = F / (maxSqrRadius*M_PI);
            circularity_ = std::min(1., CS);
        }
        else
        {
            circularity_ = 0.;
        }
    }

    return *circularity_;
}

double RegionImpl::Compactness() const
{
    const double L = RegionImpl::Contlength();
    const double F = RegionImpl::Area();
    if (F > G_D_TOL)
    {
        const double CS = (L*L) / (4 * F*M_PI);
        return std::max(1., CS);
    }
    else
    {
        return std::numeric_limits<double>::max();
    }
}

double RegionImpl::Convexity() const
{
    const double FC = RegionImpl::GetConvex()->Area();
    const double FO = RegionImpl::Area();
    if (FC > G_D_TOL)
    {
        return FO/FC;
    }
    else
    {
        return 0.;
    }
}

cv::Scalar RegionImpl::Diameter() const
{
    return RegionImpl::GetConvex()->Diameter();
}

cv::Point3d RegionImpl::EllipticAxis() const
{
    return RegionImpl::GetContour()->EllipticAxis();
}

double RegionImpl::Anisometry() const
{
    return RegionImpl::GetContour()->Anisometry();
}

double RegionImpl::Bulkiness() const
{
    return RegionImpl::GetContour()->Bulkiness();
}

double RegionImpl::StructureFactor() const
{
    return RegionImpl::GetContour()->StructureFactor();
}

double RegionImpl::Orientation() const
{
    return RegionImpl::GetContour()->EllipticAxis().z;
}

int RegionImpl::Count() const
{
    return 1;
}

int RegionImpl::CountRuns() const
{
    return static_cast<int>(rgn_runs_.size());
}

int RegionImpl::CountRows() const
{
    if (RegionImpl::Empty())
    {
        return 0;
    }
    else
    {
        const RowBeginSequence &rowRanges = RegionImpl::GetRowBeginSequence();
        return static_cast<int>(rowRanges.size() - 1);
    }
}

int RegionImpl::CountConnect() const
{
    return RegionImpl::GetContour()->CountCurves();
}

int RegionImpl::CountHoles() const
{
    return RegionImpl::GetHole()->CountCurves();
}

cv::Ptr<Region> RegionImpl::SelectObj(const int index) const
{
    if (0 == index)
    {
        return (cv::Ptr<RegionImpl>)const_cast<RegionImpl *>(this)->shared_from_this();
    }
    else
    {
        return cv::Ptr<RegionImpl>();
    }
}

cv::Ptr<Region> RegionImpl::SelectArea(const double minArea, const double maxArea) const
{
    double minA = minArea;
    double maxA = maxArea;
    if (minA < 0.)
    {
        minA = std::numeric_limits<double>::min();
    }

    if (maxA < 0.)
    {
        maxA = std::numeric_limits<double>::max();
    }

    const double A = RegionImpl::Area();
    if (A <= maxA && A >= minA)
    {
        return (cv::Ptr<RegionImpl>)const_cast<RegionImpl *>(this)->shared_from_this();
    }
    else
    {
        return cv::Ptr<RegionImpl>();
    }
}

cv::Ptr<Contour> RegionImpl::GetContour() const
{
    if (RegionImpl::Empty()) {
        return cv::makePtr<ContourImpl>();
    } else {
        if (contour_) {
            return contour_;
        } else {
            TraceContour();
            if (contour_)
            {
                return contour_;
            }
            else
            {
                return cv::makePtr<ContourImpl>();
            }
        }
    }
}

cv::Ptr<Contour> RegionImpl::GetHole() const
{
    if (RegionImpl::Empty()) {
        return cv::makePtr<ContourImpl>();
    }
    else {
        if (hole_) {
            return hole_;
        }
        else {
            TraceContour();
            if (hole_)
            {
                return hole_;
            }
            else
            {
                return cv::makePtr<ContourImpl>();
            }
        }
    }
}

cv::Ptr<Contour> RegionImpl::GetConvex() const
{
    cv::Ptr<Contour> contour = RegionImpl::GetContour();
    if (contour)
    {
        return contour->GetConvex();
    }
    return makePtr<ContourImpl>();
}

void RegionImpl::GetPoints(std::vector<cv::Point> &points) const
{
    const int numPoints = cvCeil(RegionImpl::Area());
    points.resize(numPoints);
    cv::Point *pPoints = points.data();
    for (const RunLength &rl : rgn_runs_)
    {
        for (int x=rl.colb; x < rl.cole; ++x)
        {
            pPoints->x = x;
            pPoints->y = rl.row;
            pPoints += 1;
        }
    }
}

cv::Ptr<Contour> RegionImpl::GetPolygon(const float tolerance) const
{
    cv::Ptr<Contour> contour = RegionImpl::GetContour();
    if (contour)
    {
        contour->Simplify(tolerance);
    }
    return makePtr<ContourImpl>();
}

void RegionImpl::GetRuns(std::vector<cv::Point3i> &runs) const
{
    runs.resize(rgn_runs_.size());
    cv::Point3i *pRuns = runs.data();
    for (const RunLength &rl : rgn_runs_)
    {
        pRuns->x = rl.colb;
        pRuns->y = rl.row;
        pRuns->z = rl.cole;
        pRuns += 1;
    }
}

cv::Ptr<Region> RegionImpl::Complement(const cv::Rect &universe) const
{
    const cv::Rect bbox = BoundingBox() - cv::Point(1, 1) + cv::Size(1, 1);
    const cv::Rect rcUniv = bbox | universe;

    RegionComplementOp compOp;
    RunSequence resRuns = compOp.Do(rgn_runs_, rcUniv);

    return makePtr<RegionImpl>(&resRuns);
}

cv::Ptr<Region> RegionImpl::Difference(const cv::Ptr<Region> &subRgn) const
{
    const cv::Ptr<RegionImpl> subRgnImpl = subRgn.dynamicCast<RegionImpl>();
    if (subRgnImpl)
    {
        RegionDifferenceOp diffOp;
        RunSequence resRuns = diffOp.Do(rgn_runs_, subRgnImpl->rgn_runs_);
        if (!resRuns.empty())
        {
            return makePtr<RegionImpl>(&resRuns);
        }
    }

    return makePtr<RegionImpl>();
}

cv::Ptr<Region> RegionImpl::Intersection(const cv::Ptr<Region> &otherRgn) const
{
    const cv::Ptr<RegionImpl> otherRgnImpl = otherRgn.dynamicCast<RegionImpl>();
    if (otherRgnImpl)
    {
        RegionIntersectionOp interOp;
        RunSequence resRuns = interOp.Do(rgn_runs_, otherRgnImpl->rgn_runs_);
        if (!resRuns.empty())
        {
            return makePtr<RegionImpl>(&resRuns);
        }
    }

    return makePtr<RegionImpl>();
}

cv::Ptr<Region> RegionImpl::SymmDifference(const cv::Ptr<Region> &otherRgn) const
{
    const cv::Ptr<RegionImpl> otherRgnImpl = otherRgn.dynamicCast<RegionImpl>();
    if (otherRgnImpl)
    {
        RegionSymmDifferenceOp symOp;
        RunSequence resRuns = symOp.Do(rgn_runs_, otherRgnImpl->rgn_runs_);
        if (!resRuns.empty())
        {
            return makePtr<RegionImpl>(&resRuns);
        }
    }

    return makePtr<RegionImpl>();
}

cv::Ptr<Region> RegionImpl::Union1() const
{
    return makePtr<RegionImpl>();
}

cv::Ptr<Region> RegionImpl::Union2(const cv::Ptr<Region> &otherRgn) const
{
    const cv::Ptr<RegionImpl> otherRgnImpl = otherRgn.dynamicCast<RegionImpl>();
    if (otherRgnImpl)
    {
        RegionUnion2Op unionOp;
        RunSequence resRuns;
        unionOp.Do(rgn_runs_, otherRgnImpl->rgn_runs_, resRuns);
        if (!resRuns.empty())
        {
            return makePtr<RegionImpl>(&resRuns);
        }
    }

    return makePtr<RegionImpl>();
}

bool RegionImpl::TestEqual(const cv::Ptr<Region> &otherRgn) const
{
    const cv::Ptr<RegionImpl> otherRgnImpl = otherRgn.dynamicCast<RegionImpl>();
    if (!otherRgnImpl)
    {
        return false;
    }

    if (otherRgnImpl->rgn_runs_.size() != rgn_runs_.size())
    {
        return false;
    }

    RunSequence::const_iterator itRun = rgn_runs_.cbegin();
    RunSequence::const_iterator itORun = otherRgnImpl->rgn_runs_.cbegin();
    for (; itRun != rgn_runs_.cend(); ++itRun, ++itORun)
    {
        if (itRun->row != itORun->row || itRun->colb != itORun->colb || itRun->cole != itORun->cole)
        {
            return false;
        }
    }

    return true;
}

bool RegionImpl::TestPoint(const cv::Point &point) const
{
    auto lb = std::lower_bound(rgn_runs_.cbegin(), rgn_runs_.cend(), point.y, [](const RunLength &run, const int val) { return  run.row < val; });
    while (lb != rgn_runs_.cend() && point.y == lb->row)
    {
        if (point.x < lb->cole && point.x >= lb->colb)
        {
            return true;
        }

        lb += 1;
    }

    return false;
}

bool RegionImpl::TestSubset(const cv::Ptr<Region> &otherRgn) const
{
    const cv::Ptr<RegionImpl> otherRgnImpl = otherRgn.dynamicCast<RegionImpl>();
    if (!otherRgnImpl)
    {
        return false;
    }

    if (otherRgnImpl->Empty())
    {
        return true;
    }

    RunSequence::const_pointer cur1 = otherRgnImpl->rgn_runs_.data();
    RunSequence::const_pointer cur2 = rgn_runs_.data();
    const RunSequence::const_pointer end1 = otherRgnImpl->rgn_runs_.data() + otherRgnImpl->rgn_runs_.size();
    const RunSequence::const_pointer end2 = rgn_runs_.data() + rgn_runs_.size();
    cur2 = std::lower_bound(cur2, end2, cur1->row, [](const RunLength &run, const int val) { return  run.row < val; });

    while (cur1 != end1)
    {
        while (cur2 != end2 && cur1->row != cur2->row)
        {
            ++cur2;
        }

        bool foundContainingRun = false;
        while (cur2 != end2 && cur1->row == cur2->row)
        {
            if (cur1->cole <= cur2->cole && cur1->colb >= cur2->colb)
            {
                foundContainingRun = true;
                break;
            }

            ++cur2;
        }

        if (!foundContainingRun)
        {
            return false;
        }

        ++cur1;
    }

    return true;
}

cv::Ptr<Region> RegionImpl::Move(const cv::Point &delta) const
{
    if (Empty())
    {
        return makePtr<RegionImpl>();
    }
    else
    {
        RunSequence dstRuns(rgn_runs_.size());
        std::memcpy(dstRuns.data(), rgn_runs_.data(), rgn_runs_.size() * sizeof(RunSequence::value_type));
        for (RunLength &rl : dstRuns)
        {
            rl.row += delta.y;
            rl.colb += delta.x;
            rl.cole += delta.x;
        }
        return makePtr<RegionImpl>(&dstRuns);
    }
}

cv::Ptr<Region> RegionImpl::Zoom(const cv::Size2f &scale) const
{
    if (!Empty())
    {
        TraceContour();

        int numEdges = 0;
        int maxNumPoints = 0;
        GetContourInfo(contour_, numEdges, maxNumPoints);
        GetContourInfo(hole_, numEdges, maxNumPoints);

        UScalablePointSequence v(maxNumPoints);
        UScalablePolyEdgeSequence edges(numEdges);
        UScalablePolyEdgeSequence::pointer pEdge = edges.data();
        ZoomContourToEdges(contour_, scale, v, pEdge);
        ZoomContourToEdges(hole_, scale, v, pEdge);

        edges.resize(std::distance(edges.data(), pEdge));
        if (edges.size()>1)
        {
            RunSequence runs = FillEdgeCollection_(edges);
            return makePtr<RegionImpl>(&runs);
        }
    }

    return makePtr<RegionImpl>();
}

cv::Ptr<Region> RegionImpl::Shrink(const cv::Size2f &ratio) const
{
    if (RegionImpl::Empty())
    {
        err_msg_ = "empty region";
        return Region::GenEmpty();
    }

    if (ratio.width > 1.f || ratio.height > 1.f)
    {
        err_msg_ = "ratio must between 0 and 1";
        return Region::GenEmpty();
    }

    if (ratio.width < std::numeric_limits<float>::epsilon() || ratio.height < std::numeric_limits<float>::epsilon())
    {
        err_msg_ = "ratio must between 0 and 1";
        return Region::GenEmpty();
    }

    RunSequence shrinkRuns(rgn_runs_.size());
    RunLength *lastAddedOut = shrinkRuns.data();
    for (const RunLength &rl : rgn_runs_)
    {
        const int colb = cvRound(rl.colb * ratio.width);
        const int cole = cvRound(rl.cole * ratio.width);
        if (colb != cole)
        {
            lastAddedOut->row   = cvRound(rl.row * ratio.height);
            lastAddedOut->colb  = colb;
            lastAddedOut->cole  = cole;
            lastAddedOut->label = rl.label;
            lastAddedOut += 1;
        }
    }

    const auto &compr = [](const RunLength &left, const RunLength &right) 
    { 
        return left.colb < right.colb || (left.colb == right.colb && left.cole < right.cole);
    };
    shrinkRuns.resize(std::distance(shrinkRuns.data(), lastAddedOut));

    lastAddedOut = shrinkRuns.data();
    RunLength *curIn = shrinkRuns.data() + 1;
    const RunLength *endIn = shrinkRuns.data() + shrinkRuns.size();
    while (curIn != endIn)
    {
        if (curIn->row != lastAddedOut->row)
        {
            std::sort(lastAddedOut, curIn, compr);
            lastAddedOut = curIn;
        }
        curIn += 1;
    }
    std::sort(lastAddedOut, curIn, compr);

    lastAddedOut = shrinkRuns.data();
    curIn        = shrinkRuns.data() + 1;
    endIn        = shrinkRuns.data() + shrinkRuns.size();
    while (curIn != endIn)
    {
        if (curIn->row == lastAddedOut->row && curIn->colb <= lastAddedOut->cole)
        {
            lastAddedOut->cole = std::max(curIn->cole, lastAddedOut->cole);
        }
        else
        {
            lastAddedOut += 1;
            lastAddedOut->row   = curIn->row;
            lastAddedOut->colb  = curIn->colb;
            lastAddedOut->cole  = curIn->cole;
            lastAddedOut->label = curIn->label;
        }

        curIn += 1;
    }

    shrinkRuns.resize(std::distance(shrinkRuns.data(), lastAddedOut)+1);
    if (1==shrinkRuns.size() && shrinkRuns[0].colb == shrinkRuns[1].cole)
    {
        return Region::GenEmpty();
    }
    else
    {
        return makePtr<RegionImpl>(&shrinkRuns);
    }
}

cv::Ptr<Region> RegionImpl::AffineTrans(const cv::Matx33d &homoMat2D) const
{
    if (!Empty())
    {
        TraceContour();

        int numEdges = 0;
        int maxNumPoints = 0;
        GetContourInfo(contour_, numEdges, maxNumPoints);
        GetContourInfo(hole_, numEdges, maxNumPoints);

        UScalablePointSequence v(maxNumPoints);
        UScalablePolyEdgeSequence edges(numEdges);
        UScalablePolyEdgeSequence::pointer pEdge = edges.data();

        const cv::Matx33d m0 = HomoMat2d::Translate(homoMat2D, cv::Point2d(0.5, 0.5));
        const cv::Matx33d m1 = HomoMat2d::TranslateLocal(m0, cv::Point2d(-0.5, -0.5));
        AffineContourToEdges(contour_, m1, v, pEdge);
        AffineContourToEdges(hole_, m1, v, pEdge);

        edges.resize(std::distance(edges.data(), pEdge));
        if (edges.size() > 1)
        {
            RunSequence runs = FillEdgeCollection_(edges);
            return makePtr<RegionImpl>(&runs);
        }
    }

    return makePtr<RegionImpl>();
}

cv::Ptr<Region> RegionImpl::Connect() const
{
    std::vector<cv::Ptr<Region>> rgns;
    if (RegionImpl::Empty())
    {
        return makePtr<RegionImpl>();
    }

    cv::String optVal;
    int connectivity = 8;
    GetGlobalOption("connectivity", optVal);
    if ("4" == optVal)
    {
        connectivity = 4;
    }

    const int nThreads = static_cast<int>(tbb::global_control::active_value(tbb::global_control::max_allowed_parallelism));
    const RowBeginSequence &rowRanges = GetRowBeginSequence();
    const int numRows = static_cast<int>(rowRanges.size() - 1);

    const bool is_parallel = nThreads > 1 && (numRows / nThreads) >= 2;
    if (is_parallel)
    {
        ConnectWuParallel connectWuParallel;
        connectWuParallel.Connect(this, connectivity, rgns);
    }
    else
    {
        ConnectWuSerial connectWuSerial;
        connectWuSerial.Connect(this, connectivity, rgns);
    }

    return makePtr<RegionArrayImpl>(&rgns);
}

cv::Ptr<Region> RegionImpl::Clone() const
{
    RunSequence dstRuns = rgn_runs_;
    return makePtr<RegionImpl>(&dstRuns);
}

cv::String RegionImpl::GetErrorStatus() const
{
    return err_msg_;
}

int RegionImpl::Save(const cv::String &fileName, const cv::Ptr<Dict> &opts) const
{
    return WriteToFile<RegionImpl>(*this, "region", fileName, opts, err_msg_);
}

int RegionImpl::Load(const cv::String &fileName, const cv::Ptr<Dict> &opts)
{
    return LoadFromFile<RegionImpl>(*this, "region", fileName, opts, err_msg_);
}

int RegionImpl::Serialize(const cv::String &name, H5Group *g) const
{
    err_msg_.resize(0);
    std::ostringstream bytes;
    H5GroupImpl *group = dynamic_cast<H5GroupImpl *>(g);
    if (!group || !group->Valid())
    {
        err_msg_ = "invalid database";
        return MLR_H5DB_INVALID;
    }

    try
    {
        {
            boost::iostreams::filtering_ostream fout;
            fout.push(boost::iostreams::lzma_compressor());
            fout.push(bytes);
            boost::archive::text_oarchive oa(fout);
            oa << boost::serialization::make_nvp("region", *this);
        }

        H5::DataSet dataSet;
        int r = group->SetDataSet(name, bytes.str(), dataSet);
        if (MLR_SUCCESS != r)
        {
            err_msg_ = "save database failed";
            return r;
        }

        group->SetAttribute(dataSet, cv::String("TypeGUID"), RegionImpl::TypeGUID());
        group->SetAttribute(dataSet, cv::String("Version"), 0);
    }
    catch (const std::exception &e)
    {
        err_msg_ = e.what();
        return MLR_IO_STREAM_EXCEPTION;
    }

    return MLR_SUCCESS;
}

const RowBeginSequence &RegionImpl::GetRowBeginSequence() const
{
    if (!rgn_runs_.empty() && row_begs_.empty())
    {
        GatherBasicFeatures();
    }

    return row_begs_;
}

void RegionImpl::FromMask(const cv::Mat &mask)
{
    int dph = mask.depth();
    int cnl = mask.channels();
    if (CV_8U == dph && 1 == cnl)
    {
        RunSequence &rgnRuns = const_cast<RunSequence &>(rgn_runs_);
        RunSequence allRuns = RunLengthEncode(mask, 1, 255);
        rgnRuns.swap(allRuns);
    }
}

void RegionImpl::FromPathVector(const Geom::PathVector &pv)
{
    UScalableUCharSequence buf;
    Geom::OptRect bbox = pv.boundsFast();
    if (bbox)
    {
        int t = cvFloor(bbox.get().top());
        int b = cvCeil(bbox.get().bottom()) + 1;
        int l = cvFloor(bbox.get().left());
        int r = cvCeil(bbox.get().right()) + 1;
        cv::Rect rect(cv::Point(l - 3, t - 3), cv::Point(r + 3, b + 3));
        cv::Mat mask = Util::PathToMask(pv*Geom::Translate(-rect.x, -rect.y), rect.size(), buf);
        FromMask(mask);

        for (RunLength &run : const_cast<RunSequence &>(rgn_runs_))
        {
            run.colb += rect.x;
            run.cole += rect.x;
            run.row += rect.y;
        }
    }
}

void RegionImpl::DrawVerifiedRGBA(Mat &img, const Scalar& fillColor) const
{
    const uchar R = cv::saturate_cast<uchar>(fillColor[0]);
    const uchar G = cv::saturate_cast<uchar>(fillColor[1]);
    const uchar B = cv::saturate_cast<uchar>(fillColor[2]);
    const int A = cv::saturate_cast<uchar>(fillColor[3]);
    if (255 == A)
    {
        const auto filler = [&, R, G, B](const tbb::blocked_range<int> &br)
        {
            RunSequence::const_pointer pRun = rgn_runs_.data() + br.begin();
            const RunSequence::const_pointer pRunEnd = rgn_runs_.data() + br.end();
            for (; pRun != pRunEnd; ++pRun)
            {
                if (pRun->row >= 0 && pRun->row < img.rows)
                {
                    const int colb = std::max(pRun->colb, 0) * 4;
                    const int cole = std::min(pRun->cole, img.cols) * 4;
                    uchar *pRow = img.ptr<uchar>(pRun->row);
                    for (int col = colb; col < cole; col += 4)
                    {
                        uchar *pSrcPixel = pRow + col;
                        pSrcPixel[0] = B;
                        pSrcPixel[1] = G;
                        pSrcPixel[2] = R;
                        pSrcPixel[3] = 255;
                    }
                }
            }
        };

        if (rgn_runs_.size() > 64)
        {
            tbb::parallel_for(tbb::blocked_range<int>(0, static_cast<int>(rgn_runs_.size())), filler);
        }
        else
        {
            filler(tbb::blocked_range<int>(0, static_cast<int>(rgn_runs_.size())));
        }
    }
    else if (0 == A)
    {
        // Not need to do anything
    }
    else
    {
        const int beta = 255 - A;
        const int r = A * R;
        const int g = A * G;
        const int b = A * B;
        const auto filler = [&, R, G, B](const tbb::blocked_range<int> &br)
        {
            RunSequence::const_pointer pRun = rgn_runs_.data() + br.begin();
            const RunSequence::const_pointer pRunEnd = rgn_runs_.data() + br.end();
            for (; pRun != pRunEnd; ++pRun)
            {
                if (pRun->row >= 0 && pRun->row < img.rows)
                {
                    const int colb = std::max(pRun->colb, 0) * 4;
                    const int cole = std::min(pRun->cole, img.cols) * 4;
                    uchar *pRow = img.ptr<uchar>(pRun->row);
                    for (int col = colb; col < cole; col += 4)
                    {
                        uchar *pSrcPixel = pRow + col;
                        pSrcPixel[0] = static_cast<uchar>((pSrcPixel[0] * beta + b) / 255);
                        pSrcPixel[1] = static_cast<uchar>((pSrcPixel[1] * beta + g) / 255);
                        pSrcPixel[2] = static_cast<uchar>((pSrcPixel[2] * beta + r) / 255);
                        pSrcPixel[3] = 255;
                    }
                }
            }
        };

        if (rgn_runs_.size() > 64)
        {
            tbb::parallel_for(tbb::blocked_range<int>(0, static_cast<int>(rgn_runs_.size())), filler);
        }
        else
        {
            filler(tbb::blocked_range<int>(0, static_cast<int>(rgn_runs_.size())));
        }
    }
}

void RegionImpl::DrawVerifiedRGB(Mat &img, const Scalar& fillColor) const
{
    const uchar R = cv::saturate_cast<uchar>(fillColor[0]);
    const uchar G = cv::saturate_cast<uchar>(fillColor[1]);
    const uchar B = cv::saturate_cast<uchar>(fillColor[2]);
    const int A   = cv::saturate_cast<uchar>(fillColor[3]);
    if (255 == A)
    {
        const auto filler = [&, R, G, B](const tbb::blocked_range<int> &br)
        {
            RunSequence::const_pointer pRun = rgn_runs_.data() + br.begin();
            const RunSequence::const_pointer pRunEnd = rgn_runs_.data() + br.end();
            for (; pRun != pRunEnd; ++pRun)
            {
                if (pRun->row >= 0 && pRun->row < img.rows)
                {
                    const int colb = std::max(pRun->colb, 0) * 3;
                    const int cole = std::min(pRun->cole, img.cols) * 3;
                    uchar *pRow = img.ptr<uchar>(pRun->row);
                    for (int col = colb; col < cole; col += 3)
                    {
                        uchar *pSrcPixel = pRow + col;
                        pSrcPixel[0] = B;
                        pSrcPixel[1] = G;
                        pSrcPixel[2] = R;
                    }
                }
            }
        };

        if (rgn_runs_.size() > 64)
        {
            tbb::parallel_for(tbb::blocked_range<int>(0, static_cast<int>(rgn_runs_.size())), filler);
        }
        else
        {
            filler(tbb::blocked_range<int>(0, static_cast<int>(rgn_runs_.size())));
        }
    }
    else if (0 == A)
    {
        // Not need to do anything
    }
    else
    {
        const int beta = 255 - A;
        const int r = A * R;
        const int g = A * G;
        const int b = A * B;
        const auto filler = [&, R, G, B](const tbb::blocked_range<int> &br)
        {
            RunSequence::const_pointer pRun = rgn_runs_.data() + br.begin();
            const RunSequence::const_pointer pRunEnd = rgn_runs_.data() + br.end();
            for (; pRun != pRunEnd; ++pRun)
            {
                if (pRun->row >= 0 && pRun->row < img.rows)
                {
                    const int colb = std::max(pRun->colb, 0) * 3;
                    const int cole = std::min(pRun->cole, img.cols) * 3;
                    uchar *pRow = img.ptr<uchar>(pRun->row);
                    for (int col = colb; col < cole; col += 3)
                    {
                        uchar *pSrcPixel = pRow + col;
                        pSrcPixel[0] = static_cast<uchar>((pSrcPixel[0] * beta + b) / 255);
                        pSrcPixel[1] = static_cast<uchar>((pSrcPixel[1] * beta + g) / 255);
                        pSrcPixel[2] = static_cast<uchar>((pSrcPixel[2] * beta + r) / 255);
                    }
                }
            }
        };

        if (rgn_runs_.size() > 64)
        {
            tbb::parallel_for(tbb::blocked_range<int>(0, static_cast<int>(rgn_runs_.size())), filler);
        }
        else
        {
            filler(tbb::blocked_range<int>(0, static_cast<int>(rgn_runs_.size())));
        }
    }
}

void RegionImpl::DrawVerifiedGray(Mat &img, const Scalar& fillColor) const
{
    const uchar gray = cv::saturate_cast<uchar>((fillColor[0] + fillColor[1] + fillColor[2]) / 3);
    const uchar alpha = cv::saturate_cast<uchar>(fillColor[3]);
    if (255==alpha)
    {
        const auto filler = [&, gray](const tbb::blocked_range<int> &br)
        {
            RunSequence::const_pointer pRun = rgn_runs_.data() + br.begin();
            const RunSequence::const_pointer pRunEnd = rgn_runs_.data() + br.end();
            for (; pRun != pRunEnd; ++pRun)
            {
                if (pRun->row >= 0 && pRun->row < img.rows)
                {
                    const int colb = std::max(pRun->colb, 0);
                    const int cole = std::min(pRun->cole, img.cols);
                    if (colb < cole)
                    {
                        std::memset(img.ptr<uchar>(pRun->row) + colb, gray, cole - colb);
                    }
                }
            }
        };

        if (rgn_runs_.size()>64)
        {
            tbb::parallel_for(tbb::blocked_range<int>(0, static_cast<int>(rgn_runs_.size())), filler);
        }
        else
        {
            filler(tbb::blocked_range<int>(0, static_cast<int>(rgn_runs_.size())));
        }
    }
    else if (0 == alpha)
    {
        // Not need to do anything
    }
    else
    {
        const int a = alpha;
        const int b = 255 - a;
        const int c = a * gray;
        const auto filler = [&, c](const tbb::blocked_range<int> &br)
        {
            RunSequence::const_pointer pRun = rgn_runs_.data() + br.begin();
            const RunSequence::const_pointer pRunEnd = rgn_runs_.data() + br.end();
            for (; pRun != pRunEnd; ++pRun)
            {
                if (pRun->row >= 0 && pRun->row < img.rows)
                {
                    const int colb = std::max(pRun->colb, 0);
                    const int cole = std::min(pRun->cole, img.cols);
                    uchar *pRow = img.ptr<uchar>(pRun->row);
                    for (int col = colb ; col < cole; ++col)
                    {
                        pRow[col] = static_cast<uchar>((pRow[col]*b + c)/255);
                    }
                }
            }
        };

        if (rgn_runs_.size() > 64)
        {
            tbb::parallel_for(tbb::blocked_range<int>(0, static_cast<int>(rgn_runs_.size())), filler);
        }
        else
        {
            filler(tbb::blocked_range<int>(0, static_cast<int>(rgn_runs_.size())));
        }
    }
}

void RegionImpl::TraceContour() const
{
    if (!rgn_runs_.empty() && !contour_ && !hole_)
    {
        const auto &rowBegs = GetRowBeginSequence();

        const int nThreads = static_cast<int>(tbb::global_control::active_value(tbb::global_control::max_allowed_parallelism));
        const bool is_parallel = nThreads > 1 && (rgn_runs_.size() / nThreads) >= 1024*10;

        if (is_parallel)
        {
            RunLengthRDParallelEncoder rdEncoder;
            rdEncoder.Encode(rgn_runs_.data(), rgn_runs_.data() + rgn_runs_.size(), rowBegs);
            rdEncoder.Link();
            ScalablePoint2fSequenceSequence outers, holes;
            rdEncoder.Track(outers, holes);
            contour_ = makePtr<ContourImpl>(&outers, K_NO);
            hole_ = makePtr<ContourImpl>(&holes, K_NO);
        }
        else
        {
            RunLengthRDSerialEncoder rdEncoder;
            rdEncoder.Encode(rgn_runs_.data(), rgn_runs_.data() + rgn_runs_.size(), rowBegs);
            rdEncoder.Link();
            ScalablePoint2fSequenceSequence outers, holes;
            rdEncoder.Track(outers, holes);
            contour_ = makePtr<ContourImpl>(&outers, K_NO);
            hole_    = makePtr<ContourImpl>(&holes, K_NO);
        }
    }
}

void RegionImpl::GatherBasicFeatures() const
{
    if (!rgn_runs_.empty())
    {
        row_begs_.reserve(rgn_runs_.back().row-rgn_runs_.front().row + 2);
        const int numRuns = static_cast<int>(rgn_runs_.size());
        int currentRow = rgn_runs_.front().row;

        double a = 0, x = 0, y = 0;
        cv::Point minPoint{ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
        cv::Point maxPoint{ std::numeric_limits<int>::min(), std::numeric_limits<int>::min() };

        int begIdx = 0;
        for (int runIdx = 0; runIdx < numRuns; ++runIdx)
        {
            const RunLength &rl = rgn_runs_[runIdx];
            if (rl.row != currentRow)
            {
                row_begs_.emplace_back(begIdx); begIdx = runIdx; currentRow = rl.row;
            }

            const auto n = rl.cole - rl.colb;
            a += n;
            x += (rl.cole - 1 + rl.colb) * n / 2.0;
            y += rl.row * n;

            if (rl.row < minPoint.y) { minPoint.y = rl.row; }
            if (rl.row > maxPoint.y) { maxPoint.y = rl.row; }
            if (rl.colb < minPoint.x) { minPoint.x = rl.colb; }
            if (rl.cole > maxPoint.x) { maxPoint.x = rl.cole; }
        }

        row_begs_.emplace_back(begIdx);
        row_begs_.emplace_back(numRuns);

        area_ = a;
        if (a > 0) {
            centroid_.emplace(x / a, y / a);
        } else {
            centroid_.emplace(0, 0);
        }

        maxPoint.y += 1;
        bbox_ = cv::Rect(minPoint, maxPoint);
    }
    else
    {
        area_.emplace(0);
        bbox_.emplace(0, 0, 0, 0);
        centroid_.emplace(0, 0);
    }
}

double RegionImpl::MaxSqrRadius() const
{
    if (boost::none == max_radius_)
    {
        cv::Point2d cPt = Centroid();
        double maxRadius = 0.;
        for (const auto &run : rgn_runs_)
        {
            cv::Point2d sPt( run.colb, run.row );
            cv::Point2d ePt( run.cole - 1, run.row );
            const double sRad = cv::normL2Sqr<double>(cPt-sPt);
            const double eRad = cv::normL2Sqr<double>(cPt-ePt);
            if (sRad > maxRadius)
            {
                maxRadius = sRad;
            }
            if (eRad > maxRadius)
            {
                maxRadius = eRad;
            }
        }
        max_radius_ = maxRadius;
    }

    return *max_radius_;
}

void RegionImpl::GetContourInfo(const cv::Ptr<Contour> &contour, int &numEdges, int &maxNumPoints) const
{
    if (contour && contour->TestClosed())
    {
        cv::Ptr<ContourImpl> contourArray = contour.dynamicCast<ContourImpl>();
        const ScalablePoint2fSequenceSequence &curves = contourArray->Curves();
        for (const auto &crv : curves)
        {
            const int numpoints = static_cast<int>(crv.size()-1);
            numEdges += (numpoints + 1);
            maxNumPoints = std::max(maxNumPoints, numpoints);
        }
    }
}

void RegionImpl::ZoomContourToEdges(const cv::Ptr<Contour> &contour,
    const cv::Size2f &scale,
    UScalablePointSequence &v,
    UScalablePolyEdgeSequence::pointer &pEdge) const
{
    if (contour && contour->TestClosed())
    {
        cv::Ptr<ContourImpl> contourArray = contour.dynamicCast<ContourImpl>();
        const auto &curves = contourArray->Curves();
        for (const auto &crv : curves)
        {
            const int numpoints = static_cast<int>(crv.size() - 1);
            const UScalablePointSequence::pointer pvEnd = v.data() + numpoints;
            auto vf = crv.data();
            for (UScalablePointSequence::pointer pv = v.data(); pv != pvEnd; ++pv, ++vf)
            {
                pv->x = cvRound(vf->x * F_XY_ONE * scale.width);
                pv->y = cvRound(vf->y * F_XY_ONE * scale.height);
            }

            CollectPolyEdges_(v.data(), numpoints, pEdge);
        }
    }
}

void RegionImpl::AffineContourToEdges(const cv::Ptr<Contour> &contour,
    const cv::Matx33d &m,
    UScalablePointSequence &v,
    UScalablePolyEdgeSequence::pointer &pEdge) const
{
    if (contour && contour->TestClosed())
    {
        cv::Ptr<ContourImpl> contourArray = contour.dynamicCast<ContourImpl>();
        const auto &curves = contourArray->Curves();
        for (const auto &crv : curves)
        {
            const int numpoints = static_cast<int>(crv.size() - 1);
            const UScalablePointSequence::pointer pvEnd = v.data() + numpoints;
            auto vf = crv.data();
            for (UScalablePointSequence::pointer pv = v.data(); pv != pvEnd; ++pv, ++vf)
            {
                pv->x = cvRound((m.val[0] * vf->x + m.val[1] * vf->y + m.val[2])*D_XY_ONE);
                pv->y = cvRound((m.val[3] * vf->x + m.val[4] * vf->y + m.val[5])*D_XY_ONE);
            }
            CollectPolyEdges_(v.data(), numpoints, pEdge);
        }
    }
}

void CollectPolyEdges_(const cv::Point* v, const int count, UScalablePolyEdgeSequence::pointer &pEdge)
{
    cv::Point pt0 = v[count - 1], pt1;
    pt0.y = (pt0.y + XY_DELTA) >> XY_SHIFT;

    for (int i = 0; i < count; i++, pt0 = pt1)
    {
        pt1 = v[i];
        pt1.y = (pt1.y + XY_DELTA) >> XY_SHIFT;

        if (pt0.y == pt1.y)
            continue;

        if (pt0.y < pt1.y)
        {
            pEdge->y0 = pt0.y;
            pEdge->y1 = pt1.y;
            pEdge->x = pt0.x;
        }
        else
        {
            pEdge->y0 = pt1.y;
            pEdge->y1 = pt0.y;
            pEdge->x = pt1.x;
        }

        pEdge->dx = (pt1.x - pt0.x) / (pt1.y - pt0.y);
        ++pEdge;
    }
}

RunSequence FillEdgeCollection_(UScalablePolyEdgeSequence &edges)
{
    int total = static_cast<int>(edges.size());
    int y_max = INT_MIN, y_min = INT_MAX;
    int x_max = INT_MIN, x_min = INT_MAX;

    for (const auto &e1 : edges)
    {
        assert(e1.y0 < e1.y1);
        const int x1 = e1.x + (e1.y1 - e1.y0) * e1.dx;
        y_min = std::min(y_min, e1.y0);
        y_max = std::max(y_max, e1.y1);
        x_min = std::min(x_min, e1.x);
        x_max = std::max(x_max, e1.x);
        x_min = std::min(x_min, x1);
        x_max = std::max(x_max, x1);
    }

    if (y_min > y_max)
    {
        return RunSequence();
    }

    std::sort(edges.begin(), edges.end(), CmpEdges());

    // start drawing
    PolyEdge tmp;
    tmp.y0 = INT_MAX;
    edges.push_back(tmp); // after this point we do not add
                          // any elements to edges, thus we can use pointers
    int i = 0;
    tmp.next = 0;
    PolyEdge *e = &edges[i];

    RunSequence dstRuns;
    dstRuns.reserve((y_max - y_min + 1) * 3);
    for (int y = e->y0; y < y_max; y++)
    {
        PolyEdge *last, *prelast, *keep_prelast;
        int sort_flag = 0;
        int draw = 0;

        prelast = &tmp;
        last = tmp.next;
        while (last || e->y0 == y)
        {
            if (last && last->y1 == y)
            {
                // exclude edge if y reaches its lower point
                prelast->next = last->next;
                last = last->next;
                continue;
            }
            keep_prelast = prelast;
            if (last && (e->y0 > y || last->x < e->x))
            {
                // go to the next edge in active list
                prelast = last;
                last = last->next;
            }
            else if (i < total)
            {
                // insert new edge into active list if y reaches its upper point
                prelast->next = e;
                e->next = last;
                prelast = e;
                e = &edges[++i];
            }
            else
                break;

            if (draw)
            {
                // convert x's from fixed-point to image coordinates
                int x1, x2;
                if (keep_prelast->x > prelast->x)
                {
                    x1 = prelast->x >> XY_SHIFT;
                    x2 = (keep_prelast->x + XY_DELTA) >> XY_SHIFT;
                }
                else
                {
                    x1 = keep_prelast->x >> XY_SHIFT;
                    x2 = (prelast->x + XY_DELTA) >> XY_SHIFT;
                }

                if (!dstRuns.empty() && y == dstRuns.back().row && x1 <= dstRuns.back().cole)
                {
                    dstRuns.back().cole = x2 + 1;
                }
                else
                {
                    dstRuns.emplace_back(y, x1, x2 + 1);
                }

                keep_prelast->x += keep_prelast->dx;
                prelast->x += prelast->dx;
            }
            draw ^= 1;
        }

        // sort edges (using bubble sort)
        keep_prelast = 0;

        do
        {
            prelast = &tmp;
            last = tmp.next;

            while (last != keep_prelast && last->next != 0)
            {
                PolyEdge *te = last->next;

                // swap edges
                if (last->x > te->x)
                {
                    prelast->next = te;
                    last->next = te->next;
                    te->next = last;
                    prelast = te;
                    sort_flag = 1;
                }
                else
                {
                    prelast = last;
                    last = te;
                }
            }
            keep_prelast = prelast;
        } while (sort_flag && keep_prelast != tmp.next && keep_prelast != &tmp);
    }

    return dstRuns;
}

}
}
