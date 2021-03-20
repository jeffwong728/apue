#include "mvlabcaster.h"

bool IsMvlabPyTypes(const pybind11::handle o, const MvlabPyType t)
{
    try
    {
        pybind11::object mainModule = pybind11::module_::import("__main__");
        pybind11::object mainNamespace = mainModule.attr("__dict__");
        pybind11::eval<pybind11::eval_single_statement>("import mvlab", mainNamespace);

        if (MvlabPyType::kMVPT_REGION == t)
        {
            pybind11::object rgn = pybind11::eval<pybind11::eval_expr>("mvlab.Region_GenEmpty()", mainNamespace);
            return o.get_type().ptr() == rgn.get_type().ptr();
        }
        else if (MvlabPyType::kMVPT_CONTOUR == t)
        {
            pybind11::object contr = pybind11::eval<pybind11::eval_expr>("mvlab.Contour_GenEmpty()", mainNamespace);
            return o.get_type().ptr() == contr.get_type().ptr();
        }
        else
        {
            return false;
        }
    }
    catch (const pybind11::error_already_set&e)
    {
        return false;
    }
}

namespace pybind11 {
namespace detail {

bool type_caster<cv::Mat>::load(handle obj, bool)
{
        array b = reinterpret_borrow<array>(obj);
        buffer_info info = b.request();

        int nh = 1;
        int nw = 1;
        int nc = 1;
        int ndims = info.ndim;
        if (ndims == 2) {
            nh = info.shape[0];
            nw = info.shape[1];
        }
        else if (ndims == 3) {
            nh = info.shape[0];
            nw = info.shape[1];
            nc = info.shape[2];
        }
        else {
            throw std::logic_error("Only support 2d, 2d matrix");
            return false;
        }

        int dtype;
        if (info.format == format_descriptor<unsigned char>::format()) {
            dtype = CV_8UC(nc);
        }
        else if (info.format == format_descriptor<int>::format()) {
            dtype = CV_32SC(nc);
        }
        else if (info.format == format_descriptor<float>::format()) {
            dtype = CV_32FC(nc);
        }
        else {
            throw std::logic_error("Unsupported type, only support uchar, int32, float");
            return false;
        }
        value = cv::Mat(nh, nw, dtype, info.ptr);
        return true;
    }

//! 2. cast cv::Mat to numpy.ndarray    
handle type_caster<cv::Mat>::cast(const cv::Mat& mat, return_value_policy, handle defval)
{
    std::string format = format_descriptor<unsigned char>::format();
    size_t elemsize = sizeof(unsigned char);
    int nw = mat.cols;
    int nh = mat.rows;
    int nc = mat.channels();
    int depth = mat.depth();
    int type = mat.type();
    int dim = (depth == type) ? 2 : 3;
    if (depth == CV_8U) {
        format = format_descriptor<unsigned char>::format();
        elemsize = sizeof(unsigned char);
    }
    else if (depth == CV_32S) {
        format = format_descriptor<int>::format();
        elemsize = sizeof(int);
    }
    else if (depth == CV_32F) {
        format = format_descriptor<float>::format();
        elemsize = sizeof(float);
    }
    else {
        throw std::logic_error("Unsupport type, only support uchar, int32, float");
    }

    std::vector<size_t> bufferdim;
    std::vector<size_t> strides;
    if (dim == 2) {
        bufferdim = { (size_t)nh, (size_t)nw };
        strides = { elemsize * (size_t)nw, elemsize };
    }
    else if (dim == 3) {
        bufferdim = { (size_t)nh, (size_t)nw, (size_t)nc };
        strides = { (size_t)elemsize * nw * nc, (size_t)elemsize * nc, (size_t)elemsize };
    }
    return array(buffer_info(mat.data, elemsize, format, dim, bufferdim, strides)).release();
}

}
}//! end namespace pybind11::detail

