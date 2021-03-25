#include "pystation.h"
#include "pyproject.h"
#include "pyrect.h"
#include "pyellipse.h"
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/circle.h>
#include <2geom/pathvector.h>
#include <2geom/path-sink.h>
#pragma warning( pop )
#include <ui/toplevel/rootframe.h>
#include <ui/toplevel/projpanel.h>
#include <ui/projs/rectnode.h>
#include <ui/projs/ellipsenode.h>
#include <ui/projs/stationnode.h>
#include <ui/projs/projtreemodel.h>
#include <ui/cv/cairocanvas.h>
#include <opencv2/mvlab.hpp>

std::string PyStation::GetName() const
{
    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        return spStation->GetTitle().ToStdString();
    }
    else
    {
        return std::string();
    }
}

pybind11::object PyStation::NewRect(const double center_x, const double center_y, const double width, const double height)
{
    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            RectData rd;
            rd.points[0][0] = center_x - width / 2;
            rd.points[0][1] = center_y - height / 2;
            rd.points[1][0] = center_x + width / 2;
            rd.points[1][1] = center_y - height / 2;
            rd.points[2][0] = center_x + width / 2;
            rd.points[2][1] = center_y + height / 2;
            rd.points[3][0] = center_x - width / 2;
            rd.points[3][1] = center_y + height / 2;
            rd.radii.fill({ 0, 0 });

            Geom::Affine ide = Geom::Affine::identity();
            for (int i = 0; i < rd.transform.size(); ++i)
            {
                rd.transform[i] = ide[i];
            }

            PyRect pyRect;
            pyRect.wpObj = cavs->AddRect(rd);
            cavs->RefreshDrawable(std::dynamic_pointer_cast<DrawableNode>(pyRect.wpObj.lock()));
            return pybind11::cast(pyRect);
        }
    }

    return pybind11::none();
}

pybind11::object PyStation::NewEllipse(const double center_x, const double center_y, const double width, const double height)
{
    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            GenericEllipseArcData ed;
            ed.points[0][0] = center_x - width / 2;
            ed.points[0][1] = center_y - height / 2;
            ed.points[1][0] = center_x + width / 2;
            ed.points[1][1] = center_y - height / 2;
            ed.points[2][0] = center_x + width / 2;
            ed.points[2][1] = center_y + height / 2;
            ed.points[3][0] = center_x - width / 2;
            ed.points[3][1] = center_y + height / 2;
            ed.angles[0] = 0.;
            ed.angles[1] = 360.;
            ed.type = GenericEllipseArcType::kAtSlice;

            Geom::Affine ide = Geom::Affine::identity();
            for (int i = 0; i < ed.transform.size(); ++i)
            {
                ed.transform[i] = ide[i];
            }

            PyEllipse pyElli;
            pyElli.wpObj = cavs->AddEllipse(ed);
            cavs->RefreshDrawable(std::dynamic_pointer_cast<DrawableNode>(pyElli.wpObj.lock()));
            return pybind11::cast(pyElli);
        }
    }

    return pybind11::none();
}

pybind11::object PyStation::FindEntity(const std::string &name)
{
    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        SPDrawableNode spObj = spStation->FindDrawable(name);
        if (std::dynamic_pointer_cast<RectNode>(spObj))
        {
            PyRect pyObj;
            pyObj.wpObj = spObj;
            return pybind11::cast(pyObj);
        }
        else if (std::dynamic_pointer_cast<GenericEllipseArcNode>(spObj))
        {
            PyEllipse pyObj;
            pyObj.wpObj = spObj;
            return pybind11::cast(pyObj);
        }
        else if (spObj)
        {
            PyDrawable pyObj;
            pyObj.wpObj = spObj;
            return pybind11::cast(pyObj);
        }
        else
        {
            return pybind11::none();
        }
    }

    return pybind11::none();
}

pybind11::object PyStation::GetAllEntities()
{
    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        pybind11::list objs;
        for (const SPModelNode &spNode : spStation->GetChildren())
        {
            if (std::dynamic_pointer_cast<RectNode>(spNode))
            {
                PyRect pyObj;
                pyObj.wpObj = spNode;
                objs.append(pybind11::cast(pyObj));
            }
            else if (std::dynamic_pointer_cast<GenericEllipseArcNode>(spNode))
            {
                PyEllipse pyObj;
                pyObj.wpObj = spNode;
                objs.append(pybind11::cast(pyObj));
            }
            else if (std::dynamic_pointer_cast<DrawableNode>(spNode))
            {
                PyDrawable pyObj;
                pyObj.wpObj = spNode;
                objs.append(pybind11::cast(pyObj));
            }
            else
            {
                // do nothing now
            }
        }

        return objs;
    }

    return pybind11::none();
}

