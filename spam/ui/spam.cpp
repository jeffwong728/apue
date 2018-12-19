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
#include <ui/projs/drawablenode.h>
#include <wxSVG/SVGDocument.h>

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
    wxBitmap GetBitmap(const SpamIconPurpose ip, const std::string &bmName) const;

private:
    void SaveConfig();

private:
    wxLocale locale_;
    std::unique_ptr<boost::property_tree::ptree> configTree_;
    std::stack<std::shared_ptr<SpamCmd>> undoStack_;
    std::stack<std::shared_ptr<SpamCmd>> redoStack_;
    std::unique_ptr<wxSingleInstanceChecker> uniqueApp_;
    mutable std::unordered_map<std::string, wxBitmap> bitmaps_[kICON_PURPOSE_GUARD];
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

    const wxBitmapType    bmt    = wxBITMAP_TYPE_PNG;
    const SpamIconPurpose ipTBox = kICON_PURPOSE_TOOLBOX;
    bitmaps_[ipTBox][bm_Box]            = wxBitmap(wxT("res/box.png"),              bmt);
    bitmaps_[ipTBox][bm_Ellipse]        = wxBitmap(wxT("res/ellipse.png"),          bmt);
    bitmaps_[ipTBox][bm_Polygon]        = wxBitmap(wxT("res/polygon.png"),          bmt);
    bitmaps_[ipTBox][bm_Beziergon]      = wxBitmap(wxT("res/beziergon.png"),        bmt);
    bitmaps_[ipTBox][bm_Line]           = wxBitmap(wxT("res/line.png"),             bmt);
    bitmaps_[ipTBox][bm_Arc]            = wxBitmap(wxT("res/arc.png"),              bmt);
    bitmaps_[ipTBox][bm_Zigzagline]     = wxBitmap(wxT("res/zigzagline.png"),       bmt);
    bitmaps_[ipTBox][bm_Polyline]       = wxBitmap(wxT("res/polyline.png"),         bmt);
    bitmaps_[ipTBox][bm_Bezierline]     = wxBitmap(wxT("res/bezierline.png"),       bmt);

    const SpamIconPurpose ipTBar = kICON_PURPOSE_TOOLBAR;
    bitmaps_[ipTBar][bm_ImageImport]    = wxBitmap(wxT("res/import_layer_16.png"),  bmt);
    bitmaps_[ipTBar][bm_ImageExport]    = wxBitmap(wxT("res/export_layer_16.png"),  bmt);

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

wxBitmap SpamApp::GetBitmap(const SpamIconPurpose ip, const std::string &bmName) const
{
    if (ip<kICON_PURPOSE_GUARD && ip>= kICON_PURPOSE_TOOLBOX)
    {
        auto &bms = bitmaps_[ip];
        const auto fIt = bms.find(bmName);
        if (fIt != bms.cend())
        {
            return fIt->second;
        }
        else
        {
            boost::system::error_code ec;
            boost::filesystem::path p = boost::dll::program_location(ec);
            p = p.parent_path();
            p.append(wxT("res")).append(wxT("svg")).append(bmName);
            p += wxT(".svg");

            if (boost::filesystem::exists(p, ec) && boost::filesystem::is_regular_file(p, ec))
            {
                auto svgDoc = std::make_unique<wxSVGDocument>();
                svgDoc->Load(wxString(p.native()).ToUTF8());

                const int iconSizes[kICON_PURPOSE_GUARD] = {22, 16, 24};
                wxImage img = svgDoc->Render(iconSizes[ip], iconSizes[ip], 0, true, true);
                if (img.IsOk())
                {
                    wxBitmap bmp = wxBitmap(img);
                    bms[bmName] = bmp;

                    return bmp;
                }
            }
        }
    }

    return wxBitmap();
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

bool SelectionFilter::IsPass(const SPDrawableNode &dn) const
{
    for (const SpamEntityType t : passTypes_)
    {
        if (dn && dn->IsTypeOf(t))
        {
            return true;
        }
    }

    return false;
}

void SelectionFilter::AddPassType(const SpamEntityType et)
{
    auto itLower = std::lower_bound(passTypes_.begin(), passTypes_.end(), et);
    if (itLower == passTypes_.end() || *itLower != et)
    {
        passTypes_.insert(itLower, et);
    }
}

void SelectionFilter::AddPassType(const std::vector<SpamEntityType> &ets)
{
    for (const SpamEntityType et : ets)
    {
        AddPassType(et);
    }
}

void SelectionFilter::ReplacePassType(const SpamEntityType et)
{
    passTypes_.clear();
    passTypes_.push_back(et);
}

void SelectionFilter::ReplacePassType(const std::vector<SpamEntityType> &ets)
{
    passTypes_.swap(std::vector<SpamEntityType>(ets.cbegin(), ets.cend()));
}

void SelectionFilter::AddAllPassType()
{
    Clear();
    SpamEntityType t = SpamEntityType::kET_IMAGE;
    while ( t != SpamEntityType::kET_GUARD)
    {
        passTypes_.push_back(t);
        t = static_cast<SpamEntityType>(static_cast<int>(t) + 1);
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

SelectionFilter *Spam::GetSelectionFilter(void)
{
    auto frame = dynamic_cast<RootFrame *>(wxGetApp().GetTopWindow());
    if (frame)
    {
        return frame->GetSelectionFilter();
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

SPDrawableNodeVector Spam::Difference(const SPDrawableNodeVector& lseq, const SPDrawableNodeVector& rseq)
{
    SPDrawableNodeVector result;

    for (const auto &l : lseq)
    {
        bool reserve = true;
        for (const auto &r : rseq)
        {
            if (l->GetUUIDTag() == r->GetUUIDTag())
            {
                reserve = false;
                break;
            }
        }

        if (reserve)
        {
            result.push_back(l);
        }
    }

    return result;
}

SPDrawableNodeVector Spam::Intersection(const SPDrawableNodeVector& lseq, const SPDrawableNodeVector& rseq)
{
    SPDrawableNodeVector result;

    for (const auto &l : lseq)
    {
        bool reserve = false;
        for (const auto &r : rseq)
        {
            if (l->GetUUIDTag() == r->GetUUIDTag())
            {
                reserve = true;
                break;
            }
        }

        if (reserve)
        {
            result.push_back(l);
        }
    }

    return result;
}

wxBitmap Spam::GetBitmap(const SpamIconPurpose ip, const std::string &bmName)
{
    return wxGetApp().GetBitmap(ip, bmName);
}

void Spam::SetStatus(const StatusIconType iconType, const wxString &text)
{
    auto frame = dynamic_cast<RootFrame *>(wxGetApp().GetTopWindow());
    if (frame)
    {
        frame->SetBitmapStatus(iconType, text);
    }
}

void Spam::InvalidateCanvasRect(const std::string &uuidCanv, const Geom::OptRect &dirtRect)
{
    auto frame = dynamic_cast<RootFrame *>(wxGetApp().GetTopWindow());
    if (frame)
    {
        frame->AddDirtRect(uuidCanv, dirtRect);
    }
}

void Spam::RequestRefreshAllCanvases()
{
    auto frame = dynamic_cast<RootFrame *>(wxGetApp().GetTopWindow());
    if (frame)
    {
        frame->RequestRefreshCanvas();
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