#include "pyellipse.h"
#include <ui/projs/ellipsenode.h>
#include <ui/toplevel/rootframe.h>
#include <ui/projs/stationnode.h>
#include <ui/cv/cairocanvas.h>

PyEllipse::PyEllipse()
{
}

void PyEllipse::SetX(const double x)
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spElli = std::dynamic_pointer_cast<GenericEllipseArcNode>(spObj);
        Geom::Point cPt = spElli->GetCenter();
        PyEllipse::Translate(x-cPt.x(), 0.);
    }
}

void PyEllipse::SetY(const double y)
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spElli = std::dynamic_pointer_cast<GenericEllipseArcNode>(spObj);
        Geom::Point cPt = spElli->GetCenter();
        PyEllipse::Translate(0., y - cPt.y());
    }
}

double PyEllipse::GetX() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spElli = std::dynamic_pointer_cast<GenericEllipseArcNode>(spObj);
        Geom::Point cPt = spElli->GetCenter();
        return cPt.x();
    }
    return 0;
}

double PyEllipse::GetY() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spElli = std::dynamic_pointer_cast<GenericEllipseArcNode>(spObj);
        Geom::Point cPt = spElli->GetCenter();
        return cPt.y();
    }
    return 0;
}

double PyEllipse::GetWidth() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spElli = std::dynamic_pointer_cast<GenericEllipseArcNode>(spObj);
        const auto &rd = spElli->GetData();
        Geom::Point pt0{ rd.points[0][0], rd.points[0][1] };
        Geom::Point pt1{ rd.points[1][0], rd.points[1][1] };
        return Geom::distance(pt0, pt1);
    }
    return 0;
}

double PyEllipse::GetHeight() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spElli = std::dynamic_pointer_cast<GenericEllipseArcNode>(spObj);
        const auto &rd = spElli->GetData();
        Geom::Point pt1{ rd.points[1][0], rd.points[1][1] };
        Geom::Point pt2{ rd.points[2][0], rd.points[2][1] };
        return Geom::distance(pt1, pt2);
    }
    return 0;
}

double PyEllipse::GetStartAngle() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spElli = std::dynamic_pointer_cast<GenericEllipseArcNode>(spObj);
        const auto &rd = spElli->GetData();
        return rd.angles[0];
    }
    return 0;
}

double PyEllipse::GetEndAngle() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spElli = std::dynamic_pointer_cast<GenericEllipseArcNode>(spObj);
        const auto &rd = spElli->GetData();
        return rd.angles[1];
    }
    return 0;
}

std::string PyEllipse::toString() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spElli = std::dynamic_pointer_cast<GenericEllipseArcNode>(spObj);
        std::ostringstream oss;
        const auto &rd = spElli->GetData();
        oss << spElli->GetArcTypeName() <<" " << spElli->GetTitle().ToStdString() <<": { points : [(";
        oss << rd.points[0][0] << ", " << rd.points[0][1] << "), (" << rd.points[1][0] << ", " << rd.points[1][1] << "), (";
        oss << rd.points[2][0] << ", " << rd.points[2][1] << "), (" << rd.points[3][0] << ", " << rd.points[3][1] << ")], angles : [";
        oss << rd.angles[0] << ", " << rd.angles[1] << "]";
        return oss.str();
    }
    else
    {
        return "Invalid Ellipse object";
    }
}

void PyEllipse::Translate(const double delta_x, const double delta_y)
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spElli = std::dynamic_pointer_cast<GenericEllipseArcNode>(spObj);
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        auto spStation = std::dynamic_pointer_cast<StationNode>(spElli->GetParent());
        if (spStation && frame)
        {
            CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
            if (cavs)
            {
                Geom::Affine aff = Geom::Affine::identity();
                aff *= Geom::Translate(delta_x, delta_y);
                cavs->RefreshDrawable(spElli);
                spElli->PyDoTransform(aff);
                cavs->RefreshDrawable(spElli);
                ::wxYield();
            }
        }
    }
}

void PyEllipse::Rotate(const double angle, const bool angle_as_degree)
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        auto spElli = std::dynamic_pointer_cast<GenericEllipseArcNode>(spObj);
        const auto &rd = spElli->GetData();
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        auto spStation = std::dynamic_pointer_cast<StationNode>(spElli->GetParent());
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
                cavs->RefreshDrawable(spElli);
                spElli->PyDoTransform(aff);
                cavs->RefreshDrawable(spElli);
                ::wxYield();
            }
        }
    }
}

void PyExportEllipse(pybind11::module_ &m)
{
    auto c = pybind11::class_<PyEllipse, PyDrawable>(m, "Ellipse");
    c.def_property("x", &PyEllipse::GetX, &PyEllipse::SetX);
    c.def_property("y", &PyEllipse::GetY, &PyEllipse::SetY);
    c.def_property_readonly("width", &PyEllipse::GetWidth);
    c.def_property_readonly("height", &PyEllipse::GetHeight);
    c.def("Translate", &PyEllipse::Translate, "Translate this ellipse", pybind11::arg("delta_x"), pybind11::arg("delta_y"));
    c.def("Rotate", &PyEllipse::Rotate, "Rotate this ellipse", pybind11::arg("angle"), pybind11::arg("angle_as_degree")=true);
    c.def("__repr__", &PyEllipse::toString);
}
