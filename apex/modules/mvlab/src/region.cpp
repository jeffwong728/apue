#include "precomp.hpp"
#include "utility.hpp"
#include "region_impl.hpp"

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
    bool operator ()(const PolyEdge& e1, const PolyEdge& e2)
    {
        return e1.y0 - e2.y0 ? e1.y0 < e2.y0 :
            e1.x - e2.x ? e1.x < e2.x : e1.dx < e2.dx;
    }
};

using UScalablePolyEdgeSequence = ao::uvector<PolyEdge, MyAlloc<PolyEdge>>;

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
        pResRun->colb = cvFloor(curx1);
        pResRun->cole = cvCeil(curx2) + 1;
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
        pResRun->colb = cvFloor(curx1);
        pResRun->cole = cvCeil(curx2) + 1;
        pResRun->label = 0;
        curx1 -= invslope1;
        curx2 -= invslope2;
    }
}

static cv::Point2f getIntersection3(const cv::Point2f &p0, const cv::Point2f &p1, const cv::Point2f &p2)
{
    const cv::Point2f v1 = p1 - p0;
    const cv::Point2f v2 = p2 - p0;
    const float b1 = p1.x*v1.x + p1.y*v1.y;
    const float b2 = p2.x*v2.x + p2.y*v2.y;
    const float A = v1.x*v2.y - v1.y*v2.x;

    if (std::abs(A)>std::numeric_limits<float>::epsilon())
    {
        const float x = b1 * v2.y - b2 * v1.y;
        const float y = b2 * v1.x - b1 * v2.x;
        return cv::Point2f(x/A, y/A);
    }
    else
    {
        return (p1+p2)/2;
    }
}

static RunSequence getConvexPoly(const cv::Point2f *vf, int npts)
{
    UScalablePointSequence v(npts);
    const UScalablePointSequence::pointer pvEnd = v.data() + npts;
    for (UScalablePointSequence::pointer pv = v.data(); pv != pvEnd; ++pv, ++vf)
    {
        pv->x = cvRound(vf->x * F_XY_ONE);
        pv->y = cvRound(vf->y * F_XY_ONE);
    }

    struct
    {
        int idx, di;
        int x, dx;
        int ye;
    } edge[2];

    int y, imin = 0;
    int edges = npts;
    int xmin, xmax, ymin, ymax;

    xmin = xmax = v[0].x;
    ymin = ymax = v[0].y;

    for (int i = 0; i < npts; i++)
    {
        const cv::Point &p = v[i];
        if (p.y < ymin)
        {
            ymin = p.y;
            imin = i;
        }

        ymax = std::max(ymax, p.y);
        xmax = std::max(xmax, p.x);
        xmin = std::min(xmin, p.x);
    }

    xmin = (xmin + XY_DELTA) >> XY_SHIFT;
    xmax = (xmax + XY_DELTA) >> XY_SHIFT;
    ymin = (ymin + XY_DELTA) >> XY_SHIFT;
    ymax = (ymax + XY_DELTA) >> XY_SHIFT;

    if (ymin > ymax)
    {
        return RunSequence();
    }

    RunSequence dstRuns(ymax - ymin + 1);
    RunSequence::pointer pResRun = dstRuns.data();

    edge[0].idx = edge[1].idx = imin;
    edge[0].ye = edge[1].ye = y = ymin;
    edge[0].di = 1;
    edge[1].di = npts - 1;
    edge[0].x = edge[1].x = -XY_ONE;
    edge[0].dx = edge[1].dx = 0;

    do
    {
        if (y < ymax || y == ymin)
        {
            for (int i = 0; i < 2; i++)
            {
                if (y >= edge[i].ye)
                {
                    int idx0 = edge[i].idx, di = edge[i].di;
                    int idx = idx0 + di;
                    if (idx >= npts) idx -= npts;
                    int ty = 0;

                    for (; edges-- > 0; )
                    {
                        ty = (v[idx].y + XY_DELTA) >> XY_SHIFT;
                        if (ty > y)
                        {
                            int xs = v[idx0].x;
                            int xe = v[idx].x;

                            edge[i].ye = ty;
                            edge[i].dx = ((xe - xs) * 2 + (ty - y)) / (2 * (ty - y));
                            edge[i].x = xs;
                            edge[i].idx = idx;
                            break;
                        }
                        idx0 = idx;
                        idx += di;
                        if (idx >= npts) idx -= npts;
                    }
                }
            }
        }

        if (edges < 0)
            break;

        int left = 0, right = 1;
        if (edge[0].x > edge[1].x)
        {
            left = 1, right = 0;
        }

        pResRun->row  = y;
        pResRun->colb = (edge[left].x + XY_DELTA) >> XY_SHIFT;
        pResRun->cole  = ((edge[right].x + XY_DELTA) >> XY_SHIFT) + 1;
        pResRun->label = 0;
        pResRun += 1;

        edge[0].x += edge[0].dx;
        edge[1].x += edge[1].dx;
    } while (++y <= (int)ymax);

    dstRuns.resize(std::distance(dstRuns.data(), pResRun));
    return dstRuns;
}

