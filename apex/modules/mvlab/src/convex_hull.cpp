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
    if (0==cPoints) return ScalablePoint2fSequence();
    if (1 == cPoints) return ScalablePoint2fSequence(1, *points);

    const bool clockwise = false;
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

    ScalablePoint2fSequence hull(nout);
    for (int i = 0; i < nout; i++)
    {
        hull[i] = points[hullbuf[i]];
    }

    return hull;
}

}
}
