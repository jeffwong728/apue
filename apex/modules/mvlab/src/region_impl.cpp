#include "precomp.hpp"
#include "region_impl.hpp"
#include "utility.hpp"
#include <opencv2/mvlab.hpp>

namespace cv {
namespace mvlab {

struct RDEntry
{
    RDEntry(const int x, const int y, const int code, const int qi)
        : X(x), Y(y), CODE(code), LINK(0), W_LINK(0), QI(qi), FLAG(0) {}
    int X;
    int Y;
    int CODE;
    int LINK;
    int W_LINK;
    int QI;
    int FLAG;
};

const int qis_g[11]{ 0, 2, 1, 1, 1, 0, 1, 1, 1, 2, 0 };
const int count_g[11]{ 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1 };
const int downLink_g[11][11]{ {0}, {0}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1}, {0}, {0}, {0}, {0}, {0, 1, 1, 1, 1, 0, 0, 0, 0, 1} };
const int upLink_g[11][11]{ {0}, {0}, {0}, {0}, {0}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1}, {0}, {0, 1, 0, 0, 0, 0, 1, 1, 1, 1} };

using RDList = std::vector<RDEntry, tbb::scalable_allocator<RDEntry>>;
using RDListList = std::vector<RDList, tbb::scalable_allocator<RDList>>;

class RunLengthRDEncoder
{
public:
    RunLengthRDEncoder(const RunList &rgn, const RowRangeList &rranges) : rgn_runs_(rgn), row_ranges_(rranges) {}
    RunLengthRDEncoder(RunLengthRDEncoder& x, tbb::split) : rgn_runs_(x.rgn_runs_), row_ranges_(x.row_ranges_) {}

public:
    void operator()(const tbb::blocked_range<int>& br);
    void join(const RunLengthRDEncoder& y);

private:
    const RunList &rgn_runs_;
    const RowRangeList &row_ranges_;
    RDList rd_list_;
    int P3{ 0 };
    int P4{ 1 };
    int P5{ 0 };
};