pybind11::object PyStation::FuncTest(pybind11::args args, pybind11::kwargs kwargs)
{
    const auto cArgs = pybind11::len(args);
    const auto cKArgs = pybind11::len(kwargs);

    for (auto item : args)
    {
        std::ostringstream oss;
        oss << "value = " << std::string(pybind11::str(item));
        wxLogMessage(wxString(oss.str()));
    }

    for (auto item : kwargs)
    {
        std::ostringstream oss;
        oss << "key = " << std::string(pybind11::str(item.first)) << ", " << "value = " << std::string(pybind11::str(item.second));
        wxLogMessage(wxString(oss.str()));
    }

    return pybind11::none();
}

void PyStation::DispObj(const cv::Ptr<cv::mvlab::Region> &obj)
{
    SPStationNode spStation = wpPyStation.lock();
    if (spStation && obj)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            cavs->DrawRegion(obj);
            ::wxYield();
        }
    }
}

void PyStation::DispObj(const cv::Ptr<cv::mvlab::Contour> &obj)
{
    SPStationNode spStation = wpPyStation.lock();
    if (spStation && obj)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            cavs->DrawContour(obj);
            ::wxYield();
        }
    }
}

void PyStation::DispCircle(const PyPoint2 &center, const double radius)
{
    if (radius < 0.5)
    {
        throw std::invalid_argument(std::string("Invalid radius ") + std::to_string(radius));
    }

    Geom::Path pth(Geom::Circle(std::get<0>(center), std::get<1>(center), radius));
    Geom::PathVector pathVec;
    pathVec.push_back(pth);

    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            cavs->DrawMarker(pathVec);
            ::wxYield();
        }
    }
}

void PyStation::DispArc(const PyPoint2 &center, const double radius, const PyRange &angle_range, const std::string &specification)
{
    if (radius < 0.5)
    {
        throw std::invalid_argument(std::string("Invalid radius ") + std::to_string(radius));
    }

    Geom::Circle e(std::get<0>(center), std::get<1>(center), radius);

    Geom::PathVector pathVec;
    double dAngle = std::remainder(std::get<1>(angle_range) - std::get<0>(angle_range), 360.0);
    if (dAngle < 0) dAngle += 360;
    if (dAngle < Geom::EPSILON)
    {
        Geom::Path ePath(e);
        pathVec.push_back(ePath);
    }
    else
    {
        Geom::PathBuilder pb(pathVec);
        Geom::Point saPt = e.pointAt(Geom::rad_from_deg(std::get<0>(angle_range)));
        Geom::Point eaPt = e.pointAt(Geom::rad_from_deg(std::get<1>(angle_range)));

        pb.moveTo(saPt);
        if (std::get<1>(angle_range) > std::get<0>(angle_range))
        {
            double da = std::get<1>(angle_range) - std::get<0>(angle_range);
            pb.arcTo(radius, radius, 0, da > 180, true, eaPt);
        }
        else
        {
            double da = std::get<0>(angle_range) - std::get<1>(angle_range);
            pb.arcTo(radius, radius, 0, da < 180, true, eaPt);
        }

        pb.flush();
    }

    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            cavs->DrawMarker(pathVec);
            ::wxYield();
        }
    }
}

void PyStation::DispArrow(const PyPoint2 &start_point, const PyPoint2 &end_point, const double head_size)
{
    if (head_size < 3)
    {
        throw std::invalid_argument(std::string("Invalid head_size ") + std::to_string(head_size));
    }

    Geom::Point sPt(std::get<0>(start_point), std::get<1>(start_point));
    Geom::Point ePt(std::get<0>(end_point), std::get<1>(end_point));
    Geom::Coord arrowLen = Geom::distance(sPt, ePt);
    if (arrowLen < head_size)
    {
        throw std::invalid_argument(std::string("Invalid arrow. Arrow length less than arrow head size"));
    }

    Geom::Point arrowBase = Geom::lerp(head_size / arrowLen, ePt, sPt);
    Geom::Point tPt = arrowBase + Geom::rot90(ePt - arrowBase);
    Geom::Point rPt = Geom::lerp(0.618, tPt, arrowBase);
    Geom::Point lPt = Geom::lerp(2, rPt, arrowBase);

    Geom::PathVector pathVec;
    Geom::PathBuilder pb(pathVec);
    pb.moveTo(ePt);
    pb.lineTo(rPt);
    pb.lineTo(lPt);
    pb.closePath();
    pb.moveTo(sPt);
    pb.lineTo(ePt);
    pb.flush();

    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            cavs->DrawMarker(pathVec);
            ::wxYield();
        }
    }
}

