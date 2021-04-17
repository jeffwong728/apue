#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <iostream>

extern void PyExportEntity(pybind11::module_ &m);
extern void PyExportDrawable(pybind11::module_ &m);
extern void PyExportRect(pybind11::module_ &m);
extern void PyExportEllipse(pybind11::module_ &m);
extern void PyExportStation(pybind11::module_ &m);
extern void PyExportProject(pybind11::module_ &m);
extern void PyExportFixed(pybind11::module_ &m);
extern pybind11::object PyNewStation();
extern pybind11::object PyFindStation(const std::string &name);
extern pybind11::object PyGetCurrentProject();

/// Staticly linking a Python extension for embedded Python.
PYBIND11_EMBEDDED_MODULE(spam, m)
{
    PyExportEntity(m);
    PyExportDrawable(m);
    PyExportRect(m);
    PyExportEllipse(m);
    PyExportFixed(m);
    PyExportStation(m);
    PyExportProject(m);
    m.def("CreateStation", PyNewStation);
    m.def("FindStation", PyFindStation);
    m.def("GetCurrentProject", PyGetCurrentProject);
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

void PyAddImportPath(const std::string &strDir)
{
    try
    {
        pybind11::object append = pybind11::module_::import("sys").attr("path").attr("append");
        if (append)
        {
            append(strDir);
        }
    }
    catch (const pybind11::error_already_set&e)
    {
    }
}

std::pair<std::string, bool> PyRunCommand(const std::string &strCmd)
{
    try
    {
        pybind11::object mainModule = pybind11::module_::import("__main__");
        pybind11::object mainNamespace = mainModule.attr("__dict__");
        PyClearOutput();
        pybind11::object resultObj = pybind11::eval<pybind11::eval_single_statement>(strCmd.c_str(), mainNamespace);
        const auto resultTypeStr = resultObj.get_type().str().cast<std::string>();
        const auto noneTypeStr = pybind11::none().get_type().str().cast<std::string>();
        if (resultTypeStr != noneTypeStr)
        {
            return std::make_pair(resultObj.str(), true);
        }
        else
        {
            return std::make_pair(PyGetOutput(), true);
        }
    }
    catch (const pybind11::error_already_set&e)
    {
        return std::make_pair(std::string(e.what()), false);
    }
}
