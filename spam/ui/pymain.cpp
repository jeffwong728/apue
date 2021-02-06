#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <boost/mpl/if.hpp>
#include <boost/optional.hpp>
#include <boost/type_traits/integral_constant.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <ui/toplevel/rootframe.h>
#include <ui/toplevel/projpanel.h>
#include <ui/projs/rectnode.h>
#include <ui/projs/stationnode.h>
#include <ui/projs/projtreemodel.h>
#include <iostream>

struct PyRect
{
    double x, y, w, h;
    WPRectNode wpRect;

    PyRect(const double x_, const double y_, const double w_, const double h_)
        : x(x_), y(y_), w(w_), h(h_)
    {
    }

    void SetX(const double x_) { x = x_; }
    void SetY(const double y_) { y = y_; }

    void SetWidth(const double w_)
    {
        if (w_ > 0.)
        {
            w = w_;
        }
        else
        {
            throw std::invalid_argument("Width must be positive");
        }
    }

    void SetHeight(const double h_)
    {
        if (h_ > 0.)
        {
            h = h_;
        }
        else
        {
            throw std::invalid_argument("Height must be positive");
        }
    }

    std::string toString() const
    {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }

    double GetX() const { return x; }
    double GetY() const { return y; }
    double GetWidth() const { return w; }
    double GetHeight() const { return h; }
};

struct PyStation
{
    std::string GetName() const
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

    boost::optional<PyRect> NewRect(pybind11::tuple args, pybind11::dict kwargs)
    {
        const auto cArgs = pybind11::len(args);
        const auto cKArgs = pybind11::len(kwargs);
        return boost::none;
    }

    WPStationNode wpPyStation;
};

void PyExportRect(pybind11::module_ &m)
{
    auto &c = pybind11::class_<PyRect>(m, "Rect");
    c.def(pybind11::init<const double, const double, const double, const double>());
    c.def_property("X", &PyRect::GetX, &PyRect::SetX);
    c.def_property("Y", &PyRect::GetY, &PyRect::SetY);
    c.def_property("Width", &PyRect::GetWidth, &PyRect::SetWidth);
    c.def_property("Height", &PyRect::GetHeight, &PyRect::SetHeight);
    c.def("__repr__", &PyRect::toString);
}

void PyExportStation(pybind11::module_ &m)
{
    auto &c = pybind11::class_<PyStation>(m, "Station");
    c.def("GetName", &PyStation::GetName);
    c.def("NewRect", &PyStation::NewRect);
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

/// Staticly linking a Python extension for embedded Python.
PYBIND11_EMBEDDED_MODULE(spam, m)
{
    PyExportRect(m);
    PyExportStation(m);
    m.def("CreateStation", PyNewStation);
    m.def("FindStation", PyFindStation);
}

std::string InitializePython()
{
    try {
        //PyImport_AppendInittab("spam", &PyInit_spam);
        pybind11::initialize_interpreter();
        pybind11::object mainModule = pybind11::module_::import("__main__");
        pybind11::object mainNamespace = mainModule.attr("__dict__");
        pybind11::exec("import sys", mainNamespace);
        pybind11::exec("import io", mainNamespace);
        pybind11::exec("spam_output = io.StringIO()", mainNamespace);
        pybind11::exec("sys.stdout = spam_output", mainNamespace);
        return std::string();
    }
    catch (pybind11::error_already_set &e) {
        return std::string(e.what());
    }
}

void FinalizePython()
{
    //PyImport_AppendInittab("spam", &PyInit_spam);
    pybind11::finalize_interpreter();
}

void PyClearOutput()
{
    pybind11::object mainModule = pybind11::module_::import("__main__");
    pybind11::object mainNamespace = mainModule.attr("__dict__");
    pybind11::exec("spam_output = io.StringIO()", mainNamespace);
    pybind11::exec("sys.stdout = spam_output", mainNamespace);
}

std::string PyGetOutput()
{
    pybind11::object mainModule = pybind11::module_::import("__main__");
    pybind11::object mainNamespace = mainModule.attr("__dict__");
    pybind11::exec("spamOut = spam_output.getvalue()", mainNamespace);
    std::string stdOut = pybind11::cast<std::string>(mainNamespace["spamOut"]);
    return stdOut;
}

std::pair<std::string, bool> PyRunFile(const std::string &strFullPath)
{
    try
    {
        pybind11::object mainModule = pybind11::module_::import("__main__");
        pybind11::object mainNamespace = mainModule.attr("__dict__");
        PyClearOutput();
        pybind11::eval_file(strFullPath.c_str(), mainNamespace);
        return std::make_pair(PyGetOutput(), true);
    }
    catch (const pybind11::error_already_set&e)
    {
        return std::make_pair(std::string(e.what()), false);
    }
}

std::pair<std::string, bool> PyRunCommand(const std::string &strCmd)
{
    try
    {
        pybind11::object mainModule = pybind11::module_::import("__main__");
        pybind11::object mainNamespace = mainModule.attr("__dict__");
        PyClearOutput();
        pybind11::exec("_ = None", mainNamespace);
        pybind11::exec(strCmd.c_str(), mainNamespace);
        pybind11::exec("if _ : print(_)", mainNamespace);
        return std::make_pair(PyGetOutput(), true);
    }
    catch (const pybind11::error_already_set&e)
    {
        return std::make_pair(std::string(e.what()), false);
    }
}
