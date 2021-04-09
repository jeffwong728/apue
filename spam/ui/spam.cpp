// wxWidgets "Hello World" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include "spam.h"
#include <wx/snglinst.h>
#include <ui/proc/basic.h>
#include <ui/toplevel/rootframe.h>
#include <ui/projs/drawablenode.h>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/algorithm/string.hpp>
#include <stack>
#include <memory>
#include <fstream>
#include <gmodule.h>
#include <gtk/gtk.h>

extern std::string InitializePython();
extern void FinalizePython();
extern void PyClearOutput();
extern std::string PyGetOutput();

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

class SpamApp : public wxApp
{
    friend class SpamConfig;
public:
    SpamApp()
        : configTree_(std::make_unique<boost::property_tree::ptree>())
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
    //wxLocale locale_;
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

    std::string strErr = InitializePython();
    Spam::PopupPyError(strErr);
    ::setlocale(LC_ALL, "C");

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

    GtkSettings *settings = gtk_settings_get_default();
    const int darkMode = SpamConfig::Get<bool>(cp_ThemeDarkMode, true);
    g_object_set(G_OBJECT(settings), "gtk-application-prefer-dark-theme", darkMode, NULL);

    RootFrame *frame = new RootFrame();
    SetTopWindow(frame);
    frame->Show(true);
    gtk_widget_show_all(gtk_window_get_titlebar(GTK_WINDOW(frame->m_widget)));
    return true;
}

int SpamApp::OnExit()
{
    SaveConfig();
    FinalizePython();
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
                const int iconSizes[kICON_PURPOSE_GUARD] = { 22, 16, 24 };
                GdkPixbuf *pixBuf = gdk_pixbuf_new_from_file_at_size(wxString(p.native()).ToUTF8().data(), iconSizes[ip], iconSizes[ip], nullptr);
                if (pixBuf)
                {
                    wxBitmap bmp = wxBitmap(pixBuf);
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

void SpamConfig::Set(const std::string &p, const wxColour &v)
{
    if (v.IsOk())
    {
        SpamConfig::Set(p, v.GetRGBA());
    }
}

void SpamConfig::Set(const std::string &p, const wxFont &v)
{
    if (v.IsOk())
    {
        SpamConfig::Set(p, v.GetNativeFontInfoDesc());
    }
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

template<>
wxColour SpamConfig::Get<wxColour>(const std::string &p, const wxColour &v)
{
    const auto &tree = wxGetApp().configTree_;
    boost::optional<wxUint32> ov = tree->get_optional<wxUint32>(p);
    if (ov.is_initialized())
    {
        return wxColour(ov.get());
    }
    else
    {
        return v;
    }
}

template<>
wxFont SpamConfig::Get<wxFont>(const std::string &p, const wxFont &v)
{
    const auto &tree = wxGetApp().configTree_;
    boost::optional<std::string> ov = tree->get_optional<std::string>(p);
    if (ov.is_initialized())
    {
        return wxFont(wxString::FromUTF8(ov.get()));
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
    std::vector<SpamEntityType> typs(ets.cbegin(), ets.cend());
    passTypes_.swap(typs);
}

void SelectionFilter::AddAllPassType()
{
    Clear();
    SpamEntityType t = SpamEntityType::kET_IMAGE;
    while ( t != SpamEntityType::kET_GUARD)
    {
        if (t != SpamEntityType::kET_REGION && t != SpamEntityType::kET_CONTOUR)
        {
            passTypes_.push_back(t);
        }
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

void Spam::PopupPyError(const std::string &strErr)
{
    if (!strErr.empty())
    {
        GUILogerTempSwitcher logSwitcher;
        wxLogError(wxString(strErr));
    }
}

void Spam::ClearPyOutput()
{
    PyClearOutput();
}

void Spam::LogPyOutput()
{
    std::vector<std::string> strLines;
    boost::split(strLines, GetPyOutput(), boost::is_any_of("\n"), boost::token_compress_on);
    for (const auto &l : strLines)
    {
        if (!l.empty())
        {
            wxLogMessage(wxString(l));
        }
    }
}

std::string Spam::GetPyOutput()
{
    return PyGetOutput();
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

CairoCanvas *Spam::FindCanvas(const std::string &uuidTag)
{
    auto frame = dynamic_cast<RootFrame *>(wxGetApp().GetTopWindow());
    if (frame)
    {
        return frame->FindCanvasByUUID(uuidTag);
    }

    return nullptr;
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
