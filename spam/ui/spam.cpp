// wxWidgets "Hello World" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include "spam.h"
#include <wx/snglinst.h>
#include <ui/toplevel/rootframe.h>
#include <ui/toplevel/projpanel.h>
#include <boost/dll/runtime_symbol_info.hpp>
#include <memory>
#include <fstream>
#pragma warning( push )
#pragma warning( disable : 5033 )
#ifdef pid_t
#undef pid_t
#endif
#ifdef HAVE_SSIZE_T
#undef HAVE_SSIZE_T
#endif
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#pragma warning( pop )
#include <boost/algorithm/string.hpp>

struct GUILogerTempSwitcher
{
    GUILogerTempSwitcher() : GUILogger_(std::make_unique<wxLogGui>())
    {
        oldLogger_ = wxLog::SetActiveTarget(GUILogger_.get());
    }

    ~GUILogerTempSwitcher()
    {
        wxLog::SetActiveTarget(oldLogger_);
    }

    std::unique_ptr<wxLogGui> GUILogger_;
    wxLog *oldLogger_;
};

struct World
{
    void set(std::string msg) { this->msg = msg; }
    std::string greet() { return msg; }
    std::string msg;
};

extern void spamNewStation();

/// Staticly linking a Python extension for embedded Python.
BOOST_PYTHON_MODULE(spam)
{
    boost::python::def("CreateStation", spamNewStation);
    boost::python::class_<World>("World").def("greet", &World::greet).def("set", &World::set);
}

class SpamApp : public wxApp
{
    friend class SpamConfig;
public:
    SpamApp()
        : locale_(wxLANGUAGE_CHINESE_SIMPLIFIED)
        , configTree_(std::make_unique<boost::property_tree::ptree>())
    {}

public:
    bool OnInit() wxOVERRIDE;
    int  OnExit() wxOVERRIDE;

public:
    void AddCommand(const std::shared_ptr<SpamCmd> &cmd);
    void Undo(void);
    void Redo(void);
    bool IsUndoable(void) const { return !undoStack_.empty(); }
    bool IsRedoable(void) const { return !redoStack_.empty(); }

private:
    void SaveConfig();

private:
    wxLocale locale_;
    std::unique_ptr<boost::property_tree::ptree> configTree_;
    std::stack<std::shared_ptr<SpamCmd>> undoStack_;
    std::stack<std::shared_ptr<SpamCmd>> redoStack_;
    std::unique_ptr<wxSingleInstanceChecker> uniqueApp_;
};

wxIMPLEMENT_APP(SpamApp);

bool SpamApp::OnInit()
{
    uniqueApp_.reset(new wxSingleInstanceChecker());
    uniqueApp_->Create(GetAppName(), wxGetHomeDir());

    PyImport_AppendInittab("spam", &initspam);
    Py_Initialize();

    try
    {
        boost::python::object mainModule = boost::python::import("__main__");
        boost::python::object mainNamespace = mainModule.attr("__dict__");
        boost::python::exec("import sys", mainNamespace);
        boost::python::exec("import cStringIO", mainNamespace);
        boost::python::exec("spam_output = cStringIO.StringIO()", mainNamespace);
        boost::python::exec("sys.stdout = spam_output", mainNamespace);
    }
    catch (const boost::python::error_already_set&)
    {
        Spam::PopupPyError();
    }

    boost::system::error_code ec;
    boost::filesystem::path p = boost::dll::program_location(ec);
    p = p.parent_path();
    p.append(wxT("config")).append(wxT("cfg.xml"));

    if (boost::filesystem::exists(p, ec) && boost::filesystem::is_regular_file(p, ec))
    {
        std::ifstream ifs(p.native());
        boost::property_tree::read_xml(ifs, *configTree_, boost::property_tree::xml_parser::trim_whitespace);
    }
    wxInitAllImageHandlers();
    RootFrame *frame = new RootFrame();
    SetTopWindow(frame);
    frame->Show(true);
    return true;
}

int SpamApp::OnExit()
{
    SaveConfig();
    return 0;
}

void SpamApp::SaveConfig()
{
    boost::system::error_code ec;
    boost::filesystem::path p = boost::dll::program_location(ec);
    p = p.parent_path();
    if (boost::filesystem::exists(p, ec) && boost::filesystem::is_directory(p, ec))
    {
        p.append(wxT("config"));
        if (!boost::filesystem::exists(p, ec))
        {
            boost::filesystem::create_directory(p, ec);
        }

        p.append(wxT("cfg.xml"));
        std::ofstream ofs(p.native());
        boost::property_tree::xml_parser::xml_writer_settings<std::string> xmlsetting(' ', 4);
        boost::property_tree::write_xml(ofs, *configTree_, xmlsetting);
    }
}

