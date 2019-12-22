#include "precomp.hpp"
#include "convex_hull.hpp"
#include "utility.hpp"

namespace cv {
namespace mvlab {

template<typename _Tp>
struct CHullCmpPoints
{
    bool operator()(const cv::Point_<_Tp>* p1, const cv::Point_<_Tp>* p2) const
    {
        return p1->x < p2->x || (p1->x == p2->x && p1->y < p2->y);
    }

    bool operator()(const cv::Point_<_Tp> &p1, const cv::Point_<_Tp> &p2) const
    {
        return p1.x < p2.x || (p1.x == p2.x && p1.y < p2.y);
    }
};

template<typename _Tp>
static int SklanskyImpl(const cv::Point_<_Tp>** array, int start, int end, int* stack, int nsign, int sign2)
{
    int incr = end > start ? 1 : -1;
    // prepare first triangle
    int pprev = start, pcur = pprev + incr, pnext = pcur + incr;
    int stacksize = 3;

    if (start == end ||
        (array[start]->x == array[end]->x &&
            array[start]->y == array[end]->y))
    {
        stack[0] = start;
        return 1;
    }

    stack[0] = pprev;
    stack[1] = pcur;
    stack[2] = pnext;

    end += incr; // make end = afterend

    while (pnext != end)
    {
        // check the angle p1,p2,p3
        _Tp cury = array[pcur]->y;
        _Tp nexty = array[pnext]->y;
        _Tp by = nexty - cury;

        if (CV_SIGN(by) != nsign)
        {
            _Tp ax = array[pcur]->x - array[pprev]->x;
            _Tp bx = array[pnext]->x - array[pcur]->x;
            _Tp ay = cury - array[pprev]->y;
            _Tp convexity = ay * bx - ax * by; // if >0 then convex angle

            if (CV_SIGN(convexity) == sign2 && (ax != 0 || ay != 0))
            {
                pprev = pcur;
                pcur = pnext;
                pnext += incr;
                stack[stacksize] = pnext;
                stacksize++;
            }
            else
            {
                if (pprev == start)
                {
                    pcur = pnext;
                    stack[1] = pcur;
                    pnext += incr;
                    stack[2] = pnext;
                }
                else
                {
                    stack[stacksize - 2] = pnext;
                    pcur = pprev;
                    pprev = stack[stacksize - 4];
                    stacksize--;
                }
            }
        }
        else
        {
            pnext += incr;
            stack[stacksize - 1] = pnext;
        }
    }

    return --stacksize;
}

ScalablePoint2fSequence ConvexHull::Sklansky(const cv::Point2f *points, const int cPoints)
{
    if (cPoints < 5) {
        ScalablePoint2fSequence hull(cPoints);
        for (int i = 0; i < cPoints; ++i)
        {
            hull[i] = points[i];
        }
        return hull;
    }

    constexpr bool clockwise = false;
    AdaptBuffer<const Point2f*> _pointer(cPoints);
    AdaptBuffer<int> _stack(cPoints + 2), _hullbuf(cPoints);
    const Point2f** pointerf = _pointer.data();
    int* stack = _stack.data();
    int* hullbuf = _hullbuf.data();

    for (int i = 0; i < cPoints; i++)
    {
        pointerf[i] = &points[i];
    }

    int miny_ind = 0, maxy_ind = 0;
    std::sort(pointerf, pointerf + cPoints, CHullCmpPoints<float>());
    for (int i = 1; i < cPoints; i++)
    {
        float y = pointerf[i]->y;
        if (pointerf[miny_ind]->y > y)
            miny_ind = i;
        if (pointerf[maxy_ind]->y < y)
            maxy_ind = i;
    }

    int nout = 0;
    if (pointerf[0]->x == pointerf[cPoints - 1]->x &&
        pointerf[0]->y == pointerf[cPoints - 1]->y)
    {
        hullbuf[nout++] = 0;
    }
    else
    {
        // upper half
        int *tl_stack = stack;
        int tl_count = SklanskyImpl(pointerf, 0, maxy_ind, tl_stack, -1, 1);
        int *tr_stack = stack + tl_count;
        int tr_count = SklanskyImpl(pointerf, cPoints - 1, maxy_ind, tr_stack, -1, -1);

        // gather upper part of convex hull to output
        if (!clockwise)
        {
            std::swap(tl_stack, tr_stack);
            std::swap(tl_count, tr_count);
        }

        for (int i = 0; i < tl_count - 1; i++)
            hullbuf[nout++] = int(pointerf[tl_stack[i]] - points);
        for (int i = tr_count - 1; i > 0; i--)
            hullbuf[nout++] = int(pointerf[tr_stack[i]] - points);
        int stop_idx = tr_count > 2 ? tr_stack[1] : tl_count > 2 ? tl_stack[tl_count - 2] : -1;

        // lower half
        int *bl_stack = stack;
        int bl_count = SklanskyImpl(pointerf, 0, miny_ind, bl_stack, 1, -1);
        int *br_stack = stack + bl_count;
        int br_count = SklanskyImpl(pointerf, cPoints - 1, miny_ind, br_stack, 1, 1);

        if (clockwise)
        {
            std::swap(bl_stack, br_stack);
            std::swap(bl_count, br_count);
        }

        if (stop_idx >= 0)
        {
            int check_idx = bl_count > 2 ? bl_stack[1] :
                bl_count + br_count > 2 ? br_stack[2 - bl_count] : -1;
            if (check_idx == stop_idx || (check_idx >= 0 &&
                pointerf[check_idx]->x == pointerf[stop_idx]->x &&
                pointerf[check_idx]->y == pointerf[stop_idx]->y))
            {
                // if all the points lie on the same line, then
                // the bottom part of the convex hull is the mirrored top part
                // (except the exteme points).
                bl_count = std::min(bl_count, 2);
                br_count = std::min(br_count, 2);
            }
        }

        for (int i = 0; i < bl_count - 1; i++)
            hullbuf[nout++] = int(pointerf[bl_stack[i]] - points);
        for (int i = br_count - 1; i > 0; i--)
            hullbuf[nout++] = int(pointerf[br_stack[i]] - points);
    }

    ScalablePoint2fSequence hull(nout+1);
    for (int i = 0; i < nout; i++)
    {
        hull[i] = points[hullbuf[i]];
    }
    hull.back() = hull.front();

    return hull;
}

ScalablePoint2fSequence ConvexHull::AndrewMonotoneChain(const cv::Point2f *points, const int cPoints)
{
    if (cPoints < 5) {
        ScalablePoint2fSequence hull(cPoints);
        for (int i = 0; i < cPoints; ++i)
        {
            hull[i] = points[i];
        }
        return hull;
    }

    AdaptBuffer<cv::Point2f> pointPtrs(cPoints);
    std::memcpy(pointPtrs.data(), points, cPoints*sizeof(cv::Point2f));
    std::sort(pointPtrs.data(), pointPtrs.data() + cPoints, CHullCmpPoints<float>());

    ScalablePoint2fSequence hull(cPoints+3);
    cv::Point2f *H = hull.data();
    const cv::Point2f *P = pointPtrs.data();

    // the output array H[] will be used as the stack
    int    bot = 0, top = (-1);   // indices for bottom and top of the stack
    int    i = 0;                 // array scan index

    // Get the indices of points with min x-coord and min|max y-coord
    int minmin = 0;
    float xmin = P[0].x;
    for (i = 1; i < cPoints; i++)
        if (P[i].x != xmin) break;
    int minmax = i - 1;
    if (minmax == cPoints - 1) {       // degenerate case: all x-coords == xmin
        H[++top] = P[minmin];
        if (P[minmax].y != P[minmin].y) // a  nontrivial segment
            H[++top] = P[minmax];
        hull.resize(top+1);
        return hull;
    }

    // Get the indices of points with max x-coord and min|max y-coord
    int maxmin, maxmax = cPoints - 1;
    float xmax = P[cPoints - 1].x;
    for (i = cPoints - 2; i >= 0; i--)
        if (P[i].x != xmax) break;
    maxmin = i + 1;

    // Compute the lower hull on the stack H
    H[++top] = P[minmin];      // push  minmin point onto stack
    i = minmax;
    while (++i <= maxmin)
    {
        // the lower line joins P[minmin]  with P[maxmin]
        if (Util::isTolLeft(P[minmin], P[maxmin], P[i]) <= 0 && i < maxmin)
            continue;           // ignore P[i] above or on the lower line

        while (top > 0)         // there are at least 2 points on the stack
        {
            // test if  P[i] is left of the line at the stack top
            if (Util::isTolLeft(H[top - 1], H[top], P[i]) < 0)
                break;         // P[i] is a new hull  vertex
            else
                top--;         // pop top point off  stack
        }
        H[++top] = P[i];        // push P[i] onto stack
    }

    // Next, compute the upper hull on the stack H above  the bottom hull
    if (maxmax != maxmin)      // if  distinct xmax points
        H[++top] = P[maxmax];  // push maxmax point onto stack
    bot = top;                  // the bottom point of the upper hull stack
    i = maxmin;
    while (--i >= minmax)
    {
        // the upper line joins P[maxmax]  with P[minmax]
        if (Util::isTolLeft(P[maxmax], P[minmax], P[i]) <= 0 && i > minmax)
            continue;           // ignore P[i] below or on the upper line

        while (top > bot)     // at least 2 points on the upper stack
        {
            // test if  P[i] is left of the line at the stack top
            if (Util::isTolLeft(H[top - 1], H[top], P[i]) < 0)
                break;         // P[i] is a new hull  vertex
            else
                top--;         // pop top point off  stack
        }
        H[++top] = P[i];        // push P[i] onto stack
    }

    if (minmax != minmin)
        H[++top] = P[minmin];

    hull.resize(top+1);
    return hull;
}

ScalablePoint2fSequence ConvexHull::MelkmanSimpleHull(const cv::Point2f *points, const int cPoints)
{
    if (cPoints < 5) {
        ScalablePoint2fSequence hull(cPoints);
        for (int i = 0; i < cPoints; ++i)
        {
            hull[i] = points[i];
        }
        return hull;
    }

    AdaptBuffer<cv::Point2f> pointPtrs(2*cPoints+2);
    // initialize a deque D[] from bottom to top so that the
    // 1st three vertices of P[] are a ccw triangle
    cv::Point2f *D = pointPtrs.data();
    int bot = cPoints - 2, top = bot + 3;    // initial bottom and top deque indices
    D[bot] = D[top] = points[2];        // 3rd vertex is at both bot and top
    if (Util::isTolLeft(points[0], points[1], points[2]) < 0) {
        D[bot + 1] = points[0];
        D[bot + 2] = points[1];           // ccw vertices are: 2,0,1,2
    }
    else {
        D[bot + 1] = points[1];
        D[bot + 2] = points[0];           // ccw vertices are: 2,1,0,2
    }

    // compute the hull on the deque D[]
    for (int i = 3; i < cPoints; i++) {   // process the rest of vertices
        // test if next vertex is inside the deque hull
        if ((Util::isTolLeft(D[bot], D[bot + 1], points[i]) < 0) &&
            (Util::isTolLeft(D[top - 1], D[top], points[i]) < 0))
            continue;         // skip an interior vertex

   // incrementally add an exterior vertex to the deque hull
   // get the rightmost tangent at the deque bot
        while (Util::isTolLeft(D[bot], D[bot + 1], points[i]) >= 0)
            ++bot;                 // remove bot of deque
        D[--bot] = points[i];           // insert P[i] at bot of deque

        // get the leftmost tangent at the deque top
        while (Util::isTolLeft(D[top - 1], D[top], points[i]) >= 0)
            --top;                 // pop top of deque
        D[++top] = points[i];           // push P[i] onto top of deque
    }

    ScalablePoint2fSequence hull(top - bot + 1);
    for (int h = 0; h <= (top - bot); ++h)
        hull[h] = D[bot + h];

    return hull;
}

}
}
