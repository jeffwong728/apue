#ifndef SPAM_UI_SCRIPTING_PYSTATION_H
#define SPAM_UI_SCRIPTING_PYSTATION_H

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <ui/projs/modelfwd.h>

struct PyStation
{
    std::string GetName() const;
    pybind11::object NewRect(const double center_x, const double center_y, const double width, const double height);
    pybind11::object FuncTest(pybind11::args args, pybind11::kwargs kwargs);
    WPStationNode wpPyStation;
};

#endif //SPAM_UI_SCRIPTING_PYSTATION_H
