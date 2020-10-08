#include "precomp.hpp"
#include "region_impl.hpp"
#include "region_array_impl.hpp"
#include <utility.hpp>
#include <contour/contour_array_impl.hpp>
#include <hdf5/h5group_impl.hpp>
#include <opencv2/mvlab.hpp>

BOOST_CLASS_EXPORT_IMPLEMENT(cv::mvlab::RegionArrayImpl)

namespace cv {
namespace mvlab {
const cv::String RegionArrayImpl::type_guid_s{ "A4278614-568E-4854-B0B2-A1875EDB1189" };

class SumArea {
    const std::vector<cv::Ptr<Region>> &rgns;
public:
    double area_sum;
    void operator()(const tbb::blocked_range<std::size_t>& r)
    {
        for (std::size_t i = r.begin(); i != r.end(); ++i) area_sum += rgns[i]->Area();
    }

    SumArea(const std::vector<cv::Ptr<Region>> &rs) : rgns(rs), area_sum(0) {}
    SumArea(SumArea& x, tbb::split) : rgns(x.rgns), area_sum(0) {}
    void join(const SumArea& y) { area_sum += y.area_sum; }
};

class SumHolesArea {
    const std::vector<cv::Ptr<Region>> &rgns;
public:
    double holes_area_sum;
    void operator()(const tbb::blocked_range<std::size_t>& r)
    {
        for (std::size_t i = r.begin(); i != r.end(); ++i) holes_area_sum += rgns[i]->AreaHoles();
    }

    SumHolesArea(const std::vector<cv::Ptr<Region>> &rs) : rgns(rs), holes_area_sum(0) {}
    SumHolesArea(SumHolesArea& x, tbb::split) : rgns(x.rgns), holes_area_sum(0) {}
    void join(const SumHolesArea& y) { holes_area_sum += y.holes_area_sum; }
};

class SumContLength {
    const std::vector<cv::Ptr<Region>> &rgns;
public:
    double len_sum;
    void operator()(const tbb::blocked_range<std::size_t>& r)
    {
        for (std::size_t i = r.begin(); i != r.end(); ++i) len_sum += rgns[i]->Contlength();
    }

    SumContLength(const std::vector<cv::Ptr<Region>> &rs) : rgns(rs), len_sum(0) {}
    SumContLength(SumContLength& x, tbb::split) : rgns(x.rgns), len_sum(0) {}
    void join(const SumContLength& y) { len_sum += y.len_sum; }
};

class SumCentroid {
    const std::vector<cv::Ptr<Region>> &rgns;
public:
    double area_sum;
    double x_sum;
    double y_sum;
    void operator()(const tbb::blocked_range<std::size_t>& r)
    {
        for (std::size_t i = r.begin(); i != r.end(); ++i)
        {
            const double a = rgns[i]->Area();
            const cv::Point2d c = rgns[i]->Centroid();
            area_sum += a;
            x_sum += c.x * a;
            y_sum += c.y * a;
        }
    }

    SumCentroid(const std::vector<cv::Ptr<Region>> &rs) : rgns(rs), area_sum(0), x_sum(0), y_sum(0) {}
    SumCentroid(SumCentroid& x, tbb::split) : rgns(x.rgns), area_sum(0), x_sum(0), y_sum(0) {}
    void join(const SumCentroid& y) { area_sum += y.area_sum; x_sum += y.x_sum; y_sum += y.y_sum; }
};

class SumBoundingBox {
    const std::vector<cv::Ptr<Region>> &rgns;
public:
    cv::Rect bbx_sum;
    void operator()(const tbb::blocked_range<std::size_t>& r)
    {
        for (std::size_t i = r.begin(); i != r.end(); ++i) bbx_sum |= rgns[i]->BoundingBox();
    }

