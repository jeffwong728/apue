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