void SpamApp::AddCommand(const std::shared_ptr<SpamCmd> &cmd)
{
    undoStack_.push(cmd);
    while(!redoStack_.empty()) redoStack_.pop();
}

void SpamApp::Undo(void)
{
    if (!undoStack_.empty())
    {
        auto cmd = undoStack_.top();
        undoStack_.pop();
        cmd->Undo();
        redoStack_.push(cmd);
    }
}

void SpamApp::Redo(void)
{
    if (!redoStack_.empty())
    {
        auto cmd = redoStack_.top();
        redoStack_.pop();
        cmd->Redo();
        undoStack_.push(cmd);
    }
}

void SpamConfig::Save()
{
    wxGetApp().SaveConfig();
}

std::unique_ptr<boost::property_tree::ptree> &SpamConfig::GetPropertyTree()
{
    return wxGetApp().configTree_;
}

void SpamConfig::Set(const std::string &p, const wxString &v)
{
    auto &tree = wxGetApp().configTree_;
    tree->put(p, v.ToUTF8());
}

template<>
wxString SpamConfig::Get<wxString>(const std::string &p, const wxString &v)
{
    const auto &tree = wxGetApp().configTree_;
    boost::optional<std::string> ov = tree->get_optional<std::string>(p);
    if (ov.is_initialized())
    {
        return wxString::FromUTF8(ov.get());
    }
    else
    {
        return v;
    }
}

ProjTreeModel *Spam::GetModel(void)
{
    auto frame = dynamic_cast<RootFrame *>(wxGetApp().GetTopWindow());
    if (frame)
    {
        return frame->GetProjTreeModel();
    }
    else
    {
        return nullptr;
    }
}

void Spam::PopupPyError()
{
    if (PyErr_Occurred())
    {
        PyObject *exc, *val, *tb;
        PyErr_Fetch(&exc, &val, &tb);
        PyErr_NormalizeException(&exc, &val, &tb);

        boost::python::handle<> hexc(boost::python::allow_null(exc));
        boost::python::handle<> hval(boost::python::allow_null(val));
        boost::python::handle<> htb(boost::python::allow_null(tb));

        boost::python::object traceback(boost::python::import("traceback"));
        boost::python::object formatException(traceback.attr("format_exception"));
        boost::python::object formattedList = formatException(hexc, hval, htb);

        boost::python::object formatted = boost::python::str("").join(formattedList);
        std::string estrs = boost::python::extract<std::string>(formatted);
        GUILogerTempSwitcher logSwitcher;
        wxLogError(wxString(estrs));
        PyErr_Clear();
    }
}

void Spam::ClearPyOutput()
{
    boost::python::object mainModule = boost::python::import("__main__");
    boost::python::object mainNamespace = mainModule.attr("__dict__");
    boost::python::exec("spam_output.reset()", mainNamespace);
}

void Spam::LogPyOutput()
{
    boost::python::object mainModule = boost::python::import("__main__");
    boost::python::object mainNamespace = mainModule.attr("__dict__");
    boost::python::exec("spamOut = spam_output.getvalue()", mainNamespace);
    std::string stdOut = boost::python::extract<std::string>(mainNamespace["spamOut"]);
    std::vector<std::string> strLines;
    boost::split(strLines, stdOut, boost::is_any_of("\n"), boost::token_compress_on);
    for (const auto &l : strLines)
    {
        if (!l.empty())
        {
            wxLogMessage(wxString(l));
        }
    }
}

void SpamUndoRedo::AddCommand(const std::shared_ptr<SpamCmd> &cmd)
{
    wxGetApp().AddCommand(cmd);
}

void SpamUndoRedo::Undo(void)
{
    wxGetApp().Undo();
}

void SpamUndoRedo::Redo(void)
{
    wxGetApp().Redo();
}

bool SpamUndoRedo::IsUndoable(void)
{
    return wxGetApp().IsUndoable();
}

bool SpamUndoRedo::IsRedoable(void)
{
    return wxGetApp().IsRedoable();
}

void spamNewStation()
{
    auto rf = dynamic_cast<RootFrame *>(wxGetApp().GetTopWindow());
    rf->GetProjPanel()->CreateStation();
}