static void collectPolyEdges_(const cv::Point* v, const int count, UScalablePolyEdgeSequence &edges)
{
    cv::Point pt0 = v[count - 1], pt1;
    pt0.y = (pt0.y + XY_DELTA) >> XY_SHIFT;

    UScalablePolyEdgeSequence::pointer pEdge = edges.data();
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

    edges.resize(std::distance(edges.data(), pEdge));
}

static RunSequence getPoly_(const cv::Point2f *vf, int npts)
{
    UScalablePointSequence v(npts);
    const UScalablePointSequence::pointer pvEnd = v.data() + npts;
    for (UScalablePointSequence::pointer pv = v.data(); pv != pvEnd; ++pv, ++vf)
    {
        pv->x = cvRound(vf->x * F_XY_ONE);
        pv->y = cvRound(vf->y * F_XY_ONE);
    }

    UScalablePolyEdgeSequence edges(npts+1);
    collectPolyEdges_(v.data(), npts, edges);

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

                if (y== dstRuns.back().row && x1 <= dstRuns.back().cole)
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
    if (rotatedRect.size.width > 0.5f && rotatedRect.size.height > 0.5f)
    {
        Point2f corners[4];
        rotatedRect.points(corners);
        RunSequence dstRuns = getConvexPoly(&corners[0], 4);
        return makePtr<RegionImpl>(&dstRuns);
    }
    else
    {
        return makePtr<RegionImpl>();
    }
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
    const float angMid = angBeg + angExt / 2;

    const float r = radius + 5;
    const cv::Point2f ptBeg{ center.x + r * std::cos(Util::rad(angBeg)), center.y - r * std::sin(Util::rad(angBeg)) };
    const cv::Point2f ptEnd{ center.x + r * std::cos(Util::rad(angEnd)), center.y - r * std::sin(Util::rad(angEnd)) };
    const cv::Point2f ptMid{ center.x + r * std::cos(Util::rad(angMid)), center.y - r * std::sin(Util::rad(angMid)) };

    const cv::Point2f p1 = getIntersection3(center, ptBeg, ptMid);
    const cv::Point2f p2 = getIntersection3(center, ptEnd, ptMid);

    cv::Ptr<Region> rgn1 = Region::GenTriangle(ptBeg, p1, center);
    cv::Ptr<Region> rgn2 = Region::GenTriangle(p1, p2, center);
    cv::Ptr<Region> rgn3 = Region::GenTriangle(ptEnd, p2, center);

    return Region::GenCircle(center, radius)->Intersection(rgn1->Union2(rgn2)->Union2(rgn3));
}

cv::Ptr<Region> Region::GenEllipse(const cv::Point2f &center, const cv::Size2f &size)
{
    if (size.width < 0.5f || size.height < 0.5f)
    {
        return makePtr<RegionImpl>();
    }

    const float a2 = size.width * size.width;
    const float b2 = size.height * size.height;
    const float k = a2 / b2;
    const float ayMin = center.y - size.height;
    const float ayMax = center.y + size.height;
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

    for (int y = yMin; y <= yMax; ++y)
    {
        const float dy = static_cast<float>(y) - center.y;
        const float dx = cv::sqrt(a2 - k * dy * dy);

        pResRun->row = y;
        pResRun->colb = cvRound(center.x - dx);
        pResRun->cole = cvRound(center.x + dx) + 1;
        pResRun->label = 0;

        pResRun += 1;
    }

    return makePtr<RegionImpl>(&dstRuns);
}

