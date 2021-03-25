#include "pyentity.h"
#include <ui/projs/drawablenode.h>
#include <ui/toplevel/rootframe.h>
#include <ui/projs/stationnode.h>
#include <ui/cv/cairocanvas.h>

void PyEntity::SetName(const std::string &name)
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
    }
}

pybind11::object PyEntity::GetName() const
{
    auto spObj = wpObj.lock();
    if (spObj)
    {
        return pybind11::cast(spObj->GetTitle().ToStdString());
    }
    return pybind11::none();
}

std::string PyEntity::toString() const
{
    return std::string("Spam entity object");
}

void PyExportEntity(pybind11::module_ &m)
{
    auto c = pybind11::class_<PyEntity>(m, "Entity");
    c.def("SetName", &PyEntity::SetName, "Set entity name", pybind11::arg("name"));
    c.def("GetName", &PyEntity::GetName, "Get entity name");
    c.def_property("name", &PyEntity::GetName, &PyEntity::SetName);
    c.def("__repr__", &PyEntity::toString);
}
