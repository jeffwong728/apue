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
        cv::Point2f out[3];
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
                    n = arrIdx[k];
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

}
}
