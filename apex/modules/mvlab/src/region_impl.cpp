#include "precomp.hpp"
#include "region_impl.hpp"
#include "region_collection_impl.hpp"
#include "rtd_encoder.hpp"
#include "utility.hpp"
#include <opencv2/mvlab.hpp>

namespace cv {
namespace mvlab {

RegionImpl::RegionImpl(const Rect2f &rect)
    : RegionImpl()
{
    if (rect.width > 0.f && rect.height > 0.f)
    {
        rgn_runs_.reserve(cvCeil(rect.height));

        int runIdx = 0;
        for (float y = 0.f; y < rect.height; ++y)
        {
            rgn_runs_.emplace_back(cvRound(y), cvRound(rect.x), cvRound(rect.x + rect.width));
            runIdx += 1;
        }

        contour_outers_.emplace_back(makePtr<ContourImpl>(rect));
    }
}

RegionImpl::RegionImpl(const RotatedRect &rotatedRect)
    : RegionImpl()
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
    contour_outers_.emplace_back(makePtr<ContourImpl>(pv.front(), true));
}

RegionImpl::RegionImpl(const Point2f &center, const float radius)
    : RegionImpl()
{
    Geom::PathVector pv(Geom::Path(Geom::Circle(center.x, center.y, radius)));
    FromPathVector(pv);
    contour_outers_.emplace_back(makePtr<ContourImpl>(pv.front(), true));
}

RegionImpl::RegionImpl(const Point2f &center, const Size2f &size)
    : RegionImpl()
{
    Geom::PathVector pv(Geom::Path(Geom::Ellipse(center.x, center.y, size.width, size.height, 0.0)));
    FromPathVector(pv);
    contour_outers_.emplace_back(makePtr<ContourImpl>(pv.front(), true));
}

RegionImpl::RegionImpl(const Point2f &center, const Size2f &size, const float angle)
    : RegionImpl()
{
    Geom::PathVector pv(Geom::Path(Geom::Ellipse(center.x, center.y, size.width, size.height, angle)));
    FromPathVector(pv);
    contour_outers_.emplace_back(makePtr<ContourImpl>(pv.front(), true));
}

RegionImpl::RegionImpl(RunList &&runs)
    : rgn_runs_(runs)
{
}

int RegionImpl::Draw(Mat &img,
    const Scalar& fillColor,
    const Scalar& borderColor,
    const float borderThickness,
    const int borderStyle) const
{
    if (Empty())
    {
        return MLR_REGION_EMPTY;
    }

    TraceContour();

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
    if (Empty())
    {
        return MLR_REGION_EMPTY;
    }

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

bool RegionImpl::Empty() const
{
    return rgn_runs_.empty();
}

double RegionImpl::Area() const
{
    GatherBasicFeatures();
    return *area_;
}

cv::Point2d RegionImpl::Centroid() const
{
    GatherBasicFeatures();
    return *centroid_;
}

Rect RegionImpl::BoundingBox() const
{
    GatherBasicFeatures();
    return *bbox_;
}

void RegionImpl::Connect(cv::Ptr<RegionCollection> &regions, const int connectivity) const
{
    regions = makePtr<RegionCollectionImpl>();
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
                        cb = -1;
                    }
                }
            }

            if (cb > -1)
            {
                rgn_runs_.emplace_back(r, cb, mask.cols);
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

    for (const auto &contour : contour_outers_)
    {
        Ptr<ContourImpl> spContour = contour.dynamicCast<ContourImpl>();
        if (spContour)
        {
            spContour->Feed(cr);
        }
    }

    for (const auto &contour : contour_holes_)
    {
        Ptr<ContourImpl> spContour = contour.dynamicCast<ContourImpl>();
        if (spContour)
        {
            spContour->Feed(cr);
        }
    }

    cr->set_source_rgba(fillColor[0] / 255.0, fillColor[1] / 255.0, fillColor[2] / 255.0, fillColor[3] / 255.0);
    cr->fill_preserve();
    cr->set_line_width(borderThickness);
    cr->set_source_rgba(borderColor[0] / 255.0, borderColor[1] / 255.0, borderColor[2] / 255.0, borderColor[3] / 255.0);
    cr->stroke();
}

void RegionImpl::TraceContour() const
{
    if (!rgn_runs_.empty() && contour_outers_.empty())
    {
        GatherBasicFeatures();
        RunLengthRDEncoder rdEncoder;
        rdEncoder.Encode(rgn_runs_, row_run_begs_);
        rdEncoder.Link();
        rdEncoder.Track(contour_outers_, contour_holes_);
    }
}

void RegionImpl::GatherBasicFeatures() const
{
    if (area_ == boost::none)
    {
        if (!rgn_runs_.empty())
        {
            row_run_begs_.reserve(rgn_runs_.size() + 1);
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
                    row_run_begs_.emplace_back(begIdx); begIdx = runIdx; currentRow = rl.row;
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

            row_run_begs_.emplace_back(begIdx);
            row_run_begs_.emplace_back(numRuns);

            area_ = a;
            if (a > 0) {
                centroid_.emplace(x / a, y / a);
            } else {
                centroid_.emplace(0, 0);
            }

            bbox_ = cv::Rect(minPoint, maxPoint);
        }
        else
        {
            area_.emplace(0);
            bbox_.emplace(0, 0, 0, 0);
            centroid_.emplace(0, 0);
        }
    }
}

}
}
