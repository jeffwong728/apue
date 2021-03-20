#include "pyproject.h"
#include <ui/projs/stationnode.h>
#include <ui/projs/projnode.h>

std::string PyProject::GetName() const
{
    SPProjNode spProj = wpPyProject.lock();
    if (spProj)
    {
        return spProj->GetTitle().ToStdString();
    }
    else
    {
        return std::string();
    }
}

std::vector<PyStation> PyProject::GetAllStations() const
{
    std::vector<PyStation> allStations;
    SPProjNode spProj = wpPyProject.lock();
    if (spProj)
    {
        const auto& children = spProj->GetChildren();
        for (const auto &c : children)
        {
            auto station = std::dynamic_pointer_cast<StationNode>(c);
            if (station)
            {
                PyStation pyStation{ station };
                allStations.push_back(pyStation);
            }
        }
    }

    return allStations;
}

void PyExportProject(pybind11::module_ &m)
{
    auto c = pybind11::class_<PyProject>(m, "Project");
    c.def("GetName", &PyProject::GetName);
    c.def("GetAllStations", &PyProject::GetAllStations);
    c.def_property_readonly("name", &PyProject::GetName);
    c.def_property_readonly("stations", &PyProject::GetAllStations);
}