cv::Ptr<Region> Region::GenRotatedEllipse(const cv::Point2f &center, const cv::Size2f &size, const float phi)
{
    if (size.width < 0.5f || size.height < 0.5f)
    {
        return makePtr<RegionImpl>();
    }

    float const u = std::cos(Util::rad(-phi));
    float const v = std::sin(Util::rad(-phi));
    float const rmax = std::max(size.width, size.height);
    float const l1 = rmax * rmax / (size.width*size.width);
    float const l2 = rmax * rmax / (size.height*size.height);
    float const a = l1 * u*u + l2 * v*v;
    float const b = u * v*(l1 - l2);
    float const c = l1 * v*v + l2 * u*u;
    float const yb = std::sqrt(a*rmax*rmax / (a*c - b * b));
    int const ymin = cvFloor(center.y - yb);
    int const ymax = cvCeil(center.y + yb) + 1;

    RunSequence dstRuns(ymax - ymin + 1);
    RunSequence::pointer pResRun = dstRuns.data();

    for (int y = ymin; y < ymax; ++y) {
        float const Y = float(y) - center.y + 0.25f;
        float const delta = b * b * Y * Y - a * (c * Y * Y - rmax * rmax);
        if (delta > 0.f) {
            float const sdelta = std::sqrt(delta);
            pResRun->row = y;
            pResRun->colb = cvRound(center.x - (b*Y + sdelta) / a);
            pResRun->cole = cvRound(center.x - (b*Y - sdelta) / a) + 1;
            pResRun->label = 0;

            pResRun += 1;
        }
    }

    dstRuns.resize(std::distance(dstRuns.data(), pResRun));
    return makePtr<RegionImpl>(&dstRuns);
}

cv::Ptr<Region> Region::GenEllipseSector(const cv::Point2f &center, const cv::Size2f &size, const float phi, const float startAngle, const float endAngle)
{
    if (size.width < 0.5f || size.height < 0.5f)
    {
        return makePtr<RegionImpl>();
    }

    const float angBeg = Util::constrainAngle(startAngle);
    const float angEnd = Util::constrainAngle(endAngle);
    const float angExt = (angBeg < angEnd) ? (angEnd - angBeg) : (360.f - angBeg + angEnd);
    const float angMid = angBeg + angExt / 2;

    const float radius = std::max(size.width, size.height);
    const float r = radius + 5;
    const cv::Point2f ptBeg{ center.x + r * std::cos(Util::rad(angBeg)), center.y - r * std::sin(Util::rad(angBeg)) };
    const cv::Point2f ptEnd{ center.x + r * std::cos(Util::rad(angEnd)), center.y - r * std::sin(Util::rad(angEnd)) };
    const cv::Point2f ptMid{ center.x + r * std::cos(Util::rad(angMid)), center.y - r * std::sin(Util::rad(angMid)) };

    const cv::Point2f p1 = getIntersection3(center, ptBeg, ptMid);
    const cv::Point2f p2 = getIntersection3(center, ptEnd, ptMid);

    cv::Ptr<Region> rgn1 = Region::GenTriangle(ptBeg, p1, center);
    cv::Ptr<Region> rgn2 = Region::GenTriangle(p1, p2, center);
    cv::Ptr<Region> rgn3 = Region::GenTriangle(ptEnd, p2, center);

    return Region::GenRotatedEllipse(center, size, phi)->Intersection(rgn1->Union2(rgn2)->Union2(rgn3));
}

cv::Ptr<Region> Region::GenPolygon(const std::vector<cv::Point2f> &vertexes)
{
    if (vertexes.size()>2)
    {
        RunSequence dstRuns = getPoly_(vertexes.data(), static_cast<int>(vertexes.size()));
        return makePtr<RegionImpl>(&dstRuns);
    }
    else
    {
        return makePtr<RegionImpl>();
    }
}

}
}
