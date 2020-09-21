#include "precomp.hpp"
#include "utility.hpp"
#include <opencv2/mvlab/transformations.hpp>

namespace cv {
namespace mvlab {

cv::Matx33d HomoMat2d::GenIdentity()
{
    return cv::Matx33d(1., 0., 0., 0., 1., 0., 0., 0., 1.);
}

cv::Matx33d HomoMat2d::Translate(const cv::Matx33d &homoMat2D, const cv::Point2d &txy)
{
    cv::Matx33d T(1., 0., txy.x, 0., 1., txy.y, 0., 0., 1.);
    return T * homoMat2D;
}

cv::Matx33d HomoMat2d::TranslateLocal(const cv::Matx33d &homoMat2D, const cv::Point2d &txy)
{
    cv::Matx33d t(1., 0., txy.x, 0., 1., txy.y, 0., 0., 1.);
    return homoMat2D * t;
}

cv::Matx33d HomoMat2d::Rotate(const cv::Matx33d &homoMat2D, const double phi, const cv::Point2d &pxy)
{
    const double sval = std::sin(Util::rad(phi));
    const double cval = std::cos(Util::rad(phi));
    const cv::Matx33d R(cval, -sval, 0., sval, cval, 0., 0., 0., 1.);
    const cv::Matx33d TP(1., 0., pxy.x, 0., 1., pxy.y, 0., 0., 1.);
    const cv::Matx33d TN(1., 0., -pxy.x, 0., 1., -pxy.y, 0., 0., 1.);
    return TP * (R * (TN * homoMat2D));
}

cv::Matx33d HomoMat2d::RotateLocal(const cv::Matx33d &homoMat2D, const double phi)
{
    const double sval = std::sin(Util::rad(phi));
    const double cval = std::cos(Util::rad(phi));
    const cv::Matx33d R(cval, -sval, 0., sval, cval, 0., 0., 0., 1.);
    return homoMat2D * R;
}

cv::Matx33d HomoMat2d::Scale(const cv::Matx33d &homoMat2D, const cv::Size2d &sxy, const cv::Point2d &pxy)
{
    const cv::Matx33d S(sxy.width, 0., 0., 0., sxy.height, 0., 0., 0., 1.);
    const cv::Matx33d TP(1., 0., pxy.x, 0., 1., pxy.y, 0., 0., 1.);
    const cv::Matx33d TN(1., 0., -pxy.x, 0., 1., -pxy.y, 0., 0., 1.);
    return TP * (S * (TN * homoMat2D));
}

cv::Matx33d HomoMat2d::ScaleLocal(const cv::Matx33d &homoMat2D, const cv::Size2d &sxy)
{
    cv::Matx33d S(sxy.width, 0., 0., 0., sxy.height, 0., 0., 0., 1.);
    return homoMat2D * S;
}

cv::Matx33d HomoMat2d::Slant(const cv::Matx33d &homoMat2D, const double theta, const cv::Point2d &pxy, const cv::String &axis)
{
    const cv::Matx33d TP(1., 0., pxy.x, 0., 1., pxy.y, 0., 0., 1.);
    const cv::Matx33d TN(1., 0., -pxy.x, 0., 1., -pxy.y, 0., 0., 1.);
    const double sval = std::sin(Util::rad(theta));
    const double cval = std::cos(Util::rad(theta));
    if ("x" == axis)
    {
        cv::Matx33d S(cval, 0, 0., sval, 1., 0., 0., 0., 1.);
        return TP * (S * (TN * homoMat2D));
    }
    else
    {
        cv::Matx33d S(1., -sval, 0., 0., cval, 0., 0., 0., 1.);
        return TP * (S * (TN * homoMat2D));
    }
}

cv::Matx33d HomoMat2d::SlantLocal(const cv::Matx33d &homoMat2D, const double theta, const cv::String &axis)
{
    const double sval = std::sin(Util::rad(theta));
    const double cval = std::cos(Util::rad(theta));
    if ("x" == axis)
    {
        cv::Matx33d S(cval, 0, 0., sval, 1., 0., 0., 0., 1.);
        return homoMat2D * S;
    }
    else
    {
        cv::Matx33d S(1., -sval, 0., 0., cval, 0., 0., 0., 1.);
        return homoMat2D * S;
    }
}

cv::Matx33d HomoMat2d::Compose(const cv::Matx33d &homoMat2DLeft, const cv::Matx33d &homoMat2DRight)
{
    return homoMat2DLeft * homoMat2DRight;
}

cv::Matx33d HomoMat2d::Invert(const cv::Matx33d &homoMat2D)
{
    return homoMat2D.inv();
}

cv::Matx33d HomoMat2d::Reflect(const cv::Matx33d &homoMat2D, const cv::Point2d &pxy, const cv::Point2d &qxy)
{
    const cv::Matx21d v{ pxy.y-qxy.y, qxy.x-pxy.x };
    const cv::Matx12d vt = v.t();
    const cv::Matx<double, 1, 1> x= vt * v;
    if (std::abs(x.val[0])>std::numeric_limits<double>::epsilon())
    {
        const cv::Matx22d I{1., 0., 0., 1.};
        const cv::Matx22d A = (2 / x.val[0]) * (v * vt);
        const cv::Matx22d M = I - A;
        const cv::Matx33d R{M.val[0], M.val[1], 0., M.val[2], M.val[3], 0., 0., 0., 1.};
        const cv::Matx33d TP(1., 0., pxy.x, 0., 1., pxy.y, 0., 0., 1.);
        const cv::Matx33d TN(1., 0., -pxy.x, 0., 1., -pxy.y, 0., 0., 1.);
        return TP * (R * (TN * homoMat2D));
    }
    else
    {
        return cv::Matx33d::zeros();
    }
}

cv::Matx33d HomoMat2d::ReflectLocal(const cv::Matx33d &homoMat2D, const cv::Point2d &pxy)
{
    const cv::Matx21d v{ -pxy.y, pxy.x };
    const cv::Matx12d vt = v.t();
    const cv::Matx<double, 1, 1> x = vt * v;
    if (std::abs(x.val[0]) > std::numeric_limits<double>::epsilon())
    {
        const cv::Matx22d I{ 1., 0., 0., 1. };
        const cv::Matx22d A = (2 / x.val[0]) * (v * vt);
        const cv::Matx22d M = I - A;
        const cv::Matx33d R{ M.val[0], M.val[1], 0., M.val[2], M.val[3], 0., 0., 0., 1. };
        const cv::Matx33d TP(1., 0., pxy.x, 0., 1., pxy.y, 0., 0., 1.);
        const cv::Matx33d TN(1., 0., -pxy.x, 0., 1., -pxy.y, 0., 0., 1.);
        return TP * (R * (TN * homoMat2D));
    }
    else
    {
        return cv::Matx33d::zeros();
    }
}

cv::Matx33d HomoMat2d::VectorAngleToRigid(const cv::Point2d &point1, const double angle1, const cv::Point2d &point2, const double angle2)
{
    const double sval = std::sin(Util::rad(angle2 - angle1));
    const double cval = std::cos(Util::rad(angle2 - angle1));
    const cv::Matx33d R(cval, -sval, 0., sval, cval, 0., 0., 0., 1.);
    const cv::Matx33d TP(1., 0., point2.x, 0., 1., point2.y, 0., 0., 1.);
    const cv::Matx33d TN(1., 0., -point1.x, 0., 1., -point1.y, 0., 0., 1.);
    return TP * (R * TN);
}

double HomoMat2d::Determinant(const cv::Matx33d &homoMat2D)
{
    return cv::determinant(homoMat2D);
}

cv::Point2d HomoMat2d::AffineTransPixel(const cv::Matx33d &homoMat2D, const cv::Point2d &pixel)
{
    const cv::Point2d point{ pixel.x+0.5, pixel.y+0.5 };
    const cv::Point2d atPoint = AffineTransPoint2d(homoMat2D, point);
    return cv::Point2d(atPoint.x - 0.5, atPoint.y - 0.5);
}

cv::Point2d HomoMat2d::AffineTransPoint2d(const cv::Matx33d &homoMat2D, const cv::Point2d &point)
{
    return cv::Point2d(homoMat2D.val[0] * point.x+homoMat2D.val[1]*point.y + homoMat2D.val[2], homoMat2D.val[3] * point.x + homoMat2D.val[4] * point.y + homoMat2D.val[5]);
}

int HomoMat2d::AffineTransImage(InputArray src, InputOutputArray dst, const cv::Matx33d &homoMat2D, const cv::Ptr<Dict> &opts)
{
    return MLR_SUCCESS;
}

}
}
