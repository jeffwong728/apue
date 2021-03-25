#include "pyrect.h"
#include <ui/projs/rectnode.h>
#include <ui/toplevel/rootframe.h>
#include <ui/projs/rectnode.h>
#include <ui/projs/stationnode.h>
#include <ui/cv/cairocanvas.h>

PyRect::PyRect()
{
}

void PyRect::SetX(const double x)
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spRect = std::dynamic_pointer_cast<RectNode>(spObj);
        if (spRect)
        {
            Geom::Point cPt = spRect->GetCenter();
            PyRect::Translate(x-cPt.x(), 0.);
        }
    }
}

void PyRect::SetY(const double y)
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spRect = std::dynamic_pointer_cast<RectNode>(spObj);
        if (spRect)
        {
            Geom::Point cPt = spRect->GetCenter();
            PyRect::Translate(0., y - cPt.y());
        }
    }
}

double PyRect::GetX() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spRect = std::dynamic_pointer_cast<RectNode>(spObj);
        if (spRect)
        {
            Geom::Point cPt = spRect->GetCenter();
            return cPt.x();
        }
    }
    return 0;
}

double PyRect::GetY() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spRect = std::dynamic_pointer_cast<RectNode>(spObj);
        Geom::Point cPt = spRect->GetCenter();
        return cPt.y();
    }
    return 0;
}

double PyRect::GetWidth() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spRect = std::dynamic_pointer_cast<RectNode>(spObj);
        Geom::Point cPt = spRect->GetCenter();
        const auto &rd = spRect->GetData();
        Geom::Point pt0{ rd.points[0][0], rd.points[0][1] };
        Geom::Point pt1{ rd.points[1][0], rd.points[1][1] };
        return Geom::distance(pt0, pt1);
    }
    return 0;
}

double PyRect::GetHeight() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spRect = std::dynamic_pointer_cast<RectNode>(spObj);
        Geom::Point cPt = spRect->GetCenter();
        const auto &rd = spRect->GetData();
        Geom::Point pt1{ rd.points[1][0], rd.points[1][1] };
        Geom::Point pt2{ rd.points[2][0], rd.points[2][1] };
        return Geom::distance(pt1, pt2);
    }
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
        return "Invalid Rect object";
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
    c.def_property_readonly("width", &PyRect::GetWidth);
    c.def_property_readonly("height", &PyRect::GetHeight);
    c.def("Translate", &PyRect::Translate, "Translate this rectangle", pybind11::arg("delta_x"), pybind11::arg("delta_y"));
    c.def("Rotate", &PyRect::Rotate, "Rotate this rectangle", pybind11::arg("angle"), pybind11::arg("angle_as_degree")=true);
    c.def("__repr__", &PyRect::toString);
}
