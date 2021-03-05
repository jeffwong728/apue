#include "pystation.h"
#include "pyrect.h"
#include <ui/toplevel/rootframe.h>
#include <ui/toplevel/projpanel.h>
#include <ui/projs/rectnode.h>
#include <ui/projs/stationnode.h>
#include <ui/projs/projtreemodel.h>
#include <ui/cv/cairocanvas.h>

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
    std::ostringstream oss;
    oss << "NewRect be called with arguments: (" << center_x << ", " << center_y << ", " << width << ", " << height << ")";
    wxLogMessage(wxString(oss.str()));

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
            pyRect.wpRect = cavs->AddRect(rd);
            cavs->RefreshDrawable(pyRect.wpRect.lock());
            return pybind11::cast(pyRect);
        }
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

void PyExportStation(pybind11::module_ &m)
{
    auto c = pybind11::class_<PyStation>(m, "Station");
    c.def("GetName", &PyStation::GetName);
    c.def("NewRect", &PyStation::NewRect, "Create a new rectangle", pybind11::arg("center_x"), pybind11::arg("center_y"), pybind11::arg("width"), pybind11::arg("height"));
    c.def_property_readonly("Name", &PyStation::GetName);
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
