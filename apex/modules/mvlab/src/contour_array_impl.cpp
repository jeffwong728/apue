#include "precomp.hpp"
#include "contour_array_impl.hpp"
#include "utility.hpp"
#include <opencv2/mvlab.hpp>

namespace cv {
namespace mvlab {

int ContourArrayImpl::Draw(Mat &img, const Scalar& color, const float thickness, const int style) const
{
    if (img.empty())
    {
        std::vector<cv::Rect> boundingBoxes;
        ContourArrayImpl::GetBoundingBox(boundingBoxes);

        Rect bbox;
        for (const cv::Rect &box : boundingBoxes)
        {
            bbox |= box;
        }

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

    for (const cv::Ptr<Contour> &c : contours_)
    {
        int res = c->Draw(img, color, thickness, style);
        if (MLR_SUCCESS != res)
        {
            return res;
        }
    }

    return MLR_SUCCESS;
}

int ContourArrayImpl::Draw(InputOutputArray img, const Scalar& color, const float thickness, const int style) const
{
    Mat imgMat = img.getMat();
    if (imgMat.empty())
    {
        int res = ContourArrayImpl::Draw(imgMat, color, thickness, style);
        img.assign(imgMat);
        return res;
    }
    else
    {
        int dph = img.depth();
        int cnl = img.channels();
        if (CV_8U == dph && (1 == cnl || 3 == cnl || 4 == cnl))
        {
            int rest = ContourArrayImpl::Draw(imgMat, color, thickness, style);
            img.assign(imgMat);
            return rest;
        }
        else
        {
            return MLR_IMAGE_FORMAT_ERROR;
        }
    }
}

bool ContourArrayImpl::Empty() const
{
    return contours_.empty() || std::all_of(contours_.cbegin(), contours_.cend(), [](const cv::Ptr<Contour> &item) { return item->Empty(); });
}

int ContourArrayImpl::Count() const
{
    return static_cast<int>(contours_.size());
}

void ContourArrayImpl::CountPoints(std::vector<int> &cPoints) const
{
    cPoints.reserve(contours_.size());
    for (const cv::Ptr<Contour> &c : contours_)
    {
        cPoints.push_back(c->CountPoints());
    }
}

void ContourArrayImpl::GetLength(std::vector<double> &lengthes) const
{
    lengthes.resize(contours_.size());
    tbb::parallel_for(0, (int)contours_.size(), 1, [&](const int i) { lengthes[i] = contours_[i]->GetLength(); });
}

void ContourArrayImpl::GetArea(std::vector<double> &areas) const
{
    areas.resize(contours_.size());
    tbb::parallel_for(0, (int)contours_.size(), 1, [&](const int i) { areas[i] = contours_[i]->GetArea(); });
}

void ContourArrayImpl::GetCentroid(std::vector<cv::Point2f> &centroids) const
{
    centroids.resize(contours_.size());
    tbb::parallel_for(0, (int)contours_.size(), 1, [&](const int i) { centroids[i] = contours_[i]->GetCentroid(); });
}

void ContourArrayImpl::GetBoundingBox(std::vector<cv::Rect> &boundingBoxes) const
{
    boundingBoxes.resize(contours_.size());
    tbb::parallel_for(0, (int)contours_.size(), 1, [&](const int i) { boundingBoxes[i] = contours_[i]->GetBoundingBox(); });
}

Ptr<Contour> ContourArrayImpl::Simplify(const float tolerance) const
{
    cv::Ptr<ContourArrayImpl> contours = makePtr<ContourArrayImpl>();
    contours->contours_.resize(contours_.size());
    tbb::parallel_for(0, (int)contours_.size(), 1, [&](const int i) { contours->contours_[i] = contours_[i]->Simplify(tolerance); });

    return contours;
}

int ContourArrayImpl::GetPoints(std::vector<Point2f> &points) const
{
    if (contours_.empty())
    {
        return MLR_CONTOUR_EMPTY;
    }
    else
    {
        contours_.front()->GetPoints(points);
        return MLR_SUCCESS;
    }
}

cv::Ptr<Contour> ContourArrayImpl::Move(const cv::Point2f &delta) const
{
    cv::Ptr<ContourArrayImpl> contours = makePtr<ContourArrayImpl>();
    contours->contours_.reserve(contours_.size());
    for (const cv::Ptr<Contour> &c : contours_)
    {
        contours->contours_.push_back(c->Move(delta));
    }

    return contours;
}

cv::Ptr<Contour> ContourArrayImpl::Zoom(const cv::Size2f &scale) const
{
    cv::Ptr<ContourArrayImpl> contours = makePtr<ContourArrayImpl>();
    contours->contours_.reserve(contours_.size());

    for (const cv::Ptr<Contour> &c : contours_)
    {
        contours->contours_.push_back(c->Zoom(scale));
    }

    return contours;
}

void ContourArrayImpl::TestClosed(std::vector<int> &isClosed) const
{
    isClosed.reserve(contours_.size());
    for (const cv::Ptr<Contour> &c : contours_)
    {
        isClosed.push_back(c->TestClosed());
    }
}

void ContourArrayImpl::TestPoint(const cv::Point2f &point, std::vector<int> &isInside) const
{
    isInside.resize(contours_.size());
    tbb::parallel_for(0, (int)contours_.size(), 1, [&](const int i) { isInside[i] = contours_[i]->TestPoint(point); });
}

void ContourArrayImpl::TestSelfIntersection(const cv::String &closeContour, std::vector<int> &doesIntersect) const
{
    doesIntersect.resize(contours_.size());
    tbb::parallel_for(0, (int)contours_.size(), 1, [&](const int i) { doesIntersect[i] = contours_[i]->TestSelfIntersection(closeContour); });
}

int ContourArrayImpl::CountPoints() const
{
    if (contours_.empty())
    {
        return 0;
    }
    else
    {
        return contours_.front()->CountPoints();
    }
}

double ContourArrayImpl::GetArea() const
{
    if (contours_.empty())
    {
        return 0.;
    }
    else
    {
        return contours_.front()->GetArea();
    }
}

double ContourArrayImpl::GetLength() const
{
    if (contours_.empty())
    {
        return 0.;
    }
    else
    {
        return contours_.front()->GetLength();
    }
}
cv::Point2d ContourArrayImpl::GetCentroid() const
{
    if (contours_.empty())
    {
        return cv::Point2d();
    }
    else
    {
        return contours_.front()->GetCentroid();
    }
}
cv::Rect ContourArrayImpl::GetBoundingBox() const
{
    if (contours_.empty())
    {
        return cv::Rect();
    }
    else
    {
        return contours_.front()->GetBoundingBox();
    }
}
bool ContourArrayImpl::TestClosed() const
{
    if (contours_.empty())
    {
        return false;
    }
    else
    {
        return contours_.front()->TestClosed();
    }
}

bool ContourArrayImpl::TestPoint(const cv::Point2f &point) const
{
    if (contours_.empty())
    {
        return false;
    }
    else
    {
        return contours_.front()->TestPoint(point);
    }
}

bool ContourArrayImpl::TestSelfIntersection(const cv::String &closeContour) const
{
    if (contours_.empty())
    {
        return false;
    }
    else
    {
        return contours_.front()->TestSelfIntersection(closeContour);
    }
}

}
}
