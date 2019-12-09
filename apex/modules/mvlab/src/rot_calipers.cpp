#include "precomp.hpp"
#include "rot_calipers.hpp"
#include "utility.hpp"

namespace cv {
namespace mvlab {

cv::RotatedRect RotatingCaliper::minAreaRect(const cv::Point2f *points, const int cPoints)
{
    RotatedRect box;

    if( cPoints == 2 )
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

}
}
