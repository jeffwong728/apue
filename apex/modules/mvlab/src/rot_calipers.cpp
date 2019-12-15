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

}
}