void PyStation::DispCross(const PyPoint2 &center, const double size, const double angle)
{
    if (size < 1)
    {
        throw std::invalid_argument(std::string("Invalid cross size ") + std::to_string(size));
    }

    Geom::Point cPt{ std::get<0>(center), std::get<1>(center) };
    Geom::Point pt0{ size * std::cos(Geom::rad_from_deg(angle)) / 2, size * std::sin(Geom::rad_from_deg(angle)) / 2 };
    Geom::Point pt1 = Geom::rot90(pt0);
    Geom::Point pt2 = Geom::rot90(pt1);
    Geom::Point pt3 = Geom::rot90(pt2);

    Geom::PathVector pathVec;
    Geom::PathBuilder pb(pathVec);
    pb.moveTo(pt0 + cPt);
    pb.lineTo(pt2 + cPt);
    pb.flush();
    pb.moveTo(pt1 + cPt);
    pb.lineTo(pt3 + cPt);
    pb.flush();

    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            cavs->DrawMarker(pathVec);
            ::wxYield();
        }
    }
}

void PyStation::DispEllipse(const PyPoint2 &center, const double phi, const PyPoint2 &radii)
{
    if (std::get<0>(radii) < 0.5 || std::get<1>(radii) < 0.5)
    {
        throw std::invalid_argument(std::string("Invalid ellipse radii"));
    }

    Geom::Point cPt{ std::get<0>(center), std::get<1>(center) };
    Geom::Point rPt{ std::get<0>(radii), std::get<1>(radii) };
    Geom::Ellipse ell(cPt, rPt, Geom::rad_from_deg(phi));
    Geom::Path pth(ell);
    Geom::PathVector pathVec;
    pathVec.push_back(pth);

    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            cavs->DrawMarker(pathVec);
            ::wxYield();
        }
    }
}

void PyStation::DispLine(const PyPoint2 &start_point, const PyPoint2 &end_point)
{
    Geom::Point sPt(std::get<0>(start_point), std::get<1>(start_point));
    Geom::Point ePt(std::get<0>(end_point), std::get<1>(end_point));

    Geom::PathVector pathVec;
    Geom::PathBuilder pb(pathVec);
    pb.moveTo(sPt);
    pb.lineTo(ePt);
    pb.flush();

    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            cavs->DrawMarker(pathVec);
            ::wxYield();
        }
    }
}

void PyStation::DispPolygon(const std::vector<PyPoint2> &vertexes)
{
    if (vertexes.size() < 3)
    {
        throw std::invalid_argument(std::string("Invalid number of vertexes ") + std::to_string(vertexes.size()));
    }

    Geom::PathVector pathVec;
    Geom::PathBuilder pb(pathVec);
    pb.moveTo(Geom::Point(std::get<0>(vertexes.front()), std::get<1>(vertexes.front())));
    for (int nn = 1; nn < static_cast<int>(vertexes.size()); ++nn)
    {
        pb.lineTo(Geom::Point(std::get<0>(vertexes[nn]), std::get<1>(vertexes[nn])));
    }
    pb.closePath();

    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            cavs->DrawMarker(pathVec);
            ::wxYield();
        }
    }
}

void PyStation::DispPolyline(const std::vector<PyPoint2> &points)
{
    if (points.size() < 2)
    {
        throw std::invalid_argument(std::string("Invalid number of points ") + std::to_string(points.size()));
    }

    Geom::PathVector pathVec;
    Geom::PathBuilder pb(pathVec);
    pb.moveTo(Geom::Point(std::get<0>(points.front()), std::get<1>(points.front())));
    for (int nn = 1; nn < static_cast<int>(points.size()); ++nn)
    {
        pb.lineTo(Geom::Point(std::get<0>(points[nn]), std::get<1>(points[nn])));
    }
    pb.flush();

    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            cavs->DrawMarker(pathVec);
            ::wxYield();
        }
    }
}

