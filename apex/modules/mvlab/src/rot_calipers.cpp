#include "precomp.hpp"
#include "rot_calipers.hpp"
#include "utility.hpp"

namespace cv {
namespace mvlab {

cv::RotatedRect RotatingCaliper::minAreaRect(const cv::Point2f *points, const int cPoints)
{
    RotatedRect box;

    if (cPoints > 2)
    {
        return minAreaRectBruteForce(points, cPoints);
    }
    else if( cPoints == 2 )
    {
        box.center.x = (points[0].x + points[1].x)*0.5f;
        box.center.y = (points[0].y + points[1].y)*0.5f;
        double dx = points[1].x - points[0].x;
        double dy = points[1].y - points[0].y;
        box.size.width = (float)std::sqrt(dx*dx + dy*dy);
        box.size.height = 0;
        box.angle = (float)atan2( dy, dx );
    }
    else
    {
        if(cPoints == 1 )
            box.center = points[0];
    }

    box.angle = (float)(box.angle*180/CV_PI);
    return box;
}

cv::RotatedRect RotatingCaliper::minAreaRectBruteForce(const cv::Point2f *points, const int cPoints)
{
    const float sArea = Util::isLeft(points[cPoints - 1], points[0], points[1]);
    const bool orientCCW = sArea > 0;
    int minLeft = 0, minRight = 0, minTop = 0, minBot0 = 0, minBot1 = 0;
    float minArea = std::numeric_limits<float>::max();
    if (orientCCW)
    {
        const cv::Point2f sv = points[0] - points[cPoints - 1];
        for (int i0 = cPoints - 1, i1 = 0; i1 < cPoints; i0 = i1++)
        {
            const cv::Point2f u = points[i1] - points[i0];
            const cv::Point2f v{ u.y, -u.x };
            const float sqrLenU = u.dot(u);

            float lMin = std::numeric_limits<float>::max(), rMax = std::numeric_limits<float>::lowest(), tMax = std::numeric_limits<float>::lowest();
            int l = i1, r = i1, t = i1;
            for (int i = 0; i < cPoints; ++i)
            {
                const cv::Point2f diff = points[i] - points[i0];
                const float du = u.dot(diff);
                const float dv = v.dot(diff);

                if (du > rMax) { rMax = du; r = i; }
                if (dv > tMax) { tMax = dv; t = i; }
                if (du < lMin) { lMin = du; l = i; }
            }

            const float area = (rMax - lMin) * tMax / sqrLenU;
            if (area < minArea)
            {
                minArea = area; minLeft = l; minRight = r; minTop = t; minBot0 = i0; minBot1 = i1;
            }

            if (sv.dot(u) < 0.f)
            {
                break;
            }
        }
    }
    else
    {
        const cv::Point2f sv = points[0] - points[cPoints - 1];
        for (int i0 = cPoints - 1, i1 = 0; i1 < cPoints; i0 = i1++)
        {
            const cv::Point2f u = points[i1] - points[i0];
            const cv::Point2f v{ -u.y, u.x };
            const float sqrLenU = u.dot(u);

            float lMax = std::numeric_limits<float>::lowest(), rMin = std::numeric_limits<float>::max(), tMax = std::numeric_limits<float>::lowest();
            int l = i1, r = i1, t = i1;
            for (int i = 0; i < cPoints; ++i)
            {
                const cv::Point2f diff = points[i] - points[i0];
                const float du = u.dot(diff);
                const float dv = v.dot(diff);

                if (du > lMax) { lMax = du; l = i; }
                if (dv > tMax) { tMax = dv; t = i; }
                if (du < rMin) { rMin = du; r = i; }
            }

            const float area = (lMax - rMin) * tMax / sqrLenU;
            if (area < minArea)
            {
                minArea = area; minLeft = l; minRight = r; minTop = t; minBot0 = i0; minBot1 = i1;
            }

            if (sv.dot(u) < 0.f)
            {
                break;
            }
        }
    }

    const cv::Point2f w = points[minBot1] - points[minBot0];
    const cv::Point2f wl = points[minLeft] - points[minBot0];
    const cv::Point2f wr = points[minRight] - points[minBot0];

    const float sqrLenW = w.dot(w);
    const float wtr = w.dot(wr) / sqrLenW;
    const float wtl = w.dot(wl) / sqrLenW;
    if (sqrLenW < 6*std::numeric_limits<float>::epsilon())
    {
        return cv::RotatedRect();
    }

    const cv::Point2f pl = Util::interPoint(wtl, points[minBot0], points[minBot1]);
    const cv::Point2f pr = Util::interPoint(wtr, points[minBot0], points[minBot1]);

    cv::Point2f v = points[minLeft] - pl;
    float sqrLenV = v.dot(v);
    if (sqrLenV < 6 * std::numeric_limits<float>::epsilon())
    {
        if (orientCCW)
        {
            v = cv::Point2f(w.y, -w.x);
        }
        else
        {
            const cv::Point2f w0 = points[minBot0] - points[minBot1];
            v = cv::Point2f(w0.y, -w0.x);
        }
    }

    sqrLenV = v.dot(v);
    if (sqrLenV < 6 * std::numeric_limits<float>::epsilon())
    {
        return cv::RotatedRect();
    }

    const cv::Point2f vt = points[minTop] - pl;

    const float vtt = v.dot(vt) / sqrLenV;
    const cv::Point2f pt = Util::interPoint(vtt, pl, v+pl);

    const cv::Point2f wv = pr - pl;
    const cv::Point2f hv = pt - pl;
    const float width = std::sqrt(wv.dot(wv));
    const float height = std::sqrt(hv.dot(hv));

    if (width < height)
    {
        const float ang = Util::constrainAngle(Util::deg(std::atan2(v.y, v.x)));
        cv::RotatedRect rr{ (pt + pr) / 2, Size2f(height, width),  ang };
        return rr;
    }
    else
    {
        const float ang = Util::constrainAngle(Util::deg(std::atan2(w.y, w.x)));
        cv::RotatedRect rr{ (pt + pr) / 2, Size2f(width, height),  ang };
        return rr;
    }
}

cv::Scalar RotatingCaliper::diameter(const cv::Point2f *points, const int cPoints)
{
    constexpr int simdSize = 8+1;
    if (cPoints < simdSize)
    {
        return diameterBruteForce(points, cPoints);
    }
    else if (cPoints < simdSize * 3)
    {
        return diameterSIMD(points, cPoints);
    }
    else
    {
        return diameterShamos(points, cPoints);
    }
}

cv::Scalar RotatingCaliper::diameterBruteForce(const cv::Point2f *points, const int cPoints)
{
    int m = 0;
    int n = 0;
    float maxSqrDist = 0;
    for (int i = 0; i < cPoints; ++i)
    {
        for (int j = i+1; j < cPoints; ++j)
        {
            cv::Point2f dxy = points[j] - points[i];
            const float sqrDist = dxy.dot(dxy);
            if (sqrDist > maxSqrDist)
            {
                maxSqrDist = sqrDist;
                m = i;
                n = j;
            }
        }
    }

    if (cPoints)
    {
        return cv::Scalar(points[m].x, points[m].y, points[n].x, points[n].y);
    }
    else
    {
        return cv::Scalar();
    }
}

cv::Scalar RotatingCaliper::diameterSIMD(const cv::Point2f *points, const int cPoints)
{
    int m = 0;
    int n = 0;
    float maxSqrDist = 0;
    constexpr int simdSize = 8;
    const vcl::Vec8i vInc(simdSize);
    for (int i = 0; i < cPoints; ++i)
    {
        const int rPoints = cPoints - (i + 1);
        const int regularRPoints = (rPoints & (-simdSize)) + (i + 1);
        const cv::Point2f *pt = points + i + 1;

        vcl::Vec8i vMaxIdx(0);
        vcl::Vec8i vIdx(i + 1, i + 2, i + 3, i + 4, i + 5, i + 6, i + 7, i + 8);
        vcl::Vec8f vMaxSqrDist(0.f);
        const vcl::Vec8f Px(points[i].x);
        const vcl::Vec8f Py(points[i].y);

        int j = i + 1;
        for (; j < regularRPoints; j += simdSize)
        {
            vcl::Vec8f v1, v2;
            v1.load(reinterpret_cast<const float *>(pt));
            v2.load(reinterpret_cast<const float *>(pt + simdSize / 2));
            vcl::Vec8f dx = vcl::blend8<0, 2, 4, 6, 8, 10, 12, 14>(v1, v2) - Px;
            vcl::Vec8f dy = vcl::blend8<1, 3, 5, 7, 9, 11, 13, 15>(v1, v2) - Py;
            vcl::Vec8f vSqrDist = vcl::square(dx) + vcl::square(dy);

            vcl::Vec8fb more = vSqrDist > vMaxSqrDist;
            vMaxIdx     = vcl::select(more, vIdx, vMaxIdx);
            vMaxSqrDist = vcl::select(more, vSqrDist, vMaxSqrDist);

            pt += simdSize;
            vIdx += vInc;
        }

        if (rPoints >= simdSize)
        {
            int   arrIdx[simdSize];
            float arrDist[simdSize];
            vMaxIdx.store(arrIdx);
            vMaxSqrDist.store(arrDist);
            for (int k = 0; k < simdSize; ++k)
            {
                if (arrDist[k] > maxSqrDist)
                {
                    maxSqrDist = arrDist[k];
                    m = i;
                    n = arrIdx[k&7];
                }
            }
        }

        for (; j < cPoints; ++j)
        {
            cv::Point2f dxy = points[j] - points[i];
            const float sqrDist = dxy.dot(dxy);
            if (sqrDist > maxSqrDist)
            {
                maxSqrDist = sqrDist;
                m = i;
                n = j;
            }
        }
    }

    if (cPoints)
    {
        return cv::Scalar(points[m].x, points[m].y, points[n].x, points[n].y);
    }
    else
    {
        return cv::Scalar();
    }
}

cv::Scalar RotatingCaliper::diameterShamos(const cv::Point2f *points, const int cPoints)
{
    int k = 1;
    const int m = cPoints - 1;
    const bool orientCCW = Util::isLeft(points[m], points[0], points[k]) > 0;

    if (orientCCW)
    {
        while (Util::isLeft(points[m], points[0], points[k + 1]) > Util::isLeft(points[m], points[0], points[k]))
        {
            k += 1;
        }
    }
    else
    {
        while (Util::isLeft(points[m], points[0], points[k + 1]) < Util::isLeft(points[m], points[0], points[k]))
        {
            k += 1;
        }
    }

    int mm = 0;
    int mn = 0;
    float maxSqrDist = 0;

    int i = 0;
    int j = k;

    if (orientCCW)
    {
        while (i <= k && j <= m)
        {
            cv::Point2f dxy = points[j] - points[i];
            float sqrDist = dxy.dot(dxy);
            if (sqrDist > maxSqrDist)
            {
                maxSqrDist = sqrDist;
                mm = i;
                mn = j;
            }

            while ((j < m) && (Util::isLeft(points[i], points[i + 1], points[j + 1]) > Util::isLeft(points[i], points[i + 1], points[j])))
            {
                dxy = points[j] - points[i];
                sqrDist = dxy.dot(dxy);
                if (sqrDist > maxSqrDist)
                {
                    maxSqrDist = sqrDist;
                    mm = i;
                    mn = j;
                }

                j += 1;
            }

            i += 1;
        }
    }
    else
    {
        while (i <= k && j <= m)
        {
            cv::Point2f dxy = points[j] - points[i];
            float sqrDist = dxy.dot(dxy);
            if (sqrDist > maxSqrDist)
            {
                maxSqrDist = sqrDist;
                mm = i;
                mn = j;
            }

            while ((j < m) && (Util::isLeft(points[i], points[i + 1], points[j + 1]) < Util::isLeft(points[i], points[i + 1], points[j])))
            {
                dxy = points[j] - points[i];
                sqrDist = dxy.dot(dxy);
                if (sqrDist > maxSqrDist)
                {
                    maxSqrDist = sqrDist;
                    mm = i;
                    mn = j;
                }

                j += 1;
            }

            i += 1;
        }
    }

    return cv::Scalar(points[mm].x, points[mm].y, points[mn].x, points[mn].y);
}

}
}