void RunLengthRDEncoder::operator()(const tbb::blocked_range<int>& br)
{
    rd_list_.emplace_back(0, 0, 0, 0);
    std::vector<int> P_BUFFER;
    std::vector<int> C_BUFFER;
    constexpr int Infinity = std::numeric_limits<int>::max();
    const int numRuns = static_cast<int>(row_ranges_.size());
    const int lBeg = rgn_runs_[row_ranges_[br.begin()].begRun].row;
    const int lEnd = (br.end() < numRuns) ? rgn_runs_[row_ranges_[br.end()].begRun].row : rgn_runs_[row_ranges_[br.end() - 1].begRun].row + 1;

    if (br.begin() > 0)
    {
        const RowRange &rr = row_ranges_[br.begin() - 1];
        if (rgn_runs_[rr.begRun].row == (lBeg - 1))
        {
            for (int rIdx = rr.begRun; rIdx < rr.endRun; ++rIdx)
            {
                P_BUFFER.push_back(rgn_runs_[rIdx].colb);
                P_BUFFER.push_back(rgn_runs_[rIdx].cole);
            }
        }
    }

    P_BUFFER.push_back(Infinity);

    int rIdx = br.begin();
    for (int l = lBeg; l <= lEnd; ++l)
    {
        if (rIdx < numRuns)
        {
            const RowRange &rr = row_ranges_[rIdx];
            if (l == rgn_runs_[rr.begRun].row)
            {
                for (int runIdx = rr.begRun; runIdx < rr.endRun; ++runIdx)
                {
                    C_BUFFER.push_back(rgn_runs_[runIdx].colb);
                    C_BUFFER.push_back(rgn_runs_[runIdx].cole);
                }
                rIdx += 1;
            }
        }

        C_BUFFER.push_back(Infinity);

        int P1 = 0;
        int P2 = 0;
        int State = 0;
        int X1 = P_BUFFER[P1];
        int X2 = C_BUFFER[P2];
        int X = X2;

        bool stay = true;
        while (stay)
        {
            int RD_CODE = 0;
            switch (State)
            {
            case 0:
                if (X1 > X2) {
                    State = 2; X = X2; P2 += 1; X2 = C_BUFFER[P2];
                }
                else if (X1 < X2) {
                    State = 1; P1 += 1; X1 = P_BUFFER[P1];
                }
                else if (X1 < Infinity) {
                    State = 3; RD_CODE = 2; P1 += 1; X1 = P_BUFFER[P1]; P2 += 1; X2 = C_BUFFER[P2];
                }
                else {
                    stay = false;
                }
                break;

            case 1:
                if (X1 > X2) {
                    State = 3; X = X2; RD_CODE = 4; P2 += 1; X2 = C_BUFFER[P2];
                }
                else if (X1 < X2) {
                    State = 0; X = X1; RD_CODE = 5; P1 += 1; X1 = P_BUFFER[P1];
                }
                else {
                    State = 4; X = X1; RD_CODE = 4; P1 += 1; X1 = P_BUFFER[P1]; P2 += 1; X2 = C_BUFFER[P2];
                }
                break;

            case 2:
                if (X1 > X2) {
                    State = 0; RD_CODE = 1; P2 += 1; X2 = C_BUFFER[P2];
                }
                else if (X1 < X2) {
                    State = 3; RD_CODE = 3; P1 += 1; X1 = P_BUFFER[P1];
                }
                else {
                    State = 5; RD_CODE = 3; P1 += 1; X1 = P_BUFFER[P1]; P2 += 1; X2 = C_BUFFER[P2];
                }
                break;

            case 3:
                if (X1 > X2) {
                    State = 5; P2 += 1; X2 = C_BUFFER[P2];
                }
                else if (X1 < X2) {
                    State = 4; X = X1; P1 += 1; X1 = P_BUFFER[P1];
                }
                else {
                    State = 0; RD_CODE = 6; P1 += 1; X1 = P_BUFFER[P1]; P2 += 1; X2 = C_BUFFER[P2];
                }
                break;

            case 4:
                if (X1 > X2) {
                    State = 0; RD_CODE = 8; P2 += 1; X2 = C_BUFFER[P2];
                }
                else if (X1 < X2) {
                    State = 3; RD_CODE = 10; P1 += 1; X1 = P_BUFFER[P1];
                }
                else {
                    State = 5; RD_CODE = 10; P1 += 1; X1 = P_BUFFER[P1]; P2 += 1; X2 = C_BUFFER[P2];
                }
                break;

            case 5:
                if (X1 > X2) {
                    State = 3; X = X2; RD_CODE = 9; P2 += 1; X2 = C_BUFFER[P2];
                }
                else if (X1 < X2) {
                    State = 0; X = X1; RD_CODE = 7; P1 += 1; X1 = P_BUFFER[P1];
                }
                else {
                    State = 4; X = X1; RD_CODE = 9; P1 += 1; X1 = P_BUFFER[P1]; P2 += 1; X2 = C_BUFFER[P2];
                }
                break;

            default:
                break;
            }

            if (RD_CODE)
            {
                P3 += 1;
                const int QI = qis_g[RD_CODE];
                rd_list_.emplace_back(X, l, RD_CODE, QI);

                if (QI)
                {
                    rd_list_[P5].W_LINK = P3;
                    P5 = P3;
                }
                else
                {
                    rd_list_[P5].W_LINK = P3 + 1;
                }

                if (5 == RD_CODE)
                {
                    if (downLink_g[RD_CODE][rd_list_[P4].CODE])
                    {
                        rd_list_[P4].LINK = P3;
                        rd_list_[P4].QI -= 1;
                        if (1 > rd_list_[P4].QI)
                        {
                            P4 = rd_list_[P4].W_LINK;
                        }
                    }

                    if (upLink_g[RD_CODE][rd_list_[P4].CODE])
                    {
                        rd_list_[P3].LINK = P4;
                        rd_list_[P4].QI -= 1;
                        if (1 > rd_list_[P4].QI)
                        {
                            P4 = rd_list_[P4].W_LINK;
                        }
                    }
                }
                else
                {
                    if (upLink_g[RD_CODE][rd_list_[P4].CODE])
                    {
                        rd_list_[P3].LINK = P4;
                        rd_list_[P4].QI -= 1;
                        if (1 > rd_list_[P4].QI)
                        {
                            P4 = rd_list_[P4].W_LINK;
                        }
                    }

                    if (downLink_g[RD_CODE][rd_list_[P4].CODE])
                    {
                        rd_list_[P4].LINK = P3;
                        rd_list_[P4].QI -= 1;
                        if (1 > rd_list_[P4].QI)
                        {
                            P4 = rd_list_[P4].W_LINK;
                        }
                    }
                }
            }
        }

        P_BUFFER.swap(C_BUFFER);
        C_BUFFER.resize(0);
    }
}

void RunLengthRDEncoder::join(const RunLengthRDEncoder& y)
{
}

RegionImpl::RegionImpl(const cv::Mat &mask)
    : Region()
    , rgn_mask_(mask)
{
}