    SumBoundingBox(const std::vector<cv::Ptr<Region>> &rs) : rgns(rs) {}
    SumBoundingBox(SumBoundingBox& x, tbb::split) : rgns(x.rgns) {}
    void join(const SumBoundingBox& y) { bbx_sum |= y.bbx_sum; }
};

class SumConnectCounter {
    const std::vector<cv::Ptr<Region>> &rgns;
public:
    int connect_sum;
    void operator()(const tbb::blocked_range<std::size_t>& r)
    {
        for (std::size_t i = r.begin(); i != r.end(); ++i) connect_sum += rgns[i]->CountConnect();
    }

    SumConnectCounter(const std::vector<cv::Ptr<Region>> &rs) : rgns(rs), connect_sum(0) {}
    SumConnectCounter(SumConnectCounter& x, tbb::split) : rgns(x.rgns), connect_sum(0) {}
    void join(const SumConnectCounter& y) { connect_sum += y.connect_sum; }
};

class SumHolesCounter {
    const std::vector<cv::Ptr<Region>> &rgns;
public:
    int num_holes_sum;
    void operator()(const tbb::blocked_range<std::size_t>& r)
    {
        for (std::size_t i = r.begin(); i != r.end(); ++i) num_holes_sum += rgns[i]->CountHoles();
    }

