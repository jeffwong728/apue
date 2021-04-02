#ifndef SPAM_UI_SCRIPTING_MVLABCASTER_H
#define SPAM_UI_SCRIPTING_MVLABCASTER_H

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/numpy.h>
#include <opencv2/mvlab.hpp>

enum class MvlabPyType
{
    kMVPT_REGION = 0,
    kMVPT_CONTOUR = 1,
    kMVPT_GUARD = 2
};

using RGBTuple = std::tuple<uint8_t, uint8_t, uint8_t>;
using RGBATuple = std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>;
using PyPoint2 = std::tuple<double, double>;
using PyPoint3 = std::tuple<double, double, double>;
using PyCircle = std::tuple<double, double, double>;
using PyRange  = std::tuple<double, double>;

extern bool IsMvlabPyTypes(const pybind11::handle o, const MvlabPyType t);

namespace pybind11 { namespace detail {

template <>
struct type_caster<cv::Ptr<cv::mvlab::Contour>>
{
public:
    PYBIND11_TYPE_CASTER(cv::Ptr<cv::mvlab::Contour>, _("cv2.mvlab_Contour"));

    bool load(handle src, bool)
    {
        struct pyopencv_Contour
        {
            PyObject_HEAD
            cv::Ptr<cv::mvlab::Contour> v;
        };

        if (IsMvlabPyTypes(src, MvlabPyType::kMVPT_CONTOUR))
        {
            PyObject *source = src.ptr();
            value = ((pyopencv_Contour*)source)->v;
            return true;
        }

        return false;
    }

    static handle cast(cv::Ptr<cv::mvlab::Contour> src, return_value_policy /* policy */, handle /* parent */) {
        return pybind11::none();
    }
};

template <>
struct type_caster<cv::Ptr<cv::mvlab::Region>>
{
    struct pyopencv_Region
    {
        PyObject_HEAD
        cv::Ptr<cv::mvlab::Region> v;
    };
public:
    PYBIND11_TYPE_CASTER(cv::Ptr<cv::mvlab::Region>, _("cv2.mvlab_Region"));

    bool load(handle src, bool)
    {
        if (IsMvlabPyTypes(src, MvlabPyType::kMVPT_REGION))
        {
            PyObject *source = src.ptr();
            value = ((pyopencv_Region*)source)->v;
            return true;
        }

        return false;
    }

    static handle cast(cv::Ptr<cv::mvlab::Region> src, return_value_policy /* policy */, handle /* parent */)
    {
        pybind11::object mainModule = pybind11::module_::import("__main__");
        pybind11::object mainNamespace = mainModule.attr("__dict__");
        pybind11::eval<pybind11::eval_single_statement>("import mvlab", mainNamespace);

        pybind11::object rgn = pybind11::eval<pybind11::eval_expr>("mvlab.Region_GenEmpty()", mainNamespace);
        PyObject *source = rgn.ptr();
        ((pyopencv_Region*)source)->v = src;

        return rgn.release();
    }
};

template<>
struct type_caster<cv::Mat>
{
public:

    PYBIND11_TYPE_CASTER(cv::Mat, _("numpy.ndarray"));

    //! 1. cast numpy.ndarray to cv::Mat    
    bool load(handle obj, bool);
    //! 2. cast cv::Mat to numpy.ndarray    
    static handle cast(const cv::Mat& mat, return_value_policy, handle defval);
};

}
} // namespace pybind11::detail

#endif //SPAM_UI_SCRIPTING_MVLABCASTER_H
