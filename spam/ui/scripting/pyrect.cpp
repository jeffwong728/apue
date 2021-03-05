#include "pyrect.h"
#include <ui/projs/rectnode.h>
#include <ui/toplevel/rootframe.h>
#include <ui/projs/rectnode.h>
#include <ui/projs/stationnode.h>
#include <ui/cv/cairocanvas.h>

PyRect::PyRect()
    : x(0), y(0), w(0), h(0)
{
}

void PyRect::SetWidth(const double w_)
{
    throw std::invalid_argument("Width must be positive");
}

void PyRect::SetHeight(const double h_)
{
    throw std::invalid_argument("Height must be positive");
}

std::string PyRect::toString() const
{
    return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
}

void PyRect::Translate(const double delta_x, const double delta_y)
{
    auto spRect = wpRect.lock();
    if (spRect)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        auto spStation = std::dynamic_pointer_cast<StationNode>(spRect->GetParent());
        if (spStation && frame)
        {
            CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
            if (cavs)
            {
                Geom::Affine aff = Geom::Affine::identity();
                aff *= Geom::Translate(delta_x, delta_y);
                cavs->RefreshDrawable(spRect);
                spRect->PyDoTransform(aff);
                cavs->RefreshDrawable(spRect);
            }
        }
    }
}

void PyExportRect(pybind11::module_ &m)
{
    auto c = pybind11::class_<PyRect>(m, "Rect");
    c.def_property("X", &PyRect::GetX, &PyRect::SetX);
    c.def_property("Y", &PyRect::GetY, &PyRect::SetY);
    c.def_property("Width", &PyRect::GetWidth, &PyRect::SetWidth);
    c.def_property("Height", &PyRect::GetHeight, &PyRect::SetHeight);
    c.def("Translate", &PyRect::Translate, "Translate this rectangle", pybind11::arg("delta_x"), pybind11::arg("delta_y"));
    c.def("__repr__", &PyRect::toString);
}