    SumHolesCounter(const std::vector<cv::Ptr<Region>> &rs) : rgns(rs), num_holes_sum(0) {}
    SumHolesCounter(SumHolesCounter& x, tbb::split) : rgns(x.rgns), num_holes_sum(0) {}
    void join(const SumHolesCounter& y) { num_holes_sum += y.num_holes_sum; }
};

RegionArrayImpl::RegionArrayImpl(const cv::Mat &allRuns, const cv::Mat &delimits)
{
    if (allRuns.empty() || delimits.empty())
    {
        return;
    }

    if (CV_32S != allRuns.depth() || 4 != allRuns.channels() || 2 != allRuns.dims || 1 != allRuns.cols)
    {
        return;
    }

    if (CV_32S != delimits.depth() || 1 != delimits.channels() || 2 != delimits.dims || 1 != delimits.cols)
    {
        return;
    }

    int allNumRuns = std::accumulate(reinterpret_cast<int*>(delimits.data), reinterpret_cast<int*>(delimits.data) + delimits.rows, 0);
    if (allNumRuns != allRuns.rows)
    {
        return;
    }

    int index = 0;
    std::vector<cv::Ptr<Region>> rgns;
    for (int n=0; n<delimits.rows; ++n)
    {
        int numRuns = delimits.at<int>(cv::Vec<int, 2>(n, 0));
        cv::Mat runs(allRuns, cv::Range(index, index + numRuns));
        rgns.push_back(cv::makePtr<RegionImpl>(runs));
        index += numRuns;
    }

    rgns_.swap(rgns);
}

RegionArrayImpl::RegionArrayImpl(const std::string &bytes)
{
    try
    {
        std::istringstream iss(bytes);
        boost::iostreams::filtering_istream fin;
        fin.push(boost::iostreams::lzma_decompressor());
        fin.push(iss);

        boost::archive::text_iarchive ia(fin);
        ia >> boost::serialization::make_nvp("regions", *this);
    }
    catch (const std::exception &e)
    {
        err_msg_ = e.what();
    }
}

int RegionArrayImpl::Draw(cv::Mat &img,
    const cv::Scalar& fillColor) const
{
    if (img.empty())
    {
        cv::Rect bbox = RegionArrayImpl::BoundingBox();
        if (bbox.width > 0 && bbox.height > 0)
        {
            img = Mat::ones(bbox.br().y + 1, bbox.br().x + 1, CV_8UC4) * 255;
        }
        else
        {
            return MLR_CONTOUR_EMPTY;
        }
    }

    if (img.empty())
    {
        return MLR_IMAGE_EMPTY;
    }

    for (const cv::Ptr<Region> &c : rgns_)
    {
        int res = c->Draw(img, fillColor);
        if (MLR_SUCCESS != res)
        {
            return res;
        }
    }

    return MLR_SUCCESS;
}

int RegionArrayImpl::Draw(cv::InputOutputArray img,
    const cv::Scalar& fillColor) const
{
    Mat imgMat = img.getMat();
    if (imgMat.empty())
    {
        int res = RegionArrayImpl::Draw(imgMat, fillColor);
        img.assign(imgMat);
        return res;
    }
    else
    {
        int dph = img.depth();
        int cnl = img.channels();
        if (CV_8U == dph && (1 == cnl || 3 == cnl || 4 == cnl))
        {
            int rest = RegionArrayImpl::Draw(imgMat, fillColor);
            img.assign(imgMat);
            return rest;
        }
        else
        {
            return MLR_IMAGE_FORMAT_ERROR;
        }
    }
}

bool RegionArrayImpl::Empty() const
{
    return rgns_.empty() || std::all_of(rgns_.cbegin(), rgns_.cend(), [](const cv::Ptr<Region> &item) { return item->Empty(); });
}

double RegionArrayImpl::Area() const
{
    SumArea sa(rgns_);
    if (IsNeedDoParallel())
    {
        tbb::parallel_reduce(tbb::blocked_range<std::size_t>(0, rgns_.size()), sa);
    }
    else
    {
        sa(tbb::blocked_range<std::size_t>(0, rgns_.size()));
    }

    return sa.area_sum;
}

cv::Point2d RegionArrayImpl::Centroid() const
{
    SumCentroid sc(rgns_);
    if (IsNeedDoParallel())
    {
        tbb::parallel_reduce(tbb::blocked_range<std::size_t>(0, rgns_.size()), sc);
    }
    else
    {
        sc(tbb::blocked_range<std::size_t>(0, rgns_.size()));
    }

    if (sc.area_sum < G_D_TOL)
    {
        return cv::Point2d();
    }
    else
    {
        return cv::Point2d(sc.x_sum / sc.area_sum, sc.y_sum / sc.area_sum);
    }
}

cv::Rect RegionArrayImpl::BoundingBox() const
{
    SumBoundingBox sbb(rgns_);
    if (IsNeedDoParallel())
    {
        tbb::parallel_reduce(tbb::blocked_range<std::size_t>(0, rgns_.size()), sbb);
    }
    else
    {
        sbb(tbb::blocked_range<std::size_t>(0, rgns_.size()));
    }

    return sbb.bbx_sum;
}

cv::Point3d RegionArrayImpl::SmallestCircle() const
{
    return cv::Point3d();
}

double RegionArrayImpl::AreaHoles() const
{
    SumHolesArea sha(rgns_);
    if (IsNeedDoParallel())
    {
        tbb::parallel_reduce(tbb::blocked_range<std::size_t>(0, rgns_.size()), sha);
    }
    else
    {
        sha(tbb::blocked_range<std::size_t>(0, rgns_.size()));
    }

    return sha.holes_area_sum;
}

double RegionArrayImpl::Contlength() const
{
    SumContLength scl(rgns_);
    if (IsNeedDoParallel())
    {
        tbb::parallel_reduce(tbb::blocked_range<std::size_t>(0, rgns_.size()), scl);
    }
    else
    {
        scl(tbb::blocked_range<std::size_t>(0, rgns_.size()));
    }

    return scl.len_sum;
}

int RegionArrayImpl::Count() const
{
    return static_cast<int>(rgns_.size());
}

int RegionArrayImpl::CountRuns() const
{
    int numRuns = 0;
    for (const cv::Ptr<Region> &rgn : rgns_)
    {
        numRuns += rgn->CountRuns();
    }
    return numRuns;
}

int RegionArrayImpl::CountRows() const
{
    int numRows = 0;
    for (const cv::Ptr<Region> &rgn : rgns_)
    {
        numRows += rgn->CountRows();
    }
    return numRows;
}

int RegionArrayImpl::CountConnect() const
{
    SumConnectCounter scc(rgns_);
    if (IsNeedDoParallel())
    {
        tbb::parallel_reduce(tbb::blocked_range<std::size_t>(0, rgns_.size()), scc);
    }
    else
    {
        scc(tbb::blocked_range<std::size_t>(0, rgns_.size()));
    }

    return scc.connect_sum;
}

int RegionArrayImpl::CountHoles() const
{
    SumHolesCounter shc(rgns_);
    if (IsNeedDoParallel())
    {
        tbb::parallel_reduce(tbb::blocked_range<std::size_t>(0, rgns_.size()), shc);
    }
    else
    {
        shc(tbb::blocked_range<std::size_t>(0, rgns_.size()));
    }

    return shc.num_holes_sum;
}

cv::Ptr<Region> RegionArrayImpl::SelectObj(const int index) const
{
    if (index>=0 && index < RegionArrayImpl::Count())
    {
        return rgns_[index];
    }
    else
    {
        return cv::Ptr<RegionImpl>();
    }
}

cv::Ptr<Contour> RegionArrayImpl::GetContour() const
{
    std::vector<cv::Ptr<Contour>> contours(rgns_.size());
    tbb::parallel_for(0, (int)rgns_.size(), 1, [&](const int i) { contours[i] = rgns_[i]->GetContour(); });
    return cv::makePtr<ContourArrayImpl>(&contours);
}

cv::Ptr<Contour> RegionArrayImpl::GetHole() const
{
    std::vector<cv::Ptr<Contour>> holes(rgns_.size());
    tbb::parallel_for(0, (int)rgns_.size(), 1, [&](const int i) { holes[i] = rgns_[i]->GetHole(); });
    return cv::makePtr<ContourArrayImpl>(&holes);
}

cv::Ptr<Contour> RegionArrayImpl::GetConvex() const
{
    std::vector<cv::Ptr<Contour>> cvxs(rgns_.size());
    tbb::parallel_for(0, (int)rgns_.size(), 1, [&](const int i) { cvxs[i] = rgns_[i]->GetConvex(); });
    return cv::makePtr<ContourArrayImpl>(&cvxs);
}

void RegionArrayImpl::GetPoints(std::vector<cv::Point> &points) const
{
    points.resize(cvCeil(RegionArrayImpl::Area()));

    double maxNumPoints = 0;
    for (const cv::Ptr<Region> &rgn : rgns_)
    {
        const double a = rgn->Area();
        if (a > maxNumPoints)
        {
            maxNumPoints = a;
        }
    }

    cv::Point *pDst = points.data();
    std::vector<cv::Point> tPoints(cvCeil(maxNumPoints));
    for (const cv::Ptr<Region> &rgn : rgns_)
    {
        rgn->GetPoints(tPoints);
        std::memcpy(pDst, tPoints.data(), sizeof(cv::Point)*tPoints.size());
        pDst += tPoints.size();
    }
}

cv::Ptr<Contour> RegionArrayImpl::GetPolygon(const float tolerance) const
{
    std::vector<cv::Ptr<Contour>> plgs(rgns_.size());
    tbb::parallel_for(0, (int)rgns_.size(), 1, [&](const int i) { plgs[i] = rgns_[i]->GetPolygon(tolerance); });
    return cv::makePtr<ContourArrayImpl>(&plgs);
}

void RegionArrayImpl::GetRuns(std::vector<cv::Point3i> &runs) const
{
    int numRuns = 0;
    int maxNumRuns = 0;
    for (const cv::Ptr<Region> &rgn : rgns_)
    {
        numRuns += rgn->CountRuns();
        maxNumRuns = std::max(rgn->CountRuns(), maxNumRuns);
    }

    runs.resize(numRuns);
    cv::Point3i *pDst = runs.data();
    std::vector<cv::Point3i> tRuns(maxNumRuns);
    for (const cv::Ptr<Region> &rgn : rgns_)
    {
        rgn->GetRuns(tRuns);
        std::memcpy(pDst, tRuns.data(), sizeof(cv::Point3i)*tRuns.size());
        pDst += tRuns.size();
    }
}

cv::Ptr<Region> RegionArrayImpl::Complement(const cv::Rect &universe) const
{
    std::vector<cv::Ptr<Region>> rgns(rgns_.size());
    tbb::parallel_for(0, (int)rgns_.size(), 1, [&](const int i) { rgns[i] = rgns_[i]->Complement(universe); });
    return makePtr<RegionArrayImpl>(&rgns);
}

cv::Ptr<Region> RegionArrayImpl::Difference(const cv::Ptr<Region> &subRgn) const
{
    std::vector<cv::Ptr<Region>> rgns(rgns_.size());
    tbb::parallel_for(0, (int)rgns_.size(), 1, [&](const int i) { rgns[i] = rgns_[i]->Difference(subRgn); });
    return makePtr<RegionArrayImpl>(&rgns);
}

cv::Ptr<Region> RegionArrayImpl::Intersection(const cv::Ptr<Region> &otherRgn) const
{
    std::vector<cv::Ptr<Region>> rgns(rgns_.size());
    tbb::parallel_for(0, (int)rgns_.size(), 1, [&](const int i) { rgns[i] = rgns_[i]->Intersection(otherRgn); });
    return makePtr<RegionArrayImpl>(&rgns);
}

cv::Ptr<Region> RegionArrayImpl::SymmDifference(const cv::Ptr<Region> &otherRgn) const
{
    std::vector<cv::Ptr<Region>> rgns(rgns_.size());
    tbb::parallel_for(0, (int)rgns_.size(), 1, [&](const int i) { rgns[i] = rgns_[i]->SymmDifference(otherRgn); });
    return makePtr<RegionArrayImpl>(&rgns);
}

cv::Ptr<Region> RegionArrayImpl::Union1() const
{
    return makePtr<RegionImpl>();
}

cv::Ptr<Region> RegionArrayImpl::Union2(const cv::Ptr<Region> &otherRgn) const
{
    std::vector<cv::Ptr<Region>> rgns(rgns_.size());
    tbb::parallel_for(0, (int)rgns_.size(), 1, [&](const int i) { rgns[i] = rgns_[i]->Union2(otherRgn); });
    return makePtr<RegionArrayImpl>(&rgns);
}

bool RegionArrayImpl::TestEqual(const cv::Ptr<Region> &otherRgn) const
{
    if (!otherRgn)
    {
        return false;
    }

    if (RegionArrayImpl::Count() != otherRgn->Count())
    {
        return false;
    }

    const int numRgns = RegionArrayImpl::Count();
    for (int idx=0; idx<numRgns; ++idx)
    {
        if (!rgns_[idx]->TestEqual(otherRgn->SelectObj(idx)))
        {
            return false;
        }
    }

    return true;
}

bool RegionArrayImpl::TestPoint(const cv::Point &point) const
{
    for (const cv::Ptr<Region> &rgn : rgns_)
    {
        if (rgn->TestPoint(point))
        {
            return true;
        }
    }

    return false;
}

bool RegionArrayImpl::TestSubset(const cv::Ptr<Region> &otherRgn) const
{
    for (const cv::Ptr<Region> &rgn : rgns_)
    {
        if (rgn->TestSubset(otherRgn))
        {
            return true;
        }
    }

    return false;
}

cv::Ptr<Region> RegionArrayImpl::Move(const cv::Point &delta) const
{
    std::vector<cv::Ptr<Region>> rgns(rgns_.size());
    tbb::parallel_for(0, (int)rgns_.size(), 1, [&](const int i) { rgns[i] = rgns_[i]->Move(delta); });
    return makePtr<RegionArrayImpl>(&rgns);
}

cv::Ptr<Region> RegionArrayImpl::Zoom(const cv::Size2f &scale) const
{
    std::vector<cv::Ptr<Region>> rgns(rgns_.size());
    tbb::parallel_for(0, (int)rgns_.size(), 1, [&](const int i) { rgns[i] = rgns_[i]->Zoom(scale); });
    return makePtr<RegionArrayImpl>(&rgns);
}

cv::Ptr<Region> RegionArrayImpl::Shrink(const cv::Size2f &ratio) const
{
    std::vector<cv::Ptr<Region>> rgns(rgns_.size());
    tbb::parallel_for(0, (int)rgns_.size(), 1, [&](const int i) { rgns[i] = rgns_[i]->Shrink(ratio); });
    return makePtr<RegionArrayImpl>(&rgns);
}

cv::Ptr<Region> RegionArrayImpl::AffineTrans(const cv::Matx33d &homoMat2D) const
{
    std::vector<cv::Ptr<Region>> rgns(rgns_.size());
    tbb::parallel_for(0, (int)rgns_.size(), 1, [&](const int i) { rgns[i] = rgns_[i]->AffineTrans(homoMat2D); });
    return makePtr<RegionArrayImpl>(&rgns);
}

cv::Ptr<Region> RegionArrayImpl::Connect() const
{
    std::vector<cv::Ptr<Region>> rgns(rgns_.size());
    tbb::parallel_for(0, (int)rgns_.size(), 1, [&](const int i) { rgns[i] = rgns_[i]->Connect(); });
    return makePtr<RegionArrayImpl>(&rgns);
}

cv::Ptr<Region> RegionArrayImpl::Dilation(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts) const
{
    return nullptr;
}

cv::Ptr<Region> RegionArrayImpl::Erosion(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts) const
{
    return nullptr;
}

cv::Ptr<Region> RegionArrayImpl::Opening(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts) const
{
    return nullptr;
}

cv::Ptr<Region> RegionArrayImpl::Closing(const cv::Ptr<Region> &structElement, const cv::Ptr<Dict> &opts) const
{
    return nullptr;
}

bool RegionArrayImpl::IsNeedDoParallel() const
{
    const int nThreads = static_cast<int>(tbb::global_control::active_value(tbb::global_control::max_allowed_parallelism));
    const int numRgns = static_cast<int>(rgns_.size());
    return nThreads > 1 && (numRgns / nThreads) >= 2;
}

cv::String RegionArrayImpl::GetErrorStatus() const
{
    return err_msg_;
}

int RegionArrayImpl::Save(const cv::String &fileName, const cv::Ptr<Dict> &opts) const
{
    return WriteToFile<RegionArrayImpl>(*this, "regions", fileName, opts, err_msg_);
}

int RegionArrayImpl::Load(const cv::String &fileName, const cv::Ptr<Dict> &opts)
{
    return LoadFromFile<RegionArrayImpl>(*this, "regions", fileName, opts, err_msg_);
}

int RegionArrayImpl::Serialize(const cv::String &name, H5Group *g) const
{
    err_msg_.resize(0);
    std::ostringstream bytes;
    H5GroupImpl *group = dynamic_cast<H5GroupImpl*>(g);
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
            oa << boost::serialization::make_nvp("regions", *this);
        }

        H5::DataSet dataSet;
        int r = group->SetDataSet(name, bytes.str(), dataSet);
        if (MLR_SUCCESS != r)
        {
            err_msg_ = "save database failed";
            return r;
        }

        group->SetAttribute(dataSet, cv::String("TypeGUID"), RegionArrayImpl::TypeGUID());
        group->SetAttribute(dataSet, cv::String("Version"), 0);
    }
    catch (const std::exception &e)
    {
        err_msg_ = e.what();
        return MLR_IO_STREAM_EXCEPTION;
    }

    return MLR_SUCCESS;
}

}
}