RegionImpl::RegionImpl(const Rect2f &rect)
    : Region()
{
    if (rect.width > 0.f && rect.height > 0.f)
    {
        rgn_runs_.reserve(cvCeil(rect.height));
        row_ranges_.reserve(rgn_runs_.size());

        int runIdx = 0;
        for (float y = 0.f; y < rect.height; ++y)
        {
            rgn_runs_.emplace_back(cvRound(y), cvRound(rect.x), cvRound(rect.x + rect.width));
            row_ranges_.emplace_back(runIdx, runIdx+1);
            runIdx += 1;
        }

        contour_outers_.emplace(1, makePtr<ContourImpl>(rect));
    }
}

RegionImpl::RegionImpl(const RotatedRect &rotatedRect)
    : Region()
{
    Point2f corners[4];
    rotatedRect.points(corners);

    Geom::PathVector pv;
    Geom::PathBuilder pb(pv);
    pb.moveTo(Geom::Point(corners[0].x, corners[0].y));
    pb.lineTo(Geom::Point(corners[1].x, corners[1].y));
    pb.lineTo(Geom::Point(corners[2].x, corners[2].y));
    pb.lineTo(Geom::Point(corners[3].x, corners[3].y));
    pb.closePath();

    FromPathVector(pv);
    contour_outers_.emplace(1, makePtr<ContourImpl>(pv.front(), true));
}

RegionImpl::RegionImpl(const Point2f &center, const float radius)
    : Region()
{
    Geom::PathVector pv(Geom::Path(Geom::Circle(center.x, center.y, radius)));
    FromPathVector(pv);
    contour_outers_.emplace(1, makePtr<ContourImpl>(pv.front(), true));
}

RegionImpl::RegionImpl(const Point2f &center, const Size2f &size)
    : Region()
{
    Geom::PathVector pv(Geom::Path(Geom::Ellipse(center.x, center.y, size.width, size.height, 0.0)));
    FromPathVector(pv);
    contour_outers_.emplace(1, makePtr<ContourImpl>(pv.front(), true));
}

RegionImpl::RegionImpl(const Point2f &center, const Size2f &size, const float angle)
    : Region()
{
    Geom::PathVector pv(Geom::Path(Geom::Ellipse(center.x, center.y, size.width, size.height, angle)));
    FromPathVector(pv);
    contour_outers_.emplace(1, makePtr<ContourImpl>(pv.front(), true));
}

int RegionImpl::Draw(Mat &img,
    const Scalar& fillColor,
    const Scalar& borderColor,
    const float borderThickness,
    const int borderStyle) const
{
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
        DrawVerified(img, fillColor, borderColor, borderThickness, borderStyle);
    }
    else if (CV_8U == dph && 3 == cnl)
    {
        Mat colorImg;
        cvtColor(img, colorImg, cv::COLOR_BGR2BGRA);
        DrawVerified(colorImg, fillColor, borderColor, borderThickness, borderStyle);
        cvtColor(colorImg, img, cv::COLOR_BGRA2BGR);
    }
    else if (CV_8U == dph && 1 == cnl)
    {
        Mat colorImg;
        cvtColor(img, colorImg, cv::COLOR_GRAY2BGRA);
        DrawVerified(colorImg, fillColor, borderColor, borderThickness, borderStyle);
        cvtColor(colorImg, img, cv::COLOR_BGRA2GRAY);
    }
    else
    {
        return MLR_IMAGE_FORMAT_ERROR;
    }

    return MLR_SUCCESS;
}

int RegionImpl::Draw(InputOutputArray img,
    const Scalar& fillColor,
    const Scalar& borderColor,
    const float borderThickness,
    const int borderStyle) const
{
    Mat imgMat = img.getMat();
    if (imgMat.empty())
    {
        int rest = RegionImpl::Draw(imgMat, fillColor, borderColor, borderThickness, borderStyle);
        img.assign(imgMat);
        return rest;
    }
    else
    {
        int dph = img.depth();
        int cnl = img.channels();
        if (CV_8U == dph && (1 == cnl || 3 == cnl || 4 == cnl))
        {
            int rest = RegionImpl::Draw(imgMat, fillColor, borderColor, borderThickness, borderStyle);
            img.assign(imgMat);
            return rest;
        }
        else
        {
            return MLR_IMAGE_FORMAT_ERROR;
        }
    }
}

double RegionImpl::Area() const
{
    if (area_ == boost::none)
    {
        double a = 0;
        double x = 0;
        double y = 0;

        for (const RunLength &rl : rgn_runs_)
        {
            const auto n = rl.cole - rl.colb;
            a += n;
            x += (rl.cole - 1 + rl.colb) * n / 2.0;
            y += rl.row * n;
        }

        area_ = a;
        if (a > 0)
        {
            centroid_.emplace(x / a, y / a);
        }
        else
        {
            centroid_.emplace(0, 0);
        }
    }

    return *area_;
}

