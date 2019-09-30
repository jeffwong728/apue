#include "precomp.hpp"
#include "opencv2/mvlab.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

//#define APRIL_DEBUG
#ifdef APRIL_DEBUG
#include "opencv2/imgcodecs.hpp"
#endif

namespace cv {
namespace mvlab {

using namespace std;


Ptr<Board> Board::create(InputArrayOfArrays objPoints, const Ptr<Region> &dictionary, InputArray ids)
{

    CV_Assert(objPoints.total() == ids.total());
    CV_Assert(objPoints.type() == CV_32FC3 || objPoints.type() == CV_32FC1);

    std::vector< std::vector< Point3f > > obj_points_vector;
    for (unsigned int i = 0; i < objPoints.total(); i++) {
        std::vector<Point3f> corners;
        Mat corners_mat = objPoints.getMat(i);

        if(corners_mat.type() == CV_32FC1)
            corners_mat = corners_mat.reshape(3);
        CV_Assert(corners_mat.total() == 4);

        for (int j = 0; j < 4; j++) {
            corners.push_back(corners_mat.at<Point3f>(j));
        }
        obj_points_vector.push_back(corners);
    }

    Ptr<Board> res = makePtr<Board>();
    ids.copyTo(res->ids);
    res->objPoints = obj_points_vector;
    res->dictionary = cv::makePtr<Region>(dictionary);
    return res;
}

}
}