void PyStation::DispRectangle1(const PyPoint2 &point1, const PyPoint2 &point2)
{
    const Geom::Coord x0 = std::min(std::get<0>(point1), std::get<0>(point2));
    const Geom::Coord x1 = std::max(std::get<0>(point1), std::get<0>(point2));
    const Geom::Coord y0 = std::min(std::get<1>(point1), std::get<1>(point2));
    const Geom::Coord y1 = std::max(std::get<1>(point1), std::get<1>(point2));

    Geom::Path pth(Geom::Rect(x0, y0, x1, y1));
    Geom::PathVector pathVec;
    pathVec.push_back(pth);

    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            cavs->DrawMarker(pathVec);
            ::wxYield();
        }
    }
}

void PyStation::DispRectangle2(const PyPoint2 &center, const double phi, const PyPoint2 &lengths)
{
    if (std::get<0>(lengths) < 0.5 || std::get<1>(lengths) < 0.5)
    {
        throw std::invalid_argument(std::string("Invalid ellipse lengths"));
    }

    const Geom::Coord x0 = -std::get<0>(lengths) / 2;
    const Geom::Coord x1 = std::get<0>(lengths) / 2;
    const Geom::Coord y0 = -std::get<1>(lengths) / 2;
    const Geom::Coord y1 = std::get<1>(lengths) / 2;

    Geom::Path pth(Geom::Rect(x0, y0, x1, y1));
    pth *= Geom::Rotate(Geom::rad_from_deg(phi));
    pth *= Geom::Translate(std::get<0>(center), std::get<1>(center));

    Geom::PathVector pathVec;
    pathVec.push_back(pth);

    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            cavs->DrawMarker(pathVec);
            ::wxYield();
        }
    }
}

void PyStation::EraseObj(const cv::Ptr<cv::mvlab::Region> &obj)
{
    SPStationNode spStation = wpPyStation.lock();
    if (spStation && obj)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            cavs->EraseRegion(obj);
            ::wxYield();
        }
    }
}

void PyStation::EraseBoxArea(const PyPoint2 &point1, const PyPoint2 &point2)
{
    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            Geom::Rect eRect(std::get<0>(point1), std::get<1>(point1), std::get<0>(point2), std::get<1>(point2));
            cavs->EraseBoxArea(eRect);
            ::wxYield();
        }
    }
}

void PyStation::EraseFullArea()
{
    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            cavs->EraseFullArea();
            ::wxYield();
        }
    }
}

void PyStation::SetDraw(const std::string &mode)
{
    if (std::string("fill")==mode || std::string("margin")==mode)
    {
        SPStationNode spStation = wpPyStation.lock();
        if (spStation) spStation->SetDraw(mode);
    }
    else
    {
        throw std::invalid_argument(std::string("Invalid draw mode: ") + mode + std::string(". Valid modes are: 'fill', 'margin'"));
    }
}

pybind11::object PyStation::GetDraw() const
{
    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        return pybind11::cast(spStation->GetDraw());
    }
    else
    {
        return pybind11::none();
    }
}

void PyStation::SetColor(const std::string &color)
{
    wxColour c;
    if (c.Set(color))
    {
        SPStationNode spStation = wpPyStation.lock();
        if (spStation) spStation->SetColor(c);
    }
    else
    {
        throw std::invalid_argument(std::string("Invalid color string: ") + color);
    }
}

void PyStation::SetColor(const uint8_t red, const uint8_t green, const uint8_t blue)
{
    wxColour c{ red, green, blue, 0xFF };
    SPStationNode spStation = wpPyStation.lock();
    if (spStation) spStation->SetColor(c);
}

void PyStation::SetColor(const uint8_t red, const uint8_t green, const uint8_t blue, const uint8_t alpha)
{
    wxColour c{ red, green, blue, alpha };
    SPStationNode spStation = wpPyStation.lock();
    if (spStation) spStation->SetColor(c);
}

void PyStation::SetColor(const std::vector<std::string> &colors)
{
    std::vector<wxColour> cs;
    cs.reserve(colors.size());

    for (const auto &cStr : colors)
    {
        wxColour c;
        if (!c.Set(cStr))
        {
            throw std::invalid_argument(std::string("Invalid color string: ") + cStr);
        }
        cs.push_back(c);
    }

    SPStationNode spStation = wpPyStation.lock();
    if (spStation) spStation->SetColor(cs);
}

void PyStation::SetColor(const std::vector<RGBTuple> &colors)
{
    std::vector<wxColour> cs;
    cs.reserve(colors.size());
    for (const auto &c : colors)
    {
        cs.emplace_back(std::get<0>(c), std::get<1>(c), std::get<2>(c), 0xFF);
    }

    SPStationNode spStation = wpPyStation.lock();
    if (spStation) spStation->SetColor(cs);
}

