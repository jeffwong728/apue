#ifndef SPAM_UI_SCRIPTING_MVLABCASTER_H
#define SPAM_UI_SCRIPTING_MVLABCASTER_H

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <opencv2/mvlab.hpp>

enum class MvlabPyType
{
    kMVPT_REGION = 0,
    kMVPT_CONTOUR = 1,
    kMVPT_GUARD = 2
};

using RGBTuple = std::tuple<uint8_t, uint8_t, uint8_t>;
using RGBATuple = std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>;

extern bool IsMvlabPyTypes(const pybind11::handle o, const MvlabPyType t);

namespace pybind11 { namespace detail {

template <> struct type_caster<cv::Ptr<cv::mvlab::Contour>>
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

template <> struct type_caster<cv::Ptr<cv::mvlab::Region>>
{
public:
    PYBIND11_TYPE_CASTER(cv::Ptr<cv::mvlab::Region>, _("cv2.mvlab_Region"));

    bool load(handle src, bool)
    {
        struct pyopencv_Region
        {
            PyObject_HEAD
            cv::Ptr<cv::mvlab::Region> v;
        };

        if (IsMvlabPyTypes(src, MvlabPyType::kMVPT_REGION))
        {
            PyObject *source = src.ptr();
            value = ((pyopencv_Region*)source)->v;
            return true;
        }

        return false;
    }

    static handle cast(cv::Ptr<cv::mvlab::Contour> src, return_value_policy /* policy */, handle /* parent */) {
        return pybind11::none();
    }
};

}
} // namespace pybind11::detail

#endif //SPAM_UI_SCRIPTING_MVLABCASTER_H
