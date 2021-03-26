#include "pydrawable.h"
#include <ui/projs/drawablenode.h>
#include <ui/toplevel/rootframe.h>
#include <ui/projs/stationnode.h>
#include <ui/cv/cairocanvas.h>

void PyDrawable::SetColor(const RGBATuple &color)
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        SPDrawableNode spDrawable = std::dynamic_pointer_cast<DrawableNode>(spObj);
        wxColour c{ std::get<0>(color), std::get<1>(color), std::get<2>(color), std::get<3>(color) };
        spDrawable->SetColor(c);
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        auto spStation = std::dynamic_pointer_cast<StationNode>(spObj->GetParent());
        if (spStation && frame)
        {
            CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
            if (cavs)
            {
                cavs->RefreshDrawable(spDrawable);
                ::wxYield();
            }
        }
    }
}

pybind11::object PyDrawable::GetColor() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        SPDrawableNode spDrawable = std::dynamic_pointer_cast<DrawableNode>(spObj);
        wxColour cl = spDrawable->GetColor();
        RGBATuple c{ cl.Red(), cl.Green(), cl.Blue(), cl.Alpha() };
        return pybind11::cast(c);
    }
    return pybind11::none();
}

void PyDrawable::SetFillColor(const RGBATuple &color)
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        SPDrawableNode spDrawable = std::dynamic_pointer_cast<DrawableNode>(spObj);
        wxColour c{ std::get<0>(color), std::get<1>(color), std::get<2>(color), std::get<3>(color) };
        spDrawable->SetFillColor(c);
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        auto spStation = std::dynamic_pointer_cast<StationNode>(spObj->GetParent());
        if (spStation && frame)
        {
            CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
            if (cavs)
            {
                cavs->RefreshDrawable(spDrawable);
                ::wxYield();
            }
        }
    }
}

pybind11::object PyDrawable::GetFillColor() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        SPDrawableNode spDrawable = std::dynamic_pointer_cast<DrawableNode>(spObj);
        wxColour cl = spDrawable->GetFillColor();
        RGBATuple c{ cl.Red(), cl.Green(), cl.Blue(), cl.Alpha() };
        return pybind11::cast(c);
    }
    return pybind11::none();
}

void PyDrawable::SetLineWidth(const double width)
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        SPDrawableNode spDrawable = std::dynamic_pointer_cast<DrawableNode>(spObj);
        spDrawable->SetLineWidth(width);
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        auto spStation = std::dynamic_pointer_cast<StationNode>(spObj->GetParent());
        if (spStation && frame)
        {
            CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
            if (cavs)
            {
                cavs->RefreshDrawable(spDrawable);
                ::wxYield();
            }
        }
    }
}

pybind11::object PyDrawable::GetLineWidth() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        SPDrawableNode spDrawable = std::dynamic_pointer_cast<DrawableNode>(spObj);
        return pybind11::cast(spDrawable->GetLineWidth());
    }
    return pybind11::none();
}

std::string PyDrawable::toString() const
{
    return std::string("Drawable object");
}

void PyExportDrawable(pybind11::module_ &m)
{
    auto c = pybind11::class_<PyDrawable, PyEntity>(m, "Drawable");
    c.def("SetColor", &PyDrawable::SetColor, "Set drawable color in RGBA format", pybind11::arg("color"));
    c.def("GetColor", &PyDrawable::GetColor, "Get the current drawable color");
    c.def("SetFillColor", &PyDrawable::SetFillColor, "Set drawable fill color in RGBA format", pybind11::arg("color"));
    c.def("SetFillColor", &PyDrawable::GetFillColor, "Get the current drawable fill color");
    c.def("SetLineWidth", &PyDrawable::SetLineWidth, "Set drawable stroke line width in pixel unit", pybind11::arg("width"));
    c.def("GetLineWidth", &PyDrawable::GetLineWidth, "Get drawable stroke line width in pixel unit");
    c.def_property("color", &PyDrawable::GetColor, &PyDrawable::SetColor);
    c.def_property("fillcolor", &PyDrawable::GetFillColor, &PyDrawable::SetFillColor);
    c.def_property("linewidth", &PyDrawable::GetLineWidth, &PyDrawable::SetLineWidth);
    c.def("__repr__", &PyDrawable::toString);
}
