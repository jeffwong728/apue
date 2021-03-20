#include "pyrect.h"
#include <ui/projs/rectnode.h>
#include <ui/toplevel/rootframe.h>
#include <ui/projs/rectnode.h>
#include <ui/projs/stationnode.h>
#include <ui/cv/cairocanvas.h>

PyRect::PyRect()
{
}

void PyRect::SetX(const double x_)
{
}

void PyRect::SetY(const double y_)
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

double PyRect::GetX() const
{
    return 0;
}

double PyRect::GetY() const
{
    return 0;
}

double PyRect::GetWidth() const
{
    return 0;
}

double PyRect::GetHeight() const
{
    return 0;
}

std::string PyRect::toString() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spRect = std::dynamic_pointer_cast<RectNode>(spObj);
        std::ostringstream oss;
        const auto &rd = spRect->GetData();
        oss << spRect->GetTitle().ToStdString() << ": { points : [(";
        oss << rd.points[0][0] << ", " << rd.points[0][1] << "), (" << rd.points[1][0] << ", " << rd.points[1][1] << "), (";
        oss << rd.points[2][0] << ", " << rd.points[2][1] << "), (" << rd.points[3][0] << ", " << rd.points[3][1] << ")], radii : [(";
        oss << rd.radii[0][0] << ", " << rd.radii[0][1] << "), (" << rd.radii[1][0] << ", " << rd.radii[1][1] << "), (";
        oss << rd.radii[2][0] << ", " << rd.radii[2][1] << "), (" << rd.radii[3][0] << ", " << rd.radii[3][1] << ")] }";
        return oss.str();
    }
    else
    {
        return "Invalid object";
    }
}

void PyRect::Translate(const double delta_x, const double delta_y)
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spRect = std::dynamic_pointer_cast<RectNode>(spObj);
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
                ::wxYield();
            }
        }
    }
}

void PyRect::Rotate(const double angle, const bool angle_as_degree)
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spRect = std::dynamic_pointer_cast<RectNode>(spObj);
        const auto &rd = spRect->GetData();
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        auto spStation = std::dynamic_pointer_cast<StationNode>(spRect->GetParent());
        if (spStation && frame)
        {
            CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
            if (cavs)
            {
                const double center_x = (rd.points[0][0] + rd.points[2][0]) / 2;
                const double center_y = (rd.points[0][1] + rd.points[2][1]) / 2;
                Geom::Affine aff = Geom::Affine::identity();
                aff *= Geom::Translate(-center_x, -center_y);
                aff *= Geom::Rotate(angle_as_degree ? Geom::rad_from_deg(angle) : angle);
                aff *= Geom::Translate(center_x, center_y);
                cavs->RefreshDrawable(spRect);
                spRect->PyDoTransform(aff);
                cavs->RefreshDrawable(spRect);
                ::wxYield();
            }
        }
    }
}

void PyExportRect(pybind11::module_ &m)
{
    auto c = pybind11::class_<PyRect, PyDrawable>(m, "Rect");
    c.def_property("x", &PyRect::GetX, &PyRect::SetX);
    c.def_property("y", &PyRect::GetY, &PyRect::SetY);
    c.def_property("width", &PyRect::GetWidth, &PyRect::SetWidth);
    c.def_property("height", &PyRect::GetHeight, &PyRect::SetHeight);
    c.def("Translate", &PyRect::Translate, "Translate this rectangle", pybind11::arg("delta_x"), pybind11::arg("delta_y"));
    c.def("Rotate", &PyRect::Rotate, "Rotate this rectangle", pybind11::arg("angle"), pybind11::arg("angle_as_degree")=true);
    c.def("__repr__", &PyRect::toString);
}