void PyStation::SetColor(const std::vector<RGBATuple> &colors)
{
    std::vector<wxColour> cs;
    cs.reserve(colors.size());
    for (const auto &c : colors)
    {
        cs.emplace_back(std::get<0>(c), std::get<1>(c), std::get<2>(c), std::get<3>(c));
    }

    SPStationNode spStation = wpPyStation.lock();
    if (spStation) spStation->SetColor(cs);
}

void PyStation::SetColored(const int number_of_colors)
{
    if (number_of_colors < 1)
    {
        throw std::invalid_argument("number_of_colors must be positive");
    }
    else
    {
        SPStationNode spStation = wpPyStation.lock();
        if (spStation) spStation->SetColored(number_of_colors);
    }
}

pybind11::object PyStation::GetColor() const
{
    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        wxColour cl = spStation->GetColor();
        RGBATuple c{ cl.Red(), cl.Green(), cl.Blue(), cl.Alpha() };
        return pybind11::cast(c);
    }

    return pybind11::none();
}

void PyStation::SetImage(const cv::Mat &image)
{
    bool everythingOK = false;
    SPStationNode spStation = wpPyStation.lock();
    if (spStation && !image.empty())
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        int pageIndex = frame->FindImagePanelIndexByUUID(spStation->GetUUIDTag());
        if (pageIndex >=0)
        {
            frame->SetImage(pageIndex, image);
            everythingOK = true;
        }
    }

    if (!everythingOK)
    {
        throw std::invalid_argument("Invalid station or image");
    }
}

pybind11::object PyStation::GetImage() const
{
    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        CairoCanvas *cavs = frame->FindCanvasByUUID(spStation->GetUUIDTag());
        if (cavs)
        {
            cv::Mat m = cavs->GetOriginalImage();
            return pybind11::cast(m);
        }
    }

    return pybind11::none();

}

void PyStation::SetLineWidth(const double width)
{
    if (width < 1)
    {
        throw std::invalid_argument("width >= 1.0");
    }
    else
    {
        SPStationNode spStation = wpPyStation.lock();
        if (spStation) spStation->SetLineWidth(width);
    }
}

pybind11::object PyStation::GetLineWidth() const
{
    SPStationNode spStation = wpPyStation.lock();
    if (spStation)
    {
        return pybind11::cast(spStation->GetLineWidth());
    }

    return pybind11::none();
}

