#include "precomp.hpp"
#include "utility.hpp"
#include "region_impl.hpp"

namespace cv {
namespace mvlab {

static void genBottomFlatTriangle(const cv::Point2f (&v)[3], RunSequence &dstRuns, const int yMin, const int yMax)
{
    const float k1 = (v[1].x - v[0].x) / (v[1].y - v[0].y);
    const float k2 = (v[2].x - v[0].x) / (v[2].y - v[0].y);
    const float invslope1 = std::min(k1, k2);
    const float invslope2 = std::max(k1, k2);

    float curx1 = v[0].x;
    float curx2 = v[0].x;

    RunSequence::pointer pResRun = dstRuns.data();
    for (int y = yMin; y <= yMax; ++y)
    {
        pResRun->row = y;
        pResRun->colb = cvRound(curx1);
        pResRun->cole = cvRound(curx2) + 1;
        pResRun->label = 0;
        pResRun += 1;
        curx1 += invslope1;
        curx2 += invslope2;
    }
}

static void genTopFlatTriangle(const cv::Point2f(&v)[3], RunSequence &dstRuns, const int yMin, const int yMax)
{
    const float k1 = (v[2].x - v[0].x) / (v[2].y - v[0].y);
    const float k2 = (v[2].x - v[1].x) / (v[2].y - v[1].y);
    const float invslope1 = std::max(k1, k2);
    const float invslope2 = std::min(k1, k2);

    float curx1 = v[2].x;
    float curx2 = v[2].x;

    RunSequence::pointer pResRun = dstRuns.data() + dstRuns.size();
    for (int y = yMax; y >= yMin; --y)
    {
        pResRun -= 1;
        pResRun->row = y;
        pResRun->colb = cvRound(curx1);
        pResRun->cole = cvRound(curx2) + 1;
        pResRun->label = 0;
        curx1 -= invslope1;
        curx2 -= invslope2;
    }
}

cv::Ptr<Region> Region::GenEmpty()
{
    return makePtr<RegionImpl>();
}

cv::Ptr<Region> Region::GenChecker(const cv::Size &sizeRegion, const cv::Size &sizePattern)
{
    if (sizeRegion.width < 1 || sizeRegion.height < 1 || sizePattern.width < 1 || sizePattern.height < 1)
    {
        return makePtr<RegionImpl>();
    }

    const int checkerWidth   = sizePattern.width * 2;
    const int upperRegWidth  = ((sizeRegion.width + checkerWidth - 1) / checkerWidth) * checkerWidth;
    RunSequence dstRuns((upperRegWidth/checkerWidth)*sizeRegion.height);
    RunSequence::pointer pResRun = dstRuns.data();

    for (int y = 0; y < sizeRegion.height; ++y)
    {
        for (int x = ((y / sizePattern.height) & 1) ? sizePattern.width : 0; x < sizeRegion.width; x += checkerWidth)
        {
            pResRun->row = y;
            pResRun->colb = x;
            pResRun->cole = std::min(x + sizePattern.width, sizeRegion.width);
            pResRun->label = 0;
            ++pResRun;
        }
    }

    assert(std::distance(dstRuns.data(), pResRun) <= dstRuns.size());
    dstRuns.resize(std::distance(dstRuns.data(), pResRun));

    return makePtr<RegionImpl>(&dstRuns);
}

cv::Ptr<Region> Region::GenTriangle(const cv::Point2f &v1, const cv::Point2f &v2, const cv::Point2f &v3)
{
    cv::Point v12 = v2 - v1;
    cv::Point v13 = v3 - v1;
    const double area = cv_abs(v12.cross(v13))/2;
    if (area < 3)
    {
        return makePtr<RegionImpl>();
    }

    cv::Point2f v[3]{v1, v2, v3};
    std::sort(&v[0], &v[3], [](const cv::Point2f &lv, const cv::Point2f &rv) { return lv.y < rv.y; });

    int yMin = cvRound(v[0].y);
    int yMax = cvRound(v[2].y);

    if (yMin > yMax)
    {
        return makePtr<RegionImpl>();
    }

    RunSequence dstRuns(yMax - yMin + 1);
    if (v[1].y == v[2].y) /* check for trivial case of bottom-flat triangle */
    {
        genBottomFlatTriangle(v, dstRuns, yMin, yMax);
    }
    else if(v[0].y == v[1].y) /* check for trivial case of top-flat triangle */
    {
        genTopFlatTriangle(v, dstRuns, yMin, yMax);
    }
    else
    {
        const cv::Point2f v4{ (v[0].x + ((v[1].y - v[0].y) / (v[2].y - v[0].y)) * (v[2].x - v[0].x)), v[1].y };
        int yMid = cvRound(v[1].y);
        if (static_cast<float>(yMid) > v[1].y)
        {
            yMid -= 1;
        }

        cv::Point2f vTopHalf[3]{ v[0], v[1], v4 };
        genBottomFlatTriangle(vTopHalf, dstRuns, yMin, yMid);

        cv::Point2f vBotHalf[3]{ v[1], v4, v[2] };
        genTopFlatTriangle(vBotHalf, dstRuns, yMid+1, yMax);
    }

    return makePtr<RegionImpl>(&dstRuns);
}

cv::Ptr<Region> Region::GenQuadrangle(const cv::Point2f &v1, const cv::Point2f &v2, const cv::Point2f &v3, const cv::Point2f &v4)
{
    cv::Ptr<Region> rgn1 = Region::GenTriangle(v1, v2, v3);
    cv::Ptr<Region> rgn2 = Region::GenTriangle(v3, v4, v1);
    return rgn1->Union2(rgn2);
}

cv::Ptr<Region> Region::GenRectangle(const cv::Rect2f &rect)
{
    if (rect.height < 1.f || rect.width < 1.f)
    {
        return makePtr<RegionImpl>();
    }

    int yMin = cvRound(rect.y);
    int yMax = cvRound(rect.y + rect.height);
    int xMin = cvRound(rect.x);
    int xMax = cvRound(rect.x + rect.width);

    if (yMin > yMax)
    {
        return makePtr<RegionImpl>();
    }

    RunSequence dstRuns(yMax - yMin);
    RunSequence::pointer pResRun = dstRuns.data();

    for (int y = yMin; y < yMax; ++y)
    {
        pResRun->row = y;
        pResRun->colb = xMin;
        pResRun->cole = xMax;
        pResRun->label = 0;

        pResRun += 1;
    }

    return makePtr<RegionImpl>(&dstRuns);
}

cv::Ptr<Region> Region::GenRotatedRectangle(const cv::RotatedRect &rotatedRect)
{
    return makePtr<RegionImpl>(rotatedRect);
}

cv::Ptr<Region> Region::GenCircle(const cv::Point2f &center, const float radius)
{
    if (radius<0.5f)
    {
        return makePtr<RegionImpl>();
    }

    const float r2 = radius * radius;
    const float ayMin = center.y - radius;
    const float ayMax = center.y + radius;
    int yMin = cvRound(ayMin);
    int yMax = cvRound(ayMax);

    while (static_cast<float>(yMin) < ayMin)
    {
        yMin += 1;
    }

    while (static_cast<float>(yMax) > ayMax)
    {
        yMax -= 1;
    }

    if (yMin > yMax)
    {
        return makePtr<RegionImpl>();
    }

    RunSequence dstRuns(yMax - yMin + 1);
    RunSequence::pointer pResRun = dstRuns.data();

    for (int y=yMin; y<=yMax; ++y)
    {
        const float dy = static_cast<float>(y) - center.y;
        const float dx = cv::sqrt(r2 - dy * dy);

        pResRun->row = y;
        pResRun->colb = cvRound(center.x - dx);
        pResRun->cole = cvRound(center.x + dx) + 1;
        pResRun->label = 0;

        pResRun += 1;
    }

    return makePtr<RegionImpl>(&dstRuns);
}

cv::Ptr<Region> Region::GenCircleSector(const cv::Point2f &center, const float radius, const float startAngle, const float endAngle)
{
    const float angBeg = Util::constrainAngle(startAngle);
    const float angEnd = Util::constrainAngle(endAngle);
    const float angExt = (angBeg < angEnd) ? (angEnd - angBeg) : (360.f - angBeg + angEnd);

    if (angExt < 180.f)
    {

    }
    else
    {

    }

    return makePtr<RegionImpl>(center, radius);
}

cv::Ptr<Region> Region::GenEllipse(const cv::Point2f &center, const cv::Size2f &size)
{
    return makePtr<RegionImpl>(center, size);
}

cv::Ptr<Region> Region::GenRotatedEllipse(const cv::Point2f &center, const cv::Size2f &size, const float angle)
{
    return makePtr<RegionImpl>(center, size, angle);
}

cv::Ptr<Region> Region::GenPolygon(const std::vector<cv::Point2f> &vertexes)
{
    return makePtr<RegionImpl>();
}

}
}