cv::Point2d RegionImpl::Centroid() const
{
    if (centroid_ == boost::none)
    {
        Area();
    }

    return *centroid_;
}

Rect RegionImpl::BoundingBox() const
{
    if (bbox_ == boost::none)
    {
        cv::Point minPoint{ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
        cv::Point maxPoint{ std::numeric_limits<int>::min(), std::numeric_limits<int>::min() };

        for (const RunLength &r : rgn_runs_)
        {
            if (r.row < minPoint.y) {
                minPoint.y = r.row;
            }

            if (r.row > maxPoint.y) {
                maxPoint.y = r.row;
            }

            if (r.colb < minPoint.x) {
                minPoint.x = r.colb;
            }

            if (r.cole > maxPoint.x) {
                maxPoint.x = r.cole;
            }
        }

        if (rgn_runs_.empty()) {
            bbox_ = cv::Rect();
        }
        else {
            bbox_ = cv::Rect(minPoint, maxPoint);
        }
    }

    return *bbox_;
}

void RegionImpl::Connect(std::vector<Ptr<Region>> &regions) const
{
    regions.resize(0);
    regions.push_back(makePtr<RegionImpl>());
    regions.push_back(makePtr<RegionImpl>());
    regions.push_back(makePtr<RegionImpl>());
}

void RegionImpl::ClearCacheData()
{
    area_     = boost::none;
    centroid_ = boost::none;
    bbox_     = boost::none;
}

void RegionImpl::FromMask(const cv::Mat &mask)
{
    int dph = mask.depth();
    int cnl = mask.channels();
    if (CV_8U == dph && 1 == cnl)
    {
        for (int r = 0; r < mask.rows; ++r)
        {
            int cb = -1;
            const uchar* pRow = mask.data + r * mask.step1();
            const int thisRowBegRun = static_cast<int>(rgn_runs_.size());
            int thisRowEndRun = thisRowBegRun;

            for (int c = 0; c < mask.cols; ++c)
            {
                if (pRow[c])
                {
                    if (cb < 0)
                    {
                        cb = c;
                    }
                }
                else
                {
                    if (cb > -1)
                    {
                        rgn_runs_.emplace_back(r, cb, c);
                        thisRowEndRun += 1;
                        cb = -1;
                    }
                }
            }

            if (cb > -1)
            {
                rgn_runs_.emplace_back(r, cb, mask.cols);
                thisRowEndRun += 1;
            }

            if (thisRowBegRun != thisRowEndRun)
            {
                row_ranges_.emplace_back(thisRowBegRun, thisRowEndRun);
            }
        }
    }

    ClearCacheData();
}

void RegionImpl::FromPathVector(const Geom::PathVector &pv)
{
    std::vector<uint8_t> buf;
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

        for (RunLength &run : rgn_runs_)
        {
            run.colb += rect.x;
            run.cole += rect.x;
            run.row += rect.y;
        }
    }
}

void RegionImpl::DrawVerified(Mat &img, const Scalar& fillColor, const Scalar& borderColor, const float borderThickness, const int borderStyle) const
{
    auto imgSurf = Cairo::ImageSurface::create(img.data, Cairo::Format::FORMAT_RGB24, img.cols, img.rows, static_cast<int>(img.step1()));
    auto cr = Cairo::Context::create(imgSurf);

    std::vector<double> dashes = Util::GetDashesPattern(borderStyle, borderThickness);
    if (!dashes.empty())
    {
        cr->set_dash(dashes, 0.);
    }
    cr->translate(0.5, 0.5);

    for (const auto &contour : *contour_outers_)
    {
        Ptr<ContourImpl> spContour = contour.dynamicCast<ContourImpl>();
        if (spContour)
        {
            Geom::CairoPathSink cairoPathSink(cr->cobj());
            cairoPathSink.feed(spContour->GetPath());
            cr->set_source_rgba(fillColor[0] / 255.0, fillColor[1] / 255.0, fillColor[2] / 255.0, fillColor[3] / 255.0);
            cr->fill_preserve();
            cr->set_line_width(borderThickness);
            cr->set_source_rgba(borderColor[0] / 255.0, borderColor[1] / 255.0, borderColor[2] / 255.0, borderColor[3] / 255.0);
            cr->stroke();
        }
    }
}

void RegionImpl::TraceContour()
{
    if (rgn_runs_.empty())
    {
        TraceContourMask();
    }
    else
    {
        TraceContourRunlength();
    }
}

void RegionImpl::TraceContourRunlength()
{

}

void RegionImpl::TraceContourMask()
{

}

}
}