void PyExportStation(pybind11::module_ &m)
{
    auto c = pybind11::class_<PyStation>(m, "Station");
    c.def("GetName", &PyStation::GetName);
    c.def("NewRect", &PyStation::NewRect, "Create a new rectangle", pybind11::arg("center_x"), pybind11::arg("center_y"), pybind11::arg("width"), pybind11::arg("height"));
    c.def("NewEllipse", &PyStation::NewEllipse, "Create a new ellipse", pybind11::arg("center_x"), pybind11::arg("center_y"), pybind11::arg("width"), pybind11::arg("height"));
    c.def("FindEntity", &PyStation::FindEntity, "Find a existing entity by name", pybind11::arg("name"));
    c.def("GetAllEntities", &PyStation::GetAllEntities, "Get all existing entity");
    c.def("DispObj", pybind11::overload_cast<const cv::Ptr<cv::mvlab::Region> &>(&PyStation::DispObj), "Display a region or regions", pybind11::arg("obj"));
    c.def("DispObj", pybind11::overload_cast<const cv::Ptr<cv::mvlab::Contour> &>(&PyStation::DispObj), "Display a contour or contours", pybind11::arg("obj"));
    c.def("DispCircle", &PyStation::DispCircle, "Displays circles in a station", pybind11::arg("center"), pybind11::arg("radius"));
    c.def("DispArc", &PyStation::DispArc, " Displays circular arcs in a station", pybind11::arg("center"), pybind11::arg("radius"), pybind11::arg("angle_range"), pybind11::arg("specification") = std::string("clockwise"));
    c.def("DispArrow", &PyStation::DispArrow, "Displays arrows in a station", pybind11::arg("start_point"), pybind11::arg("end_point"), pybind11::arg("head_size"));
    c.def("DispCross", &PyStation::DispCross, "Displays crosses in a station", pybind11::arg("center"), pybind11::arg("size"), pybind11::arg("angle"));
    c.def("DispEllipse", &PyStation::DispEllipse, "Displays ellipses", pybind11::arg("center"), pybind11::arg("phi"), pybind11::arg("radii"));
    c.def("DispLine", &PyStation::DispLine, "Draws lines in a station", pybind11::arg("start_point"), pybind11::arg("end_point"));
    c.def("DispPolygon", &PyStation::DispPolygon, "Displays a polygon in a station", pybind11::arg("vertexes"));
    c.def("DispPolyline", &PyStation::DispPolyline, "Displays a polyline in a station", pybind11::arg("points"));
    c.def("DispRectangle1", &PyStation::DispRectangle1, "Display of rectangles aligned to the coordinate axes in a station", pybind11::arg("point1"), pybind11::arg("point2"));
    c.def("DispRectangle2", &PyStation::DispRectangle2, "Displays arbitrarily oriented rectangles in a station", pybind11::arg("center"), pybind11::arg("phi"), pybind11::arg("lengths"));
    c.def("SetColor", pybind11::overload_cast<const std::string &>(&PyStation::SetColor), "Set output color by color name", pybind11::arg("color"));
    c.def("SetColor", pybind11::overload_cast<const uint8_t, const uint8_t, const uint8_t>(&PyStation::SetColor), "Set output color in RGB format", pybind11::arg("red"), pybind11::arg("green"), pybind11::arg("blue"));
    c.def("SetColor", pybind11::overload_cast<const uint8_t, const uint8_t, const uint8_t, const uint8_t>(&PyStation::SetColor), "Set output color in RGBA format", pybind11::arg("red"), pybind11::arg("green"), pybind11::arg("blue"), pybind11::arg("alpha"));
    c.def("SetColor", pybind11::overload_cast<const std::vector<std::string> &>(&PyStation::SetColor), "Set output colors by color names", pybind11::arg("colors"));
    c.def("SetColor", pybind11::overload_cast<const std::vector<RGBTuple> &>(&PyStation::SetColor), "Set output colors in RGB format", pybind11::arg("colors"));
    c.def("SetColor", pybind11::overload_cast<const std::vector<RGBATuple> &>(&PyStation::SetColor), "Set output colors in RGBA format", pybind11::arg("colors"));
    c.def("SetColored", &PyStation::SetColored, "Set multiple output colors", pybind11::arg("number_of_colors") = 12);
    c.def("SetDraw", &PyStation::SetDraw, "Define the region fill mode", pybind11::arg("mode"));
    c.def("GetDraw", &PyStation::GetDraw, "Get the current region fill mode");
    c.def("SetLineWidth", &PyStation::SetLineWidth, "Define the line width for region contour output", pybind11::arg("width"));
    c.def("GetLineWidth", &PyStation::GetLineWidth, "Get the current line width for contour display");
    c.def("GetImage", &PyStation::GetImage, "Get the current image");
    c.def("GetColor", &PyStation::GetColor, "Get the current main color");
    c.def("EraseBox", &PyStation::EraseBoxArea, "Erase all displayables inside specified box area", pybind11::arg("point1"), pybind11::arg("point2"));
    c.def("EraseAll", &PyStation::EraseFullArea, "Erase all displayables inside full station display area");
    c.def_property_readonly("name", &PyStation::GetName);
    c.def_property_readonly("entities", &PyStation::GetAllEntities);
    c.def_property("image", &PyStation::GetImage, &PyStation::SetImage);
}

pybind11::object PyNewStation()
{
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (frame)
    {
        WPStationNode wpStation = frame->GetProjPanel()->CreateStation();
        SPStationNode spStation = wpStation.lock();
        if (spStation)
        {
            PyStation pyStation{ wpStation };
            return pybind11::cast(pyStation);
        }
    }

    return pybind11::none();
}

pybind11::object PyFindStation(const std::string &name)
{
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (frame)
    {
        ProjTreeModel *m = frame->GetProjTreeModel();
        if (m)
        {
            SPStationNode spStation = m->FindStationByName(name);
            if (spStation)
            {
                PyStation pyStation{ WPStationNode(spStation) };
                return pybind11::cast(pyStation);
            }
        }
    }

    return pybind11::none();
}

pybind11::object PyGetCurrentProject()
{
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (frame)
    {
        ProjTreeModel *m = frame->GetProjTreeModel();
        if (m)
        {
            SPProjNode spProject = m->GetProject();
            if (spProject)
            {
                PyProject pyProj{ WPProjNode(spProject) };
                return pybind11::cast(pyProj);
            }
        }
    }

    return pybind11::none();
}
