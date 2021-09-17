// wxWidgets "Hello World" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include "rootframe.h"
#include "projpanel.h"
#include "logpanel.h"
#include "glpanel.h"
#include "consolepanel.h"
#include "pyeditor.h"
#include "preferencesdlg.h"
#include "thumbnailpanel.h"
#include "mainstatus.h"
#include <ui/spam.h>
#include <ui/evts.h>
#include <ui/fsm/spamer.h>
#include <ui/fsm/notool.h>
#include <ui/cv/cvimagepanel.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/stationnode.h>
#include <ui/projs/drawablenode.h>
#include <ui/projs/beziergonnode.h>
#include <ui/projs/projnode.h>
#include <ui/projs/projtreemodel.h>
#include <ui/toolbox/stylebox.h>
#include <ui/toolbox/probebox.h>
#include <ui/toolbox/procbox.h>
#include <ui/toolbox/matchbox.h>
#include <ui/toolbox/geombox.h>
#include <ui/toolbox/imgflowbox.h>
#include <ui/misc/percentvalidator.h>
#include <ui/misc/thumbnailctrl.h>
#include <ui/graphics/gtkglareawidget.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <wx/artprov.h>
#include <wx/statline.h>
#pragma warning( push )
#pragma warning( disable : 4003 )
#include <2geom/bezier.h>
#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <2geom/path-intersection.h>
#include <2geom/svg-path-parser.h>
#include <2geom/svg-path-writer.h>
#include <2geom/cairo-path-sink.h>
#pragma warning( pop )
#include <set>
#include <vector>
#include <functional>
#include <cairo.h>
#include <helper/splivarot.h>
#include <helper/commondef.h>
#include <boost/algorithm/string.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <vtksys/SystemTools.hxx>
extern std::pair<std::string, bool> PyRunFile(const std::string &strFullPath);

RootFrame::RootFrame()
    : wxFrame(NULL, wxID_ANY, wxT("Spam"))
    , mainToolPanelName_(wxT("maintool"))
    , stationNotebookName_(wxT("station"))
    , projPanelName_(wxT("proj"))
    , logPanelName_(wxT("logPanel"))
    , glPanelName_(wxT("glPanel"))
    , consolePanelName_(wxT("consolePanel"))
    , pyEditorName_(wxT("pyEditor"))
    , imagesZonePanelName_(wxT("imagesZone"))
    , toolBoxBarName_(wxT("toolBoxBar"))
    , toolBoxLabels{wxT("toolBoxInfo"), wxT("toolBoxGeom"), wxT("toolBoxProc"), wxT("toolBoxMatch"), wxT("toolBoxStyle"), wxT("toolBoxImgFlow") }
    , imageFileHistory_(9, spamID_BASE_IMAGE_FILE_HISTORY)
    , spamer_(std::make_unique<Spamer>())
    , selFilter_(std::make_unique<SelectionFilter>())
    , cavDirtRects_(std::make_unique<std::map<std::string, Geom::OptRect>>())
{
    imagePanesVisibilities_[stationNotebookName_] = false;
    imagePanesVisibilities_[projPanelName_] = false;
    imagePanesVisibilities_[imagesZonePanelName_] = false;
    imagePanesVisibilities_[toolBoxBarName_] = false;
    imagePanesVisibilities_[toolBoxLabels[kSpam_TOOLBOX_PROBE]] = false;
    imagePanesVisibilities_[toolBoxLabels[kSpam_TOOLBOX_GEOM]] = false;
    imagePanesVisibilities_[toolBoxLabels[kSpam_TOOLBOX_PROC]] = false;
    imagePanesVisibilities_[toolBoxLabels[kSpam_TOOLBOX_MATCH]] = false;
    imagePanesVisibilities_[toolBoxLabels[kSpam_TOOLBOX_STYLE]] = false;
    imagePanesVisibilities_[toolBoxLabels[kSpam_TOOLBOX_IMGFLOW]] = false;
    graphicsPanesVisibilities_[glPanelName_] = false;

    SetGTKGlobalStyle();
    ReplaceTitleBar();
#ifdef _MSC_VER
    SetIcon(wxICON(spam));
#endif
    SetStatusBar(new MainStatus(this));
    SetStatusText("Welcome to Spam!");

    Bind(wxEVT_SIZE, &RootFrame::OnSize, this, wxID_ANY);
    Bind(wxEVT_CLOSE_WINDOW, &RootFrame::OnClose, this, wxID_ANY);
    Bind(wxEVT_UPDATE_UI, &RootFrame::OnUpdateUI, this, wxID_ANY);
    Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &RootFrame::OnStationActivated, this, wxID_ANY);

    wxTheApp->Bind(spamEVT_PROJECT_NEW,     &RootFrame::OnNewProject,    this, wxID_ANY);
    wxTheApp->Bind(spamEVT_PROJECT_LOADED,  &RootFrame::OnLoadProject,   this, wxID_ANY);
    wxTheApp->Bind(spamEVT_DROP_IMAGE,      &RootFrame::OnDropImage,     this, wxID_ANY);

    CreateAuiPanes();
    SetSize(wxSize(800, 600));

    spamer_->initiate();

    auto m = GetProjTreeModel();
    if (m)
    {
        m->sig_StationAdd.connect(std::bind(&RootFrame::OnAddStations, this, std::placeholders::_1));
        m->sig_StationDelete.connect(std::bind(&RootFrame::OnDeleteStations, this, std::placeholders::_1));
        m->sig_GeomAdd.connect(std::bind(&RootFrame::OnAddGeoms, this, std::placeholders::_1));
        m->sig_GeomDelete.connect(std::bind(&RootFrame::OnDeleteGeoms, this, std::placeholders::_1));
        m->sig_GeomDelete.connect(std::bind(&Spamer::OnGeomDelete, spamer_.get(), std::placeholders::_1));
        m->sig_DrawableShapeChange.connect(std::bind(&RootFrame::OnDrawableShapeChange, this, std::placeholders::_1, std::placeholders::_2));
    }

    auto p = GetProjPanel();
    if (p)
    {
        p->sig_EntityGlow.connect(std::bind(&RootFrame::OnGlowGeom, this, std::placeholders::_1));
        p->sig_EntityDim.connect(std::bind(&RootFrame::OnDimGeom, this, std::placeholders::_1));
        p->sig_EntitySelect.connect(std::bind(&Spamer::OnDrawableSelect, spamer_.get(), std::placeholders::_1));
        p->sig_EntitySelect.connect(std::bind(&RootFrame::OnSelectEntity, this, std::placeholders::_1));

        spamer_->sig_EntityDim.connect(std::bind(&ProjPanel::DimEntity, p, std::placeholders::_1));
        spamer_->sig_EntityGlow.connect(std::bind(&ProjPanel::GlowEntity, p, std::placeholders::_1));
        spamer_->sig_EntitySel.connect(std::bind(&ProjPanel::SelectEntity, p, std::placeholders::_1));
        spamer_->sig_EntitySel.connect(std::bind(&RootFrame::OnSelectEntity, this, std::placeholders::_1));
        spamer_->sig_EntityDesel.connect(std::bind(&ProjPanel::DeselectEntity, p, std::placeholders::_1));
    }
}

RootFrame::~RootFrame()
{
    cavDirtRects_.reset();
    wxAuiMgr_.UnInit();
}

void RootFrame::CreateMenu()
{
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H", "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    menuFile->Append(spamID_LOAD_IMAGE, wxT("Load Image"), wxT("Load a image file"), wxITEM_NORMAL);

    wxMenu *menuView = new wxMenu;
    menuView->Append(spamID_VIEW_MAIN_TOOL, wxT("Main Tool Panel"), wxT("Show main tool panel"), wxITEM_CHECK);
    menuView->Append(spamID_VIEW_IMAGE, wxT("Image Panel"), wxT("Show image panel"), wxITEM_CHECK);
    menuView->Append(spamID_VIEW_PROJECT, wxT("Project Panel"), wxT("Show project panel"), wxITEM_CHECK);
    menuView->Append(spamID_VIEW_LOG, wxT("Log Panel"), wxT("Show log panel"), wxITEM_CHECK);
    menuView->Append(spamID_VIEW_IMAGES_ZONE, wxT("Images Zone Panel"), wxT("Show images zone panel"), wxITEM_CHECK);
    menuView->Append(spamID_VIEW_TOOLBOX_BAR, wxT("Toolbox Bar"), wxT("Show toolbox bar"), wxITEM_CHECK);
    menuView->AppendSeparator();
    menuView->Append(spamID_VIEW_DEFAULT_LAYOUT, wxT("Default Layout"), wxT("Show default layout"), wxITEM_NORMAL);
    menuView->AppendSeparator();
    menuView->Append(spamID_VIEW_SET_TILE_LAYOUT, wxT("Set tile layout"), wxT("Set current station view as tile layout"), wxITEM_NORMAL);
    menuView->AppendSeparator();
    menuView->Append(spamID_VIEW_TILE_LAYOUT, wxT("Tile layout"), wxT("Tile station view"), wxITEM_NORMAL);
    menuView->Append(spamID_VIEW_STACK_LAYOUT, wxT("Stack layout"), wxT("Stack station view"), wxITEM_NORMAL);

    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuView, "&View");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);

    Bind(wxEVT_MENU, &RootFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &RootFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &RootFrame::OnLoadImage, this, spamID_LOAD_IMAGE);
    Bind(wxEVT_MENU, &RootFrame::OnViewMainTool, this, spamID_VIEW_MAIN_TOOL);
    Bind(wxEVT_MENU, &RootFrame::OnViewImage, this, spamID_VIEW_IMAGE);
    Bind(wxEVT_MENU, &RootFrame::OnViewDefaultLayout, this, spamID_VIEW_DEFAULT_LAYOUT);
    Bind(wxEVT_MENU, &RootFrame::OnSetTileLayout, this, spamID_VIEW_SET_TILE_LAYOUT);
    Bind(wxEVT_MENU, &RootFrame::OnTileLayout, this, spamID_VIEW_TILE_LAYOUT);
    Bind(wxEVT_MENU, &RootFrame::OnStackLayout, this, spamID_VIEW_STACK_LAYOUT);
}

void RootFrame::CreateAuiPanes()
{
    wxAuiMgr_.SetManagedWindow(this);
    Bind(wxEVT_AUI_PANE_CLOSE, &RootFrame::OnAuiPageClosed, this);

    wxAuiToolBar* tbBar = new wxAuiToolBar(this, kSpamToolboxBar, wxDefaultPosition, wxSize(36, 200), wxAUI_TB_VERTICAL);
    tbBar->AddTool(kSpamID_TOOLBOX_PROBE, toolBoxLabels[kSpam_TOOLBOX_PROBE], wxArtProvider::GetBitmap(wxART_NEW), wxT("Image Infomation"), wxITEM_CHECK);
    tbBar->AddTool(kSpamID_TOOLBOX_GEOM, toolBoxLabels[kSpam_TOOLBOX_GEOM], wxArtProvider::GetBitmap(wxART_NEW), wxT("Geometry Tool"), wxITEM_CHECK);
    tbBar->AddTool(kSpamID_TOOLBOX_PROC, toolBoxLabels[kSpam_TOOLBOX_PROC], wxArtProvider::GetBitmap(wxART_NEW), wxT("Image Processing"), wxITEM_CHECK);
    tbBar->AddTool(kSpamID_TOOLBOX_MATCH, toolBoxLabels[kSpam_TOOLBOX_MATCH], wxArtProvider::GetBitmap(wxART_NEW), wxT("Pattern Match"), wxITEM_CHECK);
    tbBar->AddTool(kSpamID_TOOLBOX_IMGFLOW, toolBoxLabels[kSpam_TOOLBOX_IMGFLOW], wxArtProvider::GetBitmap(wxART_NEW), wxT("Pipeline Processing"), wxITEM_CHECK);
    tbBar->AddSeparator();
    tbBar->AddTool(kSpamID_TOOLBOX_STYLE, toolBoxLabels[kSpam_TOOLBOX_STYLE], wxArtProvider::GetBitmap(wxART_NEW), wxT("Geometry Style"), wxITEM_CHECK);
    tbBar->Realize();

    tbBar->Bind(wxEVT_TOOL, &RootFrame::OnToolboxInfo,      this, kSpamID_TOOLBOX_PROBE);
    tbBar->Bind(wxEVT_TOOL, &RootFrame::OnToolboxGeom,      this, kSpamID_TOOLBOX_GEOM);
    tbBar->Bind(wxEVT_TOOL, &RootFrame::OnToolboxProc,      this, kSpamID_TOOLBOX_PROC);
    tbBar->Bind(wxEVT_TOOL, &RootFrame::OnToolboxMatch,     this, kSpamID_TOOLBOX_MATCH);
    tbBar->Bind(wxEVT_TOOL, &RootFrame::OnToolboxStyle,     this, kSpamID_TOOLBOX_STYLE);
    tbBar->Bind(wxEVT_TOOL, &RootFrame::OnToolboxImgFlow,   this, kSpamID_TOOLBOX_IMGFLOW);

    wxAuiMgr_.AddPane(CreateStationNotebook(), wxAuiPaneInfo().Name(stationNotebookName_).Center().PaneBorder(false).CloseButton(false).CaptionVisible(false));
    wxAuiMgr_.AddPane(new ProjPanel(this), wxAuiPaneInfo().Name(projPanelName_).Left().Caption(wxT("Project Explorer")));
    wxAuiMgr_.AddPane(new PyEditor(this), wxAuiPaneInfo().Name(pyEditorName_).Right().Bottom().Caption("Script Editor"));
    wxAuiMgr_.AddPane(new LogPanel(this), wxAuiPaneInfo().Name(logPanelName_).Left().Bottom().Caption("Log"));
    wxAuiMgr_.AddPane(new ConsolePanel(this), wxAuiPaneInfo().Name(consolePanelName_).Right().Bottom().Caption("Console"));
    wxAuiMgr_.AddPane(new ThumbnailPanel(this), wxAuiPaneInfo().Name(imagesZonePanelName_).Bottom().Bottom().Caption("Images Zone"));
    wxAuiMgr_.AddPane(new GLPanel(this), wxAuiPaneInfo().Name(glPanelName_).Center().PaneBorder(false).CloseButton(false).CaptionVisible(false).Show(false));

    auto infoBox  = new ProbeBox(this);
    auto geomBox  = new GeomBox(this);
    auto procBox  = new ProcBox(this);
    auto matchBox = new MatchBox(this);
    auto styleBox = new StyleBox(this);
    auto imgFlowBox = new ImgFlowBox(this);

    infoBox->sig_ToolEnter.connect(std::bind(&Spamer::OnToolEnter, spamer_.get(), std::placeholders::_1));
    infoBox->sig_ToolQuit.connect(std::bind(&Spamer::OnToolQuit, spamer_.get(), std::placeholders::_1));
    infoBox->sig_OptionsChanged.connect(std::bind(&Spamer::OnToolOptionsChanged, spamer_.get(), std::placeholders::_1));
    geomBox->sig_ToolEnter.connect(std::bind(&Spamer::OnToolEnter, spamer_.get(), std::placeholders::_1));
    geomBox->sig_ToolQuit.connect(std::bind(&Spamer::OnToolQuit, spamer_.get(), std::placeholders::_1));
    geomBox->sig_OptionsChanged.connect(std::bind(&Spamer::OnToolOptionsChanged, spamer_.get(), std::placeholders::_1));
    procBox->sig_ToolEnter.connect(std::bind(&Spamer::OnToolEnter, spamer_.get(), std::placeholders::_1));
    procBox->sig_ToolQuit.connect(std::bind(&Spamer::OnToolQuit, spamer_.get(), std::placeholders::_1));
    procBox->sig_OptionsChanged.connect(std::bind(&Spamer::OnToolOptionsChanged, spamer_.get(), std::placeholders::_1));
    matchBox->sig_ToolEnter.connect(std::bind(&Spamer::OnToolEnter, spamer_.get(), std::placeholders::_1));
    matchBox->sig_ToolQuit.connect(std::bind(&Spamer::OnToolQuit, spamer_.get(), std::placeholders::_1));
    matchBox->sig_OptionsChanged.connect(std::bind(&Spamer::OnToolOptionsChanged, spamer_.get(), std::placeholders::_1));

    wxAuiMgr_.AddPane(infoBox, wxAuiPaneInfo().Name(toolBoxLabels[kSpam_TOOLBOX_PROBE]).Right().Caption("Probe").Show(false));
    wxAuiMgr_.AddPane(geomBox, wxAuiPaneInfo().Name(toolBoxLabels[kSpam_TOOLBOX_GEOM]).Right().Caption("Geometry Tool").Show(false));
    wxAuiMgr_.AddPane(procBox, wxAuiPaneInfo().Name(toolBoxLabels[kSpam_TOOLBOX_PROC]).Right().Caption("Image Processing").Show(false));
    wxAuiMgr_.AddPane(matchBox, wxAuiPaneInfo().Name(toolBoxLabels[kSpam_TOOLBOX_MATCH]).Right().Caption("Template Matching").Show(false));
    wxAuiMgr_.AddPane(imgFlowBox, wxAuiPaneInfo().Name(toolBoxLabels[kSpam_TOOLBOX_IMGFLOW]).Right().Caption("Pipeline Processing").Show(false));
    wxAuiMgr_.AddPane(styleBox, wxAuiPaneInfo().Name(toolBoxLabels[kSpam_TOOLBOX_STYLE]).Right().Caption("Style").Show(false));


    initialPerspective_ = wxAuiMgr_.SavePerspective();
    const auto &projPerspective = SpamConfig::Get<wxString>(CommonDef::GetProjPanelCfgPath());
    if (!projPerspective.empty())
    {
        wxAuiMgr_.LoadPerspective(projPerspective);
    }

    wxAuiPaneInfo toolBoxBarPaneInfo;
    toolBoxBarPaneInfo.Name(toolBoxBarName_).Caption(wxT("Toolbox Bar")).ToolbarPane().Right().Gripper(false).BestSize(36, 200);
    wxAuiMgr_.AddPane(tbBar, toolBoxBarPaneInfo);
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_PROBE]).Gripper(false);
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_GEOM]).Gripper(false);
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_PROC]).Gripper(false);
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_MATCH]).Gripper(false);
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_IMGFLOW]).Gripper(false);
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_STYLE]).Gripper(false);
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_PROBE]).CaptionVisible(false);
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_GEOM]).CaptionVisible(false);
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_PROC]).CaptionVisible(false);
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_MATCH]).CaptionVisible(false);
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_IMGFLOW]).CaptionVisible(false);
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_STYLE]).CaptionVisible(false);
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_PROBE]).MinSize(infoBox->GetSizer()->GetMinSize());
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_GEOM]).MinSize(geomBox->GetSizer()->GetMinSize());
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_PROC]).MinSize(procBox->GetSizer()->GetMinSize());
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_MATCH]).MinSize(matchBox->GetSizer()->GetMinSize());
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_IMGFLOW]).MinSize(imgFlowBox->GetSizer()->GetMinSize());
    wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_STYLE]).MinSize(styleBox->GetSizer()->GetMinSize());

    auto pyEditor = dynamic_cast<PyEditor *>(wxAuiMgr_.GetPane(pyEditorName_).window);
    if (pyEditor)
    {
        pyEditor->LoadDefaultPyFile();
        boost::system::error_code ec;
        const std::wstring ansiFilePath = SpamConfig::Get<wxString>(cp_Py3EditorScriptFullPath, wxT("")).ToStdWstring();
        boost::filesystem::path p(ansiFilePath);
        wxAuiMgr_.GetPane(pyEditorName_).Caption(wxString("Script Editor - ") + wxString(p.filename().string()));
    }

    ToggleToolboxPane(kSpam_TOOLBOX_GUARD);
    wxAuiMgr_.GetArtProvider()->SetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE, 0);
    wxAuiMgr_.Update();
}

ProjPanel *RootFrame::GetProjPanel()
{
    return dynamic_cast<ProjPanel *>(wxAuiMgr_.GetPane(projPanelName_).window);
}

void RootFrame::SaveProject(const wxString &dbPath)
{
    ProjPanel *projPanel = GetProjPanel();
    if (projPanel)
    {
        projPanel->SaveProject(dbPath);
        SpamConfig::Set(CommonDef::GetDBPathCfgPath(), dbPath);
        SpamConfig::Set(CommonDef::GetProjCfgPath(), projPanel->GetProjectName());
        SpamConfig::Save();
    }

    auto pyEditor = dynamic_cast<PyEditor *>(wxAuiMgr_.GetPane(pyEditorName_).window);
    if (pyEditor)
    {
        pyEditor->SavePyFile();
    }
}

void RootFrame::PlayScript()
{
    auto pyEditor = dynamic_cast<PyEditor *>(wxAuiMgr_.GetPane(pyEditorName_).window);
    if (pyEditor)
    {
        pyEditor->PlayPyFile();
    }
}

wxAuiNotebook *RootFrame::GetStationNotebook() const
{
    return dynamic_cast<wxAuiNotebook *>(wxAuiMgr_.GetPane(stationNotebookName_).window);
}

ProjTreeModel *RootFrame::GetProjTreeModel()
{
    auto projPanel = GetProjPanel();
    if (projPanel)
    {
        return projPanel->GetProjTreeModel();
    }

    return nullptr;
}

wxGLAreaWidget *RootFrame::GetGLWidget() const
{
    auto glPanel = dynamic_cast<GLPanel *>(wxAuiMgr_.GetPane(glPanelName_).window);
    if (glPanel)
    {
        return glPanel->GetGLWidget();
    }

    return nullptr;
}

CairoCanvas *RootFrame::FindCanvasByUUID(const std::string &uuidTag) const
{
    auto stationNB = GetStationNotebook();
    if (stationNB)
    {
        int cPages = static_cast<int>(stationNB->GetPageCount());
        for (int i=0; i<cPages; ++i)
        {
            wxWindow *page = stationNB->GetPage(i);
            if (page && (page->GetName() == uuidTag))
            {
                CVImagePanel *panel = dynamic_cast<CVImagePanel *>(page);
                if (panel)
                {
                    return panel->GetCanvas();
                }
            }
        }
    }

    return nullptr;
}

int RootFrame::FindImagePanelIndexByUUID(const std::string &uuidTag) const
{
    auto stationNB = GetStationNotebook();
    if (stationNB)
    {
        int cPages = static_cast<int>(stationNB->GetPageCount());
        for (int i = 0; i < cPages; ++i)
        {
            wxWindow *page = stationNB->GetPage(i);
            if (page && (page->GetName() == uuidTag))
            {
                return i;
            }
        }
    }

    return -1;
}

void RootFrame::UpdateToolboxUI(const int toolboxId, const int toolId, const std::string &uuidTag, const boost::any &roi)
{
    ToolBox *toolBox = nullptr;
    switch (toolboxId)
    {
    case kSpamID_TOOLBOX_PROBE: toolBox = dynamic_cast<ToolBox *>(wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_PROBE]).window); break;
    case kSpamID_TOOLBOX_GEOM: toolBox = dynamic_cast<ToolBox *>(wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_GEOM]).window); break;
    case kSpamID_TOOLBOX_PROC: toolBox = dynamic_cast<ToolBox *>(wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_PROC]).window); break;
    case kSpamID_TOOLBOX_MATCH: toolBox = dynamic_cast<ToolBox *>(wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_MATCH]).window); break;
    case kSpamID_TOOLBOX_IMGFLOW: toolBox = dynamic_cast<ToolBox *>(wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_IMGFLOW]).window); break;
    case kSpamID_TOOLBOX_STYLE: toolBox = dynamic_cast<ToolBox *>(wxAuiMgr_.GetPane(toolBoxLabels[kSpam_TOOLBOX_STYLE]).window); break;
    default: break;
    }

    if (toolBox)
    {
        toolBox->UpdateUI(toolId, uuidTag, roi);
    }
}

void RootFrame::AddDirtRect(const std::string &uuidTag, const Geom::OptRect &dirtRect)
{
    (*cavDirtRects_)[uuidTag].unionWith(dirtRect);
}

void RootFrame::RequestRefreshCanvas()
{
    if (cavDirtRects_)
    {
        for (const auto &dirtCanv : *cavDirtRects_)
        {
            CairoCanvas *canv = FindCanvasByUUID(dirtCanv.first);
            if (canv)
            {
                canv->DrawPathVector(Geom::PathVector(), dirtCanv.second);
            }
        }

        cavDirtRects_->clear();
    }
}

void RootFrame::SyncScale(double scale, wxAuiNotebook *nb, wxWindow *page)
{
    if (nb && page)
    {
        scale_ = scale;
        if (page->IsShown())
        {
            SyncScale(nb->FindExtensionCtrlByPage(page));
        }
    }
}

void RootFrame::SetStatusText(const wxString &text, int number)
{
    if (0 == number)
    {
        auto mainStatus = dynamic_cast<MainStatus *>(GetStatusBar());
        mainStatus->SetTextStatus(text);
    }
    else
    {
        GetStatusBar()->SetStatusText(text, number);
    }
}

void RootFrame::SetBitmapStatus(const StatusIconType iconType, const wxString &text)
{
    auto mainStatus = dynamic_cast<MainStatus *>(GetStatusBar());
    mainStatus->SetBitmapStatus(iconType, text);
}

void RootFrame::SetImage(const int pageIndex, const cv::Mat &image)
{
    auto stationNB = GetStationNotebook();
    if (stationNB && pageIndex >=0 && pageIndex < static_cast<int>(stationNB->GetPageCount()))
    {
        auto imgPanelPage = dynamic_cast<CVImagePanel *>(stationNB->GetPage(pageIndex));
        if (imgPanelPage)
        {
            scale_ = imgPanelPage->SetImage(image);
            auto extCtrl = stationNB->FindExtensionCtrlByPage(imgPanelPage);
            if (imgPanelPage->HasImage())
            {
                SetStationImage(imgPanelPage->GetName(), imgPanelPage->GetImage());
                EnablePageImageTool(extCtrl, true);
                SyncScale(extCtrl);
            }
            else
            {
                EnablePageImageTool(extCtrl, false);
            }
        }
    }
}

void RootFrame::SwitchMission(const bool toImage)
{
    if (toImage)
    {
        for (auto &paneItem : imagePanesVisibilities_)
        {
            auto &pane = wxAuiMgr_.GetPane(paneItem.first);
            pane.Show(paneItem.second);
        }

        wxAuiMgr_.GetPane(stationNotebookName_).Show(true);

        for (auto &paneItem : graphicsPanesVisibilities_)
        {
            auto &pane = wxAuiMgr_.GetPane(paneItem.first);
            pane.Show(false);
        }
    }
    else
    {
        for (auto &paneItem : imagePanesVisibilities_)
        {
            auto &pane = wxAuiMgr_.GetPane(paneItem.first);
            paneItem.second = pane.IsShown();
            pane.Show(false);
        }

        for (auto &paneItem : graphicsPanesVisibilities_)
        {
            auto &pane = wxAuiMgr_.GetPane(paneItem.first);
            pane.Show(true);
        }
    }

    wxAuiMgr_.Update();
}

void RootFrame::OnExit(wxCommandEvent& e)
{
    Close(false);
}

void RootFrame::OnClose(wxCloseEvent& e)
{
    SpamConfig::Set(CommonDef::GetProjPanelCfgPath(), wxAuiMgr_.SavePerspective());
    auto projPanel = GetProjPanel();
    if (projPanel && projPanel->IsProjectModified())
    {
        wxMessageDialog dialog(this, wxT("Project have modified. Do you want to save first?"),
            wxT("Project Modified"), wxCENTER | wxNO_DEFAULT | wxYES_NO | wxCANCEL | wxICON_WARNING);
        dialog.SetYesNoCancelLabels(wxT("&Yes"), wxT("&Discard changes"), wxT("&Cancel"));
        if (wxID_NO == dialog.ShowModal())
        {
            e.Skip();
        }
        else
        {
            e.Veto();
        }
    }
    else
    {
        e.Skip();
    }

    spamer_->OnAppQuit();
}

void RootFrame::OnHello(wxCommandEvent& event)
{
    //wxLogMessage("Hello world from wxWidgets!");
    wxString wildCard{ "Python files (*.py)|*.py" };
    wxFileDialog openFileDialog(this, wxT("Open Python file"), "", "", wildCard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() != wxID_CANCEL)
    {
        auto fullPath = openFileDialog.GetPath();
        std::pair<std::string, bool> res = PyRunFile(fullPath.ToStdString());
        if (res.second)
        {
            Spam::LogPyOutput();
        }
        else
        {
            Spam::PopupPyError(res.first);
        }
    }
}

void RootFrame::OnLoadImage(wxCommandEvent& WXUNUSED(e))
{
    wxString wildCard{"PNG files (*.png)|*.png"};
    wildCard.Append("|JPEG files (*.jpg;*.jpeg)|*.jpg;*.jpeg");
    wildCard.Append("|BMP files (*.bmp)|*.bmp");
    wildCard.Append("|GIF files (*.gif)|*.gif");
    wildCard.Append("|GIF files (*.tif)|*.tif");
    wildCard.Append("|All files (*.*)|*.*");

    wxFileDialog openFileDialog(this, wxT("Open image file"), "", "", wildCard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() != wxID_CANCEL)
    {
        auto fullPath = openFileDialog.GetPath();

        auto stationNB = GetStationNotebook();
        if (stationNB)
        {
            auto sel = stationNB->GetSelection();
            if (wxNOT_FOUND != sel)
            {
                auto imgPanelPage = dynamic_cast<CVImagePanel *>(stationNB->GetPage(sel));
                if (imgPanelPage)
                {
                    scale_ = imgPanelPage->LoadImageFromFile(fullPath);
                    auto extCtrl = stationNB->FindExtensionCtrlByPage(imgPanelPage);
                    if (imgPanelPage->HasImage())
                    {
                        SetStationImage(imgPanelPage->GetName(), imgPanelPage->GetImage());
                        EnablePageImageTool(extCtrl, true);
                        SyncScale(extCtrl);
                    }
                    else
                    {
                        EnablePageImageTool(extCtrl, false);
                    }

                    imageFileHistory_.AddFileToHistory(fullPath);
                }
            }
        }
    }
}

void RootFrame::OnLoadModel(wxCommandEvent& WXUNUSED(e))
{
    auto glWidget = GetGLWidget();
    if (glWidget)
    {
        const GLGUID guid(0, 0);
        glWidget->OnImportModel(guid);
    }
}

void RootFrame::OnCloseModel(wxCommandEvent& WXUNUSED(e))
{
    auto glWidget = GetGLWidget();
    if (glWidget)
    {
        glWidget->CloseModel();
    }
}

void RootFrame::OnLoadPy3(wxCommandEvent& event)
{
    wxString wildCard{ "Python 3 files (*.py;*.pyw)|*.py;*.pyw" };
    wildCard.Append("|All files (*.*)|*.*");

    wxFileDialog openFileDialog(this, wxT("Open Python 3 file"), "", "", wildCard, wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_CHANGE_DIR);
    if (openFileDialog.ShowModal() != wxID_CANCEL)
    {
        auto pyEditor = dynamic_cast<PyEditor *>(wxAuiMgr_.GetPane(pyEditorName_).window);
        if (pyEditor)
        {
            pyEditor->LoadPyFile(openFileDialog.GetPath());
            wxAuiMgr_.GetPane(pyEditorName_).Caption(wxString("Script Editor - ") + openFileDialog.GetFilename());
        }
    }
}

void RootFrame::OnAddStations(const SPModelNodeVector &stations)
{
    auto stationNB = GetStationNotebook();
    if (stationNB && !stations.empty())
    {
        stationNB->Freeze();
        for (const auto &s : stations)
        {
            auto station = std::dynamic_pointer_cast<StationNode>(s);
            if (station)
            {
                wxString uuidName = station->GetUUIDTag();
                wxSize size = wxDefaultSize;
                auto imgPanel = new CVImagePanel(stationNB, GetNextCVStationWinName(), uuidName, wxDefaultSize);
                auto initImg = station->GetImage();
                imgPanel->SetImage(initImg);
                ConnectCanvas(imgPanel);
                stationNB->AddPage(imgPanel, station->GetTitle(), true);
                if (!initImg.empty())
                {
                    imgPanel->BufferImage(station->GetTitle().ToStdString());
                }
            }
        }
        stationNB->Thaw();
    }
}

void RootFrame::OnDeleteStations(const SPModelNodeVector &stations)
{
    auto stationNB = GetStationNotebook();
    if (!stations.empty() && stationNB)
    {
        stationNB->Freeze();
        for (const auto &station : stations)
        {
            wxString uuidName = station->GetUUIDTag();
            auto cPages = stationNB->GetPageCount();
            wxWindow *stationWnd = nullptr;
            decltype(cPages) stationPageIdx = 0;
            for (decltype(cPages) idx = 0; idx<cPages; ++idx)
            {
                auto wnd = stationNB->GetPage(idx);
                if (wnd && uuidName == wnd->GetName())
                {
                    stationWnd = wnd;
                    stationPageIdx = idx;
                    break;
                }
            }

            if (stationWnd)
            {
                stationNB->DeletePage(stationPageIdx);
            }
        }
        stationNB->Thaw();
    }
}

void RootFrame::OnAddGeoms(const SPModelNodeVector &geoms)
{
    UpdateGeoms(&CairoCanvas::DrawDrawables, geoms);
}

void RootFrame::OnDeleteGeoms(const SPModelNodeVector &geoms)
{
    UpdateGeoms(&CairoCanvas::EraseDrawables, geoms);
}

void RootFrame::OnDrawableShapeChange(const SPDrawableNodeVector &drawables, const Geom::OptRect &rect)
{
    if (!drawables.empty())
    {
        auto station = drawables.front()->GetParent();
        if (station)
        {
            if (std::all_of(drawables.cbegin(), drawables.cend(), [&station](const SPModelNode &mn) { return mn->GetParent() == station; }))
            {
                CairoCanvas *cav = FindCanvasByUUID(station->GetUUIDTag());
                if (cav)
                {
                    Geom::OptRect ivalRect;
                    ivalRect.unionWith(rect);
                    for (const auto &drawable : drawables)
                    {
                        Geom::PathVector pv;
                        drawable->BuildPath(pv);
                        ivalRect.unionWith(pv.boundsFast());
                    }

                    cav->DrawPathVector(Geom::PathVector(), ivalRect);
                }
            }
        }
    }
}

void RootFrame::OnGlowGeom(const SPModelNode &geom)
{
    auto drawable = std::dynamic_pointer_cast<DrawableNode>(geom);
    auto station  = std::dynamic_pointer_cast<StationNode>(geom->GetParent());
    if (drawable && station)
    {
        auto imgPanel = FindImagePanelByStation(station);
        if (imgPanel)
        {
            drawable->HighlightFace();
            imgPanel->GetCanvas()->HighlightDrawable(drawable);
        }
    }
}

void RootFrame::OnDimGeom(const SPModelNode &geom)
{
    auto drawable = std::dynamic_pointer_cast<DrawableNode>(geom);
    auto station = std::dynamic_pointer_cast<StationNode>(geom->GetParent());
    if (drawable && station)
    {
        auto imgPanel = FindImagePanelByStation(station);
        if (imgPanel)
        {
            drawable->ClearHighlight();
            imgPanel->GetCanvas()->DimDrawable(drawable);
        }
    }
}

void RootFrame::OnNewProject(ModelEvent& e)
{
    auto projModel = dynamic_cast<const ProjTreeModel *>(e.GetModel());
    if (projModel)
    {
        auto stationNB = GetStationNotebook();
        if (stationNB)
        {
            stationNB->Freeze();
            stationNB->DeleteAllPages();
            stationNB->Thaw();
        }
    }
    e.Skip();
}

void RootFrame::OnLoadProject(ModelEvent& e)
{
    auto projModel = dynamic_cast<const ProjTreeModel *>(e.GetModel());
    if (projModel)
    {
        auto stationNB = GetStationNotebook();
        if (stationNB)
        {
            stationNB->Freeze();
            stationNB->DeleteAllPages();
            const auto &stations = projModel->GetAllStations();
            for (const auto &station : stations)
            {
                if (station)
                {
                    wxString uuidName = station->GetUUIDTag();
                    wxSize size = wxDefaultSize;
                    auto initImg = station->GetImage();
                    if (!initImg.empty())
                    {
                        size = wxSize(initImg.cols, initImg.rows);
                    }
                    auto imgPanel = new CVImagePanel(stationNB, GetNextCVStationWinName(), uuidName, size);
                    ConnectCanvas(imgPanel);
                    imgPanel->SetImage(initImg);
                    stationNB->AddPage(imgPanel, station->GetTitle(), true);
                    if (!initImg.empty())
                    {
                        imgPanel->BufferImage(station->GetTitle().ToStdString());
                    }
                }
            }
            stationNB->Thaw();
        }
    }
    e.Skip();
}

void RootFrame::OnViewMainTool(wxCommandEvent& e)
{
    auto &pane = wxAuiMgr_.GetPane(mainToolPanelName_);
    bool v = pane.IsShown();
    pane.Show(!v);
    wxAuiMgr_.Update();
}

void RootFrame::OnViewImage(wxCommandEvent& e)
{
    auto &pane = wxAuiMgr_.GetPane(stationNotebookName_);
    bool v = pane.IsShown();
    pane.Show(!v);
    wxAuiMgr_.Update();
}

void RootFrame::OnViewDefaultLayout(wxCommandEvent& e)
{
    wxAuiMgr_.LoadPerspective(initialPerspective_);
    wxAuiMgr_.Update();
}

void RootFrame::OnUpdateUI(wxUpdateUIEvent& e)
{
    for (const auto &wItem : widgets_)
    {
        switch (wItem.first)
        {
        case spamID_VIEW_IMAGE:
            gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(wItem.second), TRUE);
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wItem.second), wxAuiMgr_.GetPane(stationNotebookName_).IsShown());
            gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(wItem.second), FALSE);
            break;
        case spamID_VIEW_PROJECT:
            gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(wItem.second), TRUE);
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wItem.second), wxAuiMgr_.GetPane(projPanelName_).IsShown());
            gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(wItem.second), FALSE);
            break;
        case spamID_VIEW_LOG:
            gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(wItem.second), TRUE);
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wItem.second), wxAuiMgr_.GetPane(logPanelName_).IsShown());
            gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(wItem.second), FALSE);
            break;
        case spamID_VIEW_CONSOLE:
            gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(wItem.second), TRUE);
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wItem.second), wxAuiMgr_.GetPane(consolePanelName_).IsShown());
            gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(wItem.second), FALSE);
            break;
        case spamID_VIEW_PYEDITOR:
            gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(wItem.second), TRUE);
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wItem.second), wxAuiMgr_.GetPane(pyEditorName_).IsShown());
            gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(wItem.second), FALSE);
            break;
        case spamID_VIEW_IMAGES_ZONE:
            gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(wItem.second), TRUE);
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wItem.second), wxAuiMgr_.GetPane(imagesZonePanelName_).IsShown());
            gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(wItem.second), FALSE);
            break;
        case spamID_VIEW_TOOLBOX_BAR:
            gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(wItem.second), TRUE);
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wItem.second), wxAuiMgr_.GetPane(toolBoxBarName_).IsShown());
            gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(wItem.second), FALSE);
            break;
        case wxID_UNDO:
            gtk_widget_set_sensitive(wItem.second, SpamUndoRedo::IsUndoable());
            break;
        case wxID_REDO:
            gtk_widget_set_sensitive(wItem.second, SpamUndoRedo::IsRedoable());
            break;
        case kSpamID_PY3_SCRIPT_PLAY:
            gtk_widget_set_sensitive(wItem.second, wxAuiMgr_.GetPane(pyEditorName_).IsShown());
            break;
        default:
            break;
        }
    }
}

void RootFrame::OnStationActivated(wxDataViewEvent &e)
{
    auto node = static_cast<ModelNode*>(e.GetItem().GetID());
    auto sNode = dynamic_cast<const StationNode*>(node);
    auto stationNB = GetStationNotebook();
    if (sNode && stationNB)
    {
        auto cPages = stationNB->GetPageCount();
        std::vector<wxString> pageUUIDs(cPages);

        std::size_t nPageIndex = 0;
        std::generate(pageUUIDs.begin(), pageUUIDs.end(), [&]() { return stationNB->GetPage(nPageIndex++)->GetName(); });
        auto fIt = std::find(pageUUIDs.begin(), pageUUIDs.end(), sNode->GetUUIDTag());
        if (fIt != pageUUIDs.end())
        {
            auto stationIndex = std::distance(pageUUIDs.begin(), fIt);
            auto currSel = stationNB->GetSelection();
            if (currSel != stationIndex)
            {
                stationNB->SetSelection(std::distance(pageUUIDs.begin(), fIt));
            } 
        }
        else
        {
            stationNB->Freeze();
            wxSize size = wxDefaultSize;
            auto initImg = sNode->GetImage();
            if (!initImg.empty())
            {
                size = wxSize(initImg.cols, initImg.rows);
            }
            wxString uuidName = sNode->GetUUIDTag();
            auto imgPanel = new CVImagePanel(stationNB, GetNextCVStationWinName(), uuidName, wxDefaultSize);
            imgPanel->SetImage(initImg);
            ConnectCanvas(imgPanel);
            stationNB->AddPage(imgPanel, sNode->GetTitle(), true);
            stationNB->Thaw();

            if (!initImg.empty())
            {
                imgPanel->BufferImage(sNode->GetTitle().ToStdString());
            }
        }
    }
}

void RootFrame::OnSetTileLayout(wxCommandEvent& e)
{
    auto stationNB = GetStationNotebook();
    if (stationNB)
    {
        StringDict layout;
        wxString   perspective;
        stationNB->SavePerspective(layout, perspective);
        auto m = GetProjTreeModel();
        if (m)
        {
            auto prj = m->GetProject();
            if (prj)
            {
                prj->SetPerspective(perspective);
                m->SetModified(true);
            }

            for (const auto &l : layout)
            {
                auto s = m->FindStationByUUID(l.first);
                if (s)
                {
                    s->SetTaberName(l.second);
                    m->SetModified(true);
                }
            }
        }
    }
}

void RootFrame::OnTileLayout(wxCommandEvent& e)
{
    auto stationNB = GetStationNotebook();
    if (stationNB)
    {
        StringDict  layout;
        std::string perspective;
        std::set<std::string> allTaberNames;

        auto m = GetProjTreeModel();
        if (m)
        {
            auto prj = m->GetProject();
            if (prj)
            {
                perspective = prj->GetPerspective();
            }

            auto ss = m->GetAllStations();

            for (const auto &s : ss)
            {
                wxString uuidTag   = s->GetUUIDTag();
                std::string taberName = s->GetTaberName();
                if (!taberName.empty())
                {
                    allTaberNames.insert(taberName);
                    layout[uuidTag] = taberName;
                }
            }
        }

        if (!perspective.empty() && !layout.empty())
        {
            auto allPaneNames = GetAllTabPaneNames(perspective);
            auto pred = [&allTaberNames](const std::string &n) { return allTaberNames.cend() != allTaberNames.find(n); };
            if (std::all_of(allPaneNames.cbegin(), allPaneNames.cend(), pred))
            {
                stationNB->Freeze();
                stationNB->LoadPerspective(layout, perspective);
                stationNB->Thaw();
            }
        }
    }
}

void RootFrame::OnStackLayout(wxCommandEvent& e)
{
    auto stationNB = GetStationNotebook();
    if (stationNB)
    {
        stationNB->Freeze();
        stationNB->StackAllPages();
        stationNB->Thaw();
    }
}

void RootFrame::OnDropImage(DropImageEvent& e)
{
    imageFileHistory_.AddFileToHistory(e.GetImageFilePath());
    auto imgPanel = dynamic_cast<CVImagePanel *>(e.GetDropTarget());
    if (imgPanel)
    {
        scale_ = imgPanel->LoadImageFromFile(e.GetImageFilePath());
        auto stationNB = GetStationNotebook();
        if (stationNB)
        {
            auto extCtrl = stationNB->FindExtensionCtrlByPage(imgPanel);
            if (imgPanel->HasImage())
            {
                SetStationImage(imgPanel->GetName(), imgPanel->GetImage());
                EnablePageImageTool(extCtrl, true);
                SyncScale(extCtrl);
            }
            else
            {
                EnablePageImageTool(extCtrl, false);
            }
        }
    }
    e.Skip();
}

void RootFrame::OnTabExtension(wxAuiNotebookEvent& e)
{
    auto extCtrl = dynamic_cast<wxControl *>(e.GetEventObject());
    if (extCtrl)
    {
        auto tbStation = MakeStationToolBar(extCtrl);
        wxSizer * const sizerRoot = new wxBoxSizer(wxHORIZONTAL);
        sizerRoot->Add(tbStation, wxSizerFlags().Border(wxUP | wxDOWN, -1).CenterVertical().Proportion(1))->SetId(kSpamImageToolBar);
        sizerRoot->Add(new wxStaticLine(extCtrl, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), wxSizerFlags().Border(wxUP | wxDOWN, -1).Expand());
        extCtrl->SetSize(tbStation->GetBestSize());
        extCtrl->SetSizer(sizerRoot);
        extCtrl->GetSizer()->SetSizeHints(extCtrl);
    }
}

void RootFrame::OnPageSelect(wxAuiNotebookEvent& e)
{
    wxAuiNotebook* nb = dynamic_cast<wxAuiNotebook *>(e.GetEventObject());
    if (nb)
    {
        auto sel = e.GetSelection();
        if (wxNOT_FOUND != sel)
        {
            auto page = nb->GetPage(sel);
            auto extCtrl = nb->FindExtensionCtrlByPage(page);

            auto tmPanel = dynamic_cast<ThumbnailPanel *>(wxAuiMgr_.GetPane(imagesZonePanelName_).window);
            wxThumbnailCtrl *tmCtrl = nullptr;
            if (tmPanel)
            {
                tmCtrl = tmPanel->GetThumbnailCtrl();
            }

            if (tmCtrl)
            {
                tmCtrl->Freeze();
                tmCtrl->Clear();
            }

            CVImagePanel *imgPage = dynamic_cast<CVImagePanel *>(page);
            if (imgPage && imgPage->HasImage())
            {
                scale_ = imgPage->GetScale();
                EnablePageImageTool(extCtrl, true);
                SyncScale(extCtrl);

                CairoCanvas *cav = imgPage->GetCanvas();
                if (cav)
                {
                    for (const auto &bufItem : cav->GetImageBufferZone())
                    {
                        if (tmCtrl)
                        {
                            tmCtrl->Append(new wxPreloadImageThumbnailItem(bufItem.second.iThumbnail, bufItem.second.iStationUUID, bufItem.second.iName));
                        }
                    }
                }
            }
            else
            {
                EnablePageImageTool(extCtrl, false);
            }

            if (tmCtrl)
            {
                tmCtrl->Thaw();
            }
        }
    }
}

void RootFrame::OnAuiPageClosed(wxAuiManagerEvent& e)
{
    auto paneInfo = e.GetPane();
    if (paneInfo && paneInfo->window)
    {
        int toolPageId = paneInfo->window->GetId();
        if (toolPageId<kSpamID_TOOLPAGE_GUARD && toolPageId>=kSpamID_TOOLPAGE_PROBE)
        {
            wxAuiToolBar* tbBar = dynamic_cast<wxAuiToolBar*>(wxAuiMgr_.GetPane(toolBoxBarName_).window);
            if (tbBar)
            {
                auto numTools = static_cast<int>(tbBar->GetToolCount());
                for (int t = 0; t<numTools; ++t)
                {
                    auto toolItem = tbBar->FindToolByIndex(t);
                    toolItem->SetState(0);
                }
            }
        }

        ToolBox *toolBox = dynamic_cast<ToolBox *>(paneInfo->window);
        if (toolBox)
        {
            toolBox->QuitToolbox();
        }
    }
}

void RootFrame::OnZoom(wxCommandEvent &cmd)
{
    wxVariant *var = dynamic_cast<wxVariant *>(cmd.GetEventUserData());
    if (var)
    {
        wxControl *extCtrl = dynamic_cast<wxControl *>(var->GetWxObjectPtr());
        wxSizerItem* sizerItem = extCtrl->GetSizer()->GetItemById(kSpamImageToolBar);
        double (CVImagePanel::*zoomFun)(bool) = &CVImagePanel::ZoomIn;
        if (sizerItem)
        {
            auto tb = dynamic_cast<wxToolBar *>(sizerItem->GetWindow());
            if (tb)
            {
                int tPos = tb->GetToolPos(kSpamID_ZOOM);
                if (wxNOT_FOUND != tPos)
                {
                    wxMenu *ddMenu = tb->GetToolByPos(tPos)->GetDropdownMenu();
                    for (const wxMenuItem *mItem : ddMenu->GetMenuItems())
                    {
                        if (mItem->IsChecked()) {
                            switch (mItem->GetId())
                            {
                            case kSpamID_ZOOM_OUT: zoomFun = &CVImagePanel::ZoomOut; break;
                            case kSpamID_ZOOM_IN: zoomFun = &CVImagePanel::ZoomIn; break;
                            case kSpamID_ZOOM_EXTENT: zoomFun = &CVImagePanel::ZoomExtent; break;
                            case kSpamID_ZOOM_ORIGINAL: zoomFun = &CVImagePanel::ZoomOriginal; break;
                            case kSpamID_ZOOM_HALF: zoomFun = &CVImagePanel::ZoomHalf; break;
                            case kSpamID_ZOOM_DOUBLE: zoomFun = &CVImagePanel::ZoomDouble; break;
                            default: break;
                            }
                            break;
                        }
                    }
                }
            }
        }

        Zoom(zoomFun, cmd, false);
    }
}

void RootFrame::OnZoomIn(wxCommandEvent &cmd)
{
    Zoom(&CVImagePanel::ZoomIn, cmd);
}

void RootFrame::OnZoomOut(wxCommandEvent &cmd)
{
    Zoom(&CVImagePanel::ZoomOut, cmd);
}

void RootFrame::OnZoomExtent(wxCommandEvent &cmd)
{
    Zoom(&CVImagePanel::ZoomExtent, cmd);
}

void RootFrame::OnZoomOriginal(wxCommandEvent &cmd)
{
    Zoom(&CVImagePanel::ZoomOriginal, cmd);
}

void RootFrame::OnZoomHalf(wxCommandEvent &cmd)
{
    Zoom(&CVImagePanel::ZoomHalf, cmd);
}

void RootFrame::OnZoomDouble(wxCommandEvent &cmd)
{
    Zoom(&CVImagePanel::ZoomDouble, cmd);
}

void RootFrame::OnEnterScale(wxCommandEvent& e)
{
    wxVariant *var = dynamic_cast<wxVariant *>(e.GetEventUserData());
    if (var)
    {
        wxControl *extCtrl = dynamic_cast<wxControl *>(var->GetWxObjectPtr());
        wxAuiNotebook *nb = GetStationNotebook();
        if (extCtrl && nb)
        {
            wxSizerItem* sizerItem = extCtrl->GetSizer()->GetItemById(kSpamImageToolBar);
            if (sizerItem)
            {
                auto tb = dynamic_cast<wxToolBar *>(sizerItem->GetWindow());
                if (tb)
                {
                    if (!tb->TransferDataFromWindow())
                    {
                        tb->TransferDataToWindow();
                    }
                }
            }

            wxWindow* page = nb->FindPageByExtensionCtrl(extCtrl);
            CVImagePanel *imgPanelPage = dynamic_cast<CVImagePanel *>(page);
            if (imgPanelPage)
            {
                sizerItem = imgPanelPage->GetSizer()->GetItemById(kSpamImageCanvas);
                if (sizerItem)
                {
                    auto cvWidget = dynamic_cast<CairoCanvas *>(sizerItem->GetWindow());
                    if (cvWidget)
                    {
                        if (!cvWidget->HasFocus())
                        {
                            cvWidget->SetFocus();
                        }

                        cvWidget->ScaleImage(scale_ / 100);
                    }
                }
            }
        }
    }
}

void RootFrame::OnSelectScale(wxCommandEvent &e)
{
    OnEnterScale(e);
}

void RootFrame::OnToolboxInfo(wxCommandEvent& e)
{
    ClickToolbox(e, kSpam_TOOLBOX_PROBE);
}

void RootFrame::OnToolboxGeom(wxCommandEvent& e)
{
    ClickToolbox(e, kSpam_TOOLBOX_GEOM);
}

void RootFrame::OnToolboxProc(wxCommandEvent& e)
{
    ClickToolbox(e, kSpam_TOOLBOX_PROC);
}

void RootFrame::OnToolboxMatch(wxCommandEvent& e)
{
    ClickToolbox(e, kSpam_TOOLBOX_MATCH);
}

void RootFrame::OnToolboxStyle(wxCommandEvent& e)
{
    ClickToolbox(e, kSpam_TOOLBOX_STYLE);
}

void RootFrame::OnToolboxImgFlow(wxCommandEvent& e)
{
    ClickToolbox(e, kSpam_TOOLBOX_IMGFLOW);
}

void RootFrame::OnSelectEntity(const SPDrawableNodeVector &des)
{
    if (1==des.size())
    {
        SetBitmapStatus(StatusIconType::kSIT_NONE, wxString(wxT("Selected: ") + des.front()->GetTitle()));
    }
    else
    {
        SetBitmapStatus(StatusIconType::kSIT_NONE, wxString::Format(wxT("Selected: %d entities"), static_cast<int>(des.size())));
    }
}

void RootFrame::OnImageBufferItemAdd(const ImageBufferItem &ibi)
{
    auto tmPanel = dynamic_cast<ThumbnailPanel *>(wxAuiMgr_.GetPane(imagesZonePanelName_).window);
    if (tmPanel)
    {
        wxThumbnailCtrl *tmCtrl = tmPanel->GetThumbnailCtrl();
        if (tmCtrl)
        {
            tmCtrl->Append(new wxPreloadImageThumbnailItem(ibi.iThumbnail, ibi.iStationUUID, ibi.iName));
        }
    }
}

void RootFrame::OnImageBufferItemUpdate(const ImageBufferItem &ibi)
{
    auto tmPanel = dynamic_cast<ThumbnailPanel *>(wxAuiMgr_.GetPane(imagesZonePanelName_).window);
    if (tmPanel)
    {
        wxThumbnailCtrl *tmCtrl = tmPanel->GetThumbnailCtrl();
        if (tmCtrl)
        {
            int iPos = tmCtrl->FindItemForFilename(ibi.iName);
            if (iPos>=0)
            {
                wxThumbnailItem *tItem = tmCtrl->GetItem(iPos);
                if (tItem)
                {
                    wxPreloadImageThumbnailItem *tPreloadItem = dynamic_cast<wxPreloadImageThumbnailItem *>(tItem);
                    tPreloadItem->SetThumbnailBitmap(ibi.iThumbnail);
                    wxRect rect;
                    tmCtrl->GetItemRect(iPos, rect);
                    tmCtrl->RefreshRect(rect);
                }
            }
        }
    }
}

void RootFrame::file_import_image_cb(GtkWidget *menuitem, gpointer user_data)
{
    wxCommandEvent e;
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    frame->OnLoadImage(e);
}

void RootFrame::file_export_image_cb(GtkWidget *menuitem, gpointer user_data)
{
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
}

void RootFrame::file_import_py3_cb(GtkWidget *menuitem, gpointer user_data)
{
    wxCommandEvent e;
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    frame->OnLoadPy3(e);
}

void RootFrame::file_export_py3_cb(GtkWidget *menuitem, gpointer user_data)
{
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
}

void RootFrame::file_import_model_cb(GtkWidget *menuitem, gpointer user_data)
{
    wxCommandEvent e;
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    frame->OnLoadModel(e);
}

void RootFrame::file_close_model_cb(GtkWidget *menuitem, gpointer user_data)
{
    wxCommandEvent e;
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    frame->OnCloseModel(e);
}

void RootFrame::file_quit_cb(GtkWidget *menuitem, gpointer user_data)
{
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    frame->Close(false);
}

void RootFrame::file_save_cb(GtkWidget *menuitem, gpointer user_data)
{
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    wxString dbPath = SpamConfig::Get<wxString>(CommonDef::GetDBPathCfgPath());
    boost::filesystem::path p(dbPath.ToStdWstring());
    boost::system::error_code ec;
    if (boost::filesystem::exists(p, ec) && boost::filesystem::is_regular_file(p, ec))
    {
        frame->SaveProject(dbPath);
    }
    else
    {
        file_save_as_cb(menuitem, user_data);
    }
}

void RootFrame::file_save_as_cb(GtkWidget *menuitem, gpointer user_data)
{
    wxString wildCard{ "Spam DB files (*.spam_db)|*.spam_db" };
    wildCard.Append("|HDF5 files (*.h5)|*.h5");

    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    wxFileDialog saveFileDialog(wxGetTopLevelParent(frame), _("Open Spam DB file"), "", "", wildCard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() != wxID_CANCEL)
    {
        auto fullPath = saveFileDialog.GetPath();
        frame->SaveProject(fullPath);
    }
}

void RootFrame::undo_cb(GtkWidget *widget, gpointer user_data)
{
    SpamUndoRedo::Undo();
}

void RootFrame::redo_cb(GtkWidget *widget, gpointer user_data)
{
    SpamUndoRedo::Redo();
}

void RootFrame::play_cb(GtkWidget *widget, gpointer user_data)
{
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    if (frame)
    {
        frame->PlayScript();
    }
}

void RootFrame::help_about_cb(GtkWidget *widget, gpointer user_data)
{
#ifdef _DEBUG
    gtk_window_set_interactive_debugging(1);
#endif
}

void RootFrame::view_project_cb(GtkWidget *widget, gpointer user_data)
{
    if (gtk_check_menu_item_get_inconsistent(GTK_CHECK_MENU_ITEM(widget))) return;
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    auto &pane = frame->wxAuiMgr_.GetPane(frame->projPanelName_);
    bool v = pane.IsShown();
    pane.Show(!v);
    frame->wxAuiMgr_.Update();
}

void RootFrame::view_images_cb(GtkWidget *widget, gpointer user_data)
{
    if (gtk_check_menu_item_get_inconsistent(GTK_CHECK_MENU_ITEM(widget))) return;
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    auto &pane = frame->wxAuiMgr_.GetPane(frame->stationNotebookName_);
    bool v = pane.IsShown();
    pane.Show(!v);
    frame->wxAuiMgr_.Update();
}

void RootFrame::view_entity_cb(GtkWidget *widget, gpointer user_data)
{
    if (gtk_check_menu_item_get_inconsistent(GTK_CHECK_MENU_ITEM(widget))) return;
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    auto &pane = frame->wxAuiMgr_.GetPane(frame->imagesZonePanelName_);
    bool v = pane.IsShown();
    pane.Show(!v);
    frame->wxAuiMgr_.Update();
}

void RootFrame::view_toolbox_cb(GtkWidget *widget, gpointer user_data)
{
    if (gtk_check_menu_item_get_inconsistent(GTK_CHECK_MENU_ITEM(widget))) return;
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    auto &pane = frame->wxAuiMgr_.GetPane(frame->toolBoxBarName_);
    bool v = pane.IsShown();
    pane.Show(!v);
    frame->wxAuiMgr_.Update();
}

void RootFrame::view_log_cb(GtkWidget *widget, gpointer user_data)
{
    if (gtk_check_menu_item_get_inconsistent(GTK_CHECK_MENU_ITEM(widget))) return;
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    auto &pane = frame->wxAuiMgr_.GetPane(frame->logPanelName_);
    bool v = pane.IsShown();
    pane.Show(!v);
    frame->wxAuiMgr_.Update();
}

void RootFrame::view_console_cb(GtkWidget *widget, gpointer user_data)
{
    if (gtk_check_menu_item_get_inconsistent(GTK_CHECK_MENU_ITEM(widget))) return;
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    auto &pane = frame->wxAuiMgr_.GetPane(frame->consolePanelName_);
    bool v = pane.IsShown();
    pane.Show(!v);
    frame->wxAuiMgr_.Update();
}

void RootFrame::view_pyeditor_cb(GtkWidget *widget, gpointer user_data)
{
    if (gtk_check_menu_item_get_inconsistent(GTK_CHECK_MENU_ITEM(widget))) return;
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    auto &pane = frame->wxAuiMgr_.GetPane(frame->pyEditorName_);
    bool v = pane.IsShown();
    pane.Show(!v);
    frame->wxAuiMgr_.Update();
}

void RootFrame::mission_image_cb(GtkRadioButton* self, gpointer user_data)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self)))
    {
        RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
        frame->SwitchMission(true);
    }
    else
    {
        RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
        frame->SwitchMission(false);
    }
}

void RootFrame::mission_graphics_cb(GtkRadioButton* self, gpointer user_data)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self)))
    {
        wxLogMessage(wxT("Graphics Mission Enter."));
    }
    else
    {
        wxLogMessage(wxT("Graphics Mission Exit."));
    }
}

void RootFrame::preferences_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    PreferencesDlg dlg(frame);

    boost::property_tree::ptree configTreeBackup(*SpamConfig::GetPropertyTree());
    if (dlg.ShowModal() == wxID_OK)
    {
        auto &pane = frame->wxAuiMgr_.GetPane(frame->pyEditorName_);
        PyEditor *pyEditor = dynamic_cast<PyEditor *>(pane.window);
        if (pyEditor)
        {
            pyEditor->ApplyStyleChange();
        }
    }
    else
    {
        SpamConfig::GetPropertyTree()->swap(configTreeBackup);
    }
}

static void
activate_about(GSimpleAction *action,
    GVariant      *parameter,
    gpointer       user_data)
{
    RootFrame *frame = reinterpret_cast<RootFrame *>(user_data);
    GtkWidget *fontDlg = gtk_font_chooser_dialog_new("Choose Font", GTK_WINDOW(frame->m_widget));
    gtk_widget_show_all(fontDlg);
}

void RootFrame::ReplaceTitleBar(void)
{
    wxGCC_WARNING_SUPPRESS(deprecated-declarations)
    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Spam");
    gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header_bar), FALSE);

    GtkWidget *bbox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_set_border_width(GTK_CONTAINER(bbox), 5);
    gtk_style_context_add_class(gtk_widget_get_style_context(bbox), "linked");

    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_EXPAND);
    gtk_box_set_spacing(GTK_BOX(bbox), 0);

    GtkWidget *buttonImg = gtk_radio_button_new_with_label(nullptr, "Image");
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(buttonImg), false);
    gtk_container_add(GTK_CONTAINER(bbox), buttonImg);

    GtkWidget *buttonGra = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(buttonImg), "Graphics");
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(buttonGra), false);
    gtk_container_add(GTK_CONTAINER(bbox), buttonGra);
    gtk_header_bar_set_custom_title(GTK_HEADER_BAR(header_bar), bbox);

    GActionEntry win_entries[] = {
        { "about", activate_about, NULL, NULL, NULL },
        { "main", NULL, "s", "'pizza'", NULL },
        { "wine", NULL, NULL, "false", NULL },
        { "beer", NULL, NULL, "false", NULL },
        { "water", NULL, NULL, "true", NULL },
        { "preferences", preferences_cb, NULL, NULL, NULL },
        { "pay", NULL, "s", NULL, NULL }
    };

    GSimpleActionGroup *win_actions = g_simple_action_group_new();
    g_action_map_add_action_entries(G_ACTION_MAP(win_actions), win_entries, G_N_ELEMENTS(win_entries), this);
    gtk_widget_insert_action_group(m_widget, "RootFrame", G_ACTION_GROUP(win_actions));

    GtkBuilder *builder = gtk_builder_new_from_resource("/org/mvlab/spam/menus.ui");
    GMenuModel *titleBarGear = G_MENU_MODEL(gtk_builder_get_object(builder, "title_bar_gear_menu"));

    GtkWidget *gearMenu = gtk_menu_button_new();
    GtkWidget * gearIcon = gtk_image_new_from_icon_name("emblem-system-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_menu_button_set_direction(GTK_MENU_BUTTON(gearMenu), GTK_ARROW_NONE);
    gtk_menu_button_set_use_popover(GTK_MENU_BUTTON(gearMenu), TRUE);
    gtk_button_set_image(GTK_BUTTON(gearMenu), gearIcon);
    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(gearMenu), titleBarGear);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), gearMenu);

    g_object_unref(win_actions);
    g_object_unref(builder);

    GtkWidget *toolbar = gtk_toolbar_new();
    gtk_toolbar_set_icon_size(GTK_TOOLBAR(toolbar), GTK_ICON_SIZE_BUTTON);
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
    GtkToolItem *udTb = gtk_tool_button_new_from_stock(GTK_STOCK_UNDO);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), udTb, -1);
    GtkToolItem *rdTb = gtk_tool_button_new_from_stock(GTK_STOCK_REDO);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), rdTb, -1);
    GtkToolItem *sep1 = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep1, -1);
    GtkToolItem *newTb = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), newTb, -1);
    GtkToolItem *openTb = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), openTb, -1);
    GtkToolItem *saveTb = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), saveTb, -1);
    GtkToolItem *sep2 = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep2, -1);
    GtkToolItem *playTb = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), playTb, -1);

    gtk_widget_set_tooltip_text(GTK_WIDGET(newTb), "New a Project");
    gtk_widget_set_tooltip_text(GTK_WIDGET(openTb), "Open Project");
    gtk_widget_set_tooltip_text(GTK_WIDGET(saveTb), "Save Project");
    gtk_widget_set_tooltip_text(GTK_WIDGET(playTb), "Play Current Python 3 script");

    GtkWidget *menubar = gtk_menu_bar_new();
    GtkWidget *fileMenu = gtk_menu_new();
    GtkWidget *viewMenu = gtk_menu_new();
    GtkWidget *helpMenu = gtk_menu_new();
    GtkAccelGroup *accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(m_widget), accel_group);

    GtkWidget *fileMi = gtk_menu_item_new_with_mnemonic("_File");
    GtkWidget *newMi = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW, accel_group);
    GtkWidget *openMi = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, accel_group);
    GtkWidget *impoMi = gtk_image_menu_item_new_with_mnemonic("_Import Image...");
    GtkWidget *expoMi = gtk_image_menu_item_new_with_mnemonic("_Export Image...");
    GtkWidget *imPyMi = gtk_image_menu_item_new_with_mnemonic("_Open Python 3 Script...");
    GtkWidget *exPyMi = gtk_image_menu_item_new_with_mnemonic("_Save Python 3 Script...");
    GtkWidget *modlMi = gtk_image_menu_item_new_with_mnemonic("Import _Model...");
    GtkWidget *closMi = gtk_image_menu_item_new_with_mnemonic("_Close Model...");
    GtkWidget *saveMi = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE, accel_group);
    GtkWidget *saveAsMi = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE_AS, nullptr);
    GtkWidget *quitMi = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accel_group);
    GtkWidget *viewMi = gtk_menu_item_new_with_label("View");
    GtkWidget *projMi = gtk_check_menu_item_new_with_label("Show Project Browser");
    GtkWidget *imagMi = gtk_check_menu_item_new_with_label("Show Image Windows");
    GtkWidget *entZMi = gtk_check_menu_item_new_with_label("Show Entity Zone");
    GtkWidget *toolMi = gtk_check_menu_item_new_with_label("Show Toolbox");
    GtkWidget *logWMi = gtk_check_menu_item_new_with_label("Show Log Window");
    GtkWidget *conWMi = gtk_check_menu_item_new_with_label("Show Console Window");
    GtkWidget *pyEiMi = gtk_check_menu_item_new_with_label("Script Editor");
    GtkWidget *helpMi = gtk_menu_item_new_with_label("Help");
    GtkWidget *abouMi = gtk_menu_item_new_with_label("About");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMi), fileMenu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(viewMi), viewMenu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(helpMi), helpMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), newMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), openMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), impoMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), expoMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), modlMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), closMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), imPyMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), exPyMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), saveMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), saveAsMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), quitMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu), projMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu), imagMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu), entZMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu), toolMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu), logWMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu), conWMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu), pyEiMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(helpMenu), abouMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), viewMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), helpMi);

    gtk_widget_add_accelerator(newMi, "activate", accel_group, GDK_KEY_N, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(openMi, "activate", accel_group, GDK_KEY_O, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(impoMi, "activate", accel_group, GDK_KEY_I, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(modlMi, "activate", accel_group, GDK_KEY_M, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(closMi, "activate", accel_group, GDK_KEY_C, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(expoMi, "activate", accel_group, GDK_KEY_E, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(saveMi, "activate", accel_group, GDK_KEY_S, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(quitMi, "activate", accel_group, GDK_KEY_Q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(newMi), TRUE);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(openMi), TRUE);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(saveMi), TRUE);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(saveAsMi), TRUE);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(quitMi), TRUE);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    gtk_container_add(GTK_CONTAINER(box), menubar);
    gtk_container_add(GTK_CONTAINER(box), toolbar);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), box);

    wxString tbCss(".TitleToolBar { background: alpha (@base_color, 0.0); border-color: transparent; }");
    wxScopedCharBuffer uft8Buffer = tbCss.ToUTF8();
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(css_provider), uft8Buffer.data(), uft8Buffer.length(), nullptr);
    GtkStyleContext *context = gtk_widget_get_style_context(toolbar);
    gtk_style_context_add_class(context, "TitleToolBar");
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(css_provider);

    gtk_window_set_titlebar(GTK_WINDOW(m_widget), header_bar);

    g_signal_connect(G_OBJECT(impoMi), "activate", G_CALLBACK(file_import_image_cb), this);
    g_signal_connect(G_OBJECT(expoMi), "activate", G_CALLBACK(file_export_image_cb), this);
    g_signal_connect(G_OBJECT(modlMi), "activate", G_CALLBACK(file_import_model_cb), this);
    g_signal_connect(G_OBJECT(closMi), "activate", G_CALLBACK(file_close_model_cb), this);
    g_signal_connect(G_OBJECT(imPyMi), "activate", G_CALLBACK(file_import_py3_cb), this);
    g_signal_connect(G_OBJECT(exPyMi), "activate", G_CALLBACK(file_export_py3_cb), this);
    g_signal_connect(G_OBJECT(quitMi), "activate", G_CALLBACK(file_quit_cb), this);
    g_signal_connect(G_OBJECT(saveMi), "activate", G_CALLBACK(file_save_cb), this);
    g_signal_connect(G_OBJECT(saveAsMi), "activate", G_CALLBACK(file_save_as_cb), this);
    g_signal_connect(G_OBJECT(projMi), "toggled", G_CALLBACK(view_project_cb), this);
    g_signal_connect(G_OBJECT(imagMi), "toggled", G_CALLBACK(view_images_cb), this);
    g_signal_connect(G_OBJECT(entZMi), "toggled", G_CALLBACK(view_entity_cb), this);
    g_signal_connect(G_OBJECT(toolMi), "toggled", G_CALLBACK(view_toolbox_cb), this);
    g_signal_connect(G_OBJECT(logWMi), "toggled", G_CALLBACK(view_log_cb), this);
    g_signal_connect(G_OBJECT(conWMi), "toggled", G_CALLBACK(view_console_cb), this);
    g_signal_connect(G_OBJECT(pyEiMi), "toggled", G_CALLBACK(view_pyeditor_cb), this);
    g_signal_connect(G_OBJECT(abouMi), "activate", G_CALLBACK(help_about_cb), this);
    g_signal_connect(G_OBJECT(buttonImg), "toggled", G_CALLBACK(mission_image_cb), this);
    g_signal_connect(G_OBJECT(buttonGra), "toggled", G_CALLBACK(mission_graphics_cb), this);

    g_signal_connect(G_OBJECT(udTb), "clicked", G_CALLBACK(undo_cb), this);
    g_signal_connect(G_OBJECT(rdTb), "clicked", G_CALLBACK(redo_cb), this);
    g_signal_connect(G_OBJECT(playTb), "clicked", G_CALLBACK(play_cb), this);

    widgets_.reserve(64);
    widgets_.emplace_back(wxID_UNDO, GTK_WIDGET(udTb));
    widgets_.emplace_back(wxID_REDO, GTK_WIDGET(rdTb));
    widgets_.emplace_back(kSpamID_PY3_SCRIPT_PLAY, GTK_WIDGET(playTb));
    widgets_.emplace_back(spamID_VIEW_PROJECT, projMi);
    widgets_.emplace_back(spamID_VIEW_IMAGE, imagMi);
    widgets_.emplace_back(spamID_VIEW_LOG, logWMi);
    widgets_.emplace_back(spamID_VIEW_CONSOLE, conWMi);
    widgets_.emplace_back(spamID_VIEW_PYEDITOR, pyEiMi);
    widgets_.emplace_back(spamID_VIEW_IMAGES_ZONE, entZMi);
    widgets_.emplace_back(spamID_VIEW_TOOLBOX_BAR, toolMi);
    wxGCC_WARNING_RESTORE(deprecated-declarations)
}

void RootFrame::SetGTKGlobalStyle(void)
{
    GError *error = 0;
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_resource(GTK_CSS_PROVIDER(provider), "/org/mvlab/spam/res/css/spam.css");
    g_object_unref(provider);
}

wxAuiNotebook *RootFrame::CreateStationNotebook()
{
    long style = wxAUI_NB_BOTTOM | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_WINDOWLIST_BUTTON | wxNO_BORDER;
    wxAuiNotebook* nb = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);
    nb->Bind(wxEVT_AUINOTEBOOK_TAB_EXTENSION, &RootFrame::OnTabExtension, this, wxID_ANY);
    nb->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &RootFrame::OnPageSelect, this, wxID_ANY);
    return nb;
}

std::string RootFrame::GetNextCVStationWinName()
{
    auto cStr = std::to_string(cCVStation_++);
    return std::string("cvStationWin") + cStr;
}

std::vector<std::string> RootFrame::GetAllTabPaneNames(const std::string &perspective)
{
    std::vector<std::string> paneNames;
    std::vector<std::string> strPanes;
    boost::split(strPanes, perspective, boost::is_any_of("|"), boost::token_compress_on);
    for (const auto &strPane : strPanes)
    {
        std::vector<std::string> strAttrs;
        boost::split(strAttrs, strPane, boost::is_any_of(";"), boost::token_compress_on);
        for (const auto &strAttr : strAttrs)
        {
            std::vector<std::string> strValPairs;
            boost::split(strValPairs, strAttr, boost::is_any_of("="), boost::token_compress_on);
            if (2== strValPairs.size() && ("name"==strValPairs[0]))
            {
                if ("dummy" != strValPairs[1])
                {
                    paneNames.push_back(strValPairs[1]);
                }
            }
        }
    }

    return paneNames;
}

wxToolBar *RootFrame::MakeStationToolBar(wxWindow *parent)
{
    constexpr int sz = 20;
    const SpamIconPurpose ip = kICON_PURPOSE_TOOLBAR;
    wxToolBar *tb = new wxToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);
    tb->SetMargins(wxSize(0, 0));
    tb->SetToolBitmapSize(wxSize(sz, sz));

    wxString choices[] =
    {
        wxT("3.125 %"),
        wxT("6.25 %"),
        wxT("12.5 %"),
        wxT("25 %"),
        wxT("50 %"),
        wxT("100 %"),
        wxT("150 %"),
        wxT("200 %"),
        wxT("400 %"),
        wxT("800 %")
    };

    PercentValidator<double> val(3, &scale_, wxNUM_VAL_NO_TRAILING_ZEROES);
    val.SetRange(0.01, 10000);

    auto choice = new wxComboBox(tb, kSpamID_SCALE_CHOICE, wxEmptyString, wxDefaultPosition, wxSize(120, 20), 10, choices, wxCB_DROPDOWN | wxTE_PROCESS_ENTER, val);
    choice->SetMargins(0, 0);
    choice->SetSelection(5);

    tb->AddControl(choice, wxT("Scale Factor"));
    tb->AddSeparator();
    tb->AddTool(kSpamID_ZOOM, wxT("Zoom Out"), Spam::GetBitmap(ip, bm_ZoomOut), wxNullBitmap, wxITEM_DROPDOWN);

    wxMenu* menu = new wxMenu;
    menu->AppendRadioItem(kSpamID_ZOOM_OUT, wxT("Zoom Out"))->Check(true);
    menu->AppendRadioItem(kSpamID_ZOOM_IN, wxT("Zoom In"));
    menu->AppendRadioItem(kSpamID_ZOOM_EXTENT, wxT("Zoom Extent"));
    menu->AppendRadioItem(kSpamID_ZOOM_ORIGINAL, wxT("Zoom 1:1"));
    menu->AppendRadioItem(kSpamID_ZOOM_HALF, wxT("Zoom Half"));
    menu->AppendRadioItem(kSpamID_ZOOM_DOUBLE, wxT("Zoom Double"));
    tb->SetDropdownMenu(kSpamID_ZOOM, menu);
    tb->Realize();

    tb->Bind(wxEVT_TOOL,           &RootFrame::OnZoom,         this, kSpamID_ZOOM,          wxID_ANY, new wxVariant(parent, wxT("extCtrl")));
    menu->Bind(wxEVT_TOOL,         &RootFrame::OnZoomOut,      this, kSpamID_ZOOM_OUT,      wxID_ANY, new wxVariant(parent, wxT("extCtrl")));
    menu->Bind(wxEVT_TOOL,         &RootFrame::OnZoomIn,       this, kSpamID_ZOOM_IN,       wxID_ANY, new wxVariant(parent, wxT("extCtrl")));
    menu->Bind(wxEVT_TOOL,         &RootFrame::OnZoomExtent,   this, kSpamID_ZOOM_EXTENT,   wxID_ANY, new wxVariant(parent, wxT("extCtrl")));
    menu->Bind(wxEVT_TOOL,         &RootFrame::OnZoomOriginal, this, kSpamID_ZOOM_ORIGINAL, wxID_ANY, new wxVariant(parent, wxT("extCtrl")));
    menu->Bind(wxEVT_TOOL,         &RootFrame::OnZoomHalf,     this, kSpamID_ZOOM_HALF,     wxID_ANY, new wxVariant(parent, wxT("extCtrl")));
    menu->Bind(wxEVT_TOOL,         &RootFrame::OnZoomDouble,   this, kSpamID_ZOOM_DOUBLE,   wxID_ANY, new wxVariant(parent, wxT("extCtrl")));
    choice->Bind(wxEVT_TEXT_ENTER, &RootFrame::OnEnterScale,   this, kSpamID_SCALE_CHOICE,  wxID_ANY, new wxVariant(parent, wxT("extCtrl")));
    choice->Bind(wxEVT_COMBOBOX,   &RootFrame::OnSelectScale,  this, kSpamID_SCALE_CHOICE,  wxID_ANY, new wxVariant(parent, wxT("extCtrl")));

    tb->EnableTool(kSpamID_SCALE_CHOICE, false);
    tb->EnableTool(kSpamID_ZOOM, false);

    return tb;
}

void RootFrame::Zoom(double (CVImagePanel::*zoomFun)(bool), wxCommandEvent &cmd, bool changeIcon)
{
    wxVariant *var = dynamic_cast<wxVariant *>(cmd.GetEventUserData());
    if (var)
    {
        wxControl *extCtrl = dynamic_cast<wxControl *>(var->GetWxObjectPtr());
        wxSizerItem* sizerItem = extCtrl->GetSizer()->GetItemById(kSpamImageToolBar);
        if (sizerItem && changeIcon)
        {
            auto tb = dynamic_cast<wxToolBar *>(sizerItem->GetWindow());
            if (tb)
            {
                int tPos = tb->GetToolPos(kSpamID_ZOOM);
                if (wxNOT_FOUND != tPos)
                {
                    const SpamIconPurpose ip = kICON_PURPOSE_TOOLBAR;
                    wxMenu *ddMenu = tb->GetToolByPos(tPos)->GetDropdownMenu();
                    for (const wxMenuItem *mItem : ddMenu->GetMenuItems())
                    {
                        if (mItem->IsChecked()) {
                            switch (mItem->GetId())
                            {
                            case kSpamID_ZOOM_OUT:
                                tb->SetToolNormalBitmap(kSpamID_ZOOM, Spam::GetBitmap(ip, bm_ZoomOut));
                                break;
                            case kSpamID_ZOOM_IN:
                                tb->SetToolNormalBitmap(kSpamID_ZOOM, Spam::GetBitmap(ip, bm_ZoomIn));
                                break;
                            case kSpamID_ZOOM_EXTENT:
                                tb->SetToolNormalBitmap(kSpamID_ZOOM, Spam::GetBitmap(ip, bm_ZoomExtent));
                                break;
                            case kSpamID_ZOOM_ORIGINAL:
                                tb->SetToolNormalBitmap(kSpamID_ZOOM, Spam::GetBitmap(ip, bm_ZoomOriginal));
                                break;
                            case kSpamID_ZOOM_HALF:
                                tb->SetToolNormalBitmap(kSpamID_ZOOM, Spam::GetBitmap(ip, bm_ZoomHalf));
                                break;
                            case kSpamID_ZOOM_DOUBLE:
                                tb->SetToolNormalBitmap(kSpamID_ZOOM, Spam::GetBitmap(ip, bm_ZoomDouble));
                                break;
                            default: break;
                            }
                            break;
                        }
                    }
                }
            }
        }

        wxAuiNotebook *nb = GetStationNotebook();
        if (extCtrl && nb)
        {
            wxWindow* page = nb->FindPageByExtensionCtrl(extCtrl);
            CVImagePanel *imgPanelPage = dynamic_cast<CVImagePanel *>(page);
            if (imgPanelPage)
            {
                scale_ = (imgPanelPage->*zoomFun)(false);
                SyncScale(extCtrl);
            }
        }
    }
}

void RootFrame::UpdateGeoms(void (CairoCanvas::*updateFun)(const SPDrawableNodeVector &), const SPModelNodeVector &geoms)
{
    auto stationNB = GetStationNotebook();
    std::map<wxString, std::pair<CairoCanvas *, SPDrawableNodeVector>> canvs;

    for (const auto &geom : geoms)
    {
        if (geom && stationNB)
        {
            auto station = geom->GetParent();
            if (station)
            {
                wxString uuidName = station->GetUUIDTag();
                auto cPages = stationNB->GetPageCount();
                wxWindow *stationWnd = nullptr;
                decltype(cPages) stationPageIdx = 0;
                for (decltype(cPages) idx = 0; idx<cPages; ++idx)
                {
                    auto wnd = stationNB->GetPage(idx);
                    if (wnd && uuidName == wnd->GetName())
                    {
                        stationWnd = wnd;
                        stationPageIdx = idx;
                        break;
                    }
                }

                if (stationWnd)
                {
                    CVImagePanel *imgPanel = dynamic_cast<CVImagePanel *>(stationWnd);
                    if (imgPanel && imgPanel->GetCanvas())
                    {
                        auto &canv = canvs[uuidName];
                        canv.first = imgPanel->GetCanvas();
                        canv.second.push_back(std::dynamic_pointer_cast<DrawableNode>(geom));
                    }
                }
            }
        }
    }

    for (auto &canv : canvs)
    {
        (canv.second.first->*updateFun)(canv.second.second);
    }
}

void RootFrame::SyncScale(wxControl *extCtrl)
{
    if (extCtrl)
    {
        wxSizerItem* sizerItem = extCtrl->GetSizer()->GetItemById(kSpamImageToolBar);
        if (sizerItem)
        {
            auto tb = dynamic_cast<wxToolBar *>(sizerItem->GetWindow());
            if (tb)
            {
                tb->TransferDataToWindow();
            }
        }
    }
}

void RootFrame::EnablePageImageTool(wxControl *extCtrl, bool bEnable)
{
    if (extCtrl)
    {
        wxSizerItem* sizerItem = extCtrl->GetSizer()->GetItemById(kSpamImageToolBar);
        if (sizerItem)
        {
            auto tb = dynamic_cast<wxToolBar *>(sizerItem->GetWindow());
            if (tb)
            {
                tb->EnableTool(kSpamID_SCALE_CHOICE, bEnable);
                tb->EnableTool(kSpamID_ZOOM, bEnable);
            }
        }
    }
}

void RootFrame::SetStationImage(const wxString &uuidStation, const cv::Mat &img)
{
    auto m = GetProjTreeModel();
    if (m)
    {
        auto s = m->FindStationByUUID(uuidStation);
        if (s)
        {
            s->SetImage(img);
            m->SetModified(true);
        }
    }
}

wxPoint RootFrame::GetToolboxPopupPosition(const wxSize &popUpSize)
{
    wxPoint pos(0, 0);
    auto nb = GetStationNotebook();
    if (nb)
    {
        auto nbRect = nb->GetScreenRect();
        auto borderSize = nb->GetWindowBorderSize();
        nbRect = nbRect.Deflate(borderSize);

        pos.y = nbRect.GetTop() + 3;
        pos.x = nbRect.GetRight() - 3 - popUpSize.GetWidth();
    }

    return pos;
}

void RootFrame::ToggleToolboxPane(const int boxShow)
{
    for (const auto &label : toolBoxLabels)
    {
        auto &tbPanelInfo = wxAuiMgr_.GetPane(label);
        if (tbPanelInfo.IsShown())
        {
            tbPanelInfo.Show(false);

            ToolBox *toolBox = dynamic_cast<ToolBox *>(tbPanelInfo.window);
            if (toolBox)
            {
                toolBox->QuitToolbox();
            }
        }
    }

    if (boxShow<kSpam_TOOLBOX_GUARD && boxShow>= kSpam_TOOLBOX_PROBE)
    {
        auto &tbPanelInfo = wxAuiMgr_.GetPane(toolBoxLabels[boxShow]);
        if (!tbPanelInfo.IsShown())
        {
            tbPanelInfo.Show(true);

            ToolBox *toolBox = dynamic_cast<ToolBox *>(tbPanelInfo.window);
            if (toolBox)
            {
                toolBox->OpenToolbox();
            }
        }
    }

    wxAuiMgr_.Update();
}

void RootFrame::ConnectCanvas(CVImagePanel *imgPanel)
{
    if (imgPanel)
    {
        CairoCanvas *canv = imgPanel->GetCanvas();
        if (canv)
        {
            canv->sig_Char.connect(std::bind(&Spamer::OnCanvasChar, spamer_.get(), std::placeholders::_1));
            canv->sig_KeyUp.connect(std::bind(&Spamer::OnCanvasKeyUp, spamer_.get(), std::placeholders::_1));
            canv->sig_KeyDown.connect(std::bind(&Spamer::OnCanvasKeyDown, spamer_.get(), std::placeholders::_1));
            canv->sig_MouseMotion.connect(std::bind(&Spamer::OnCanvasMouseMotion, spamer_.get(), std::placeholders::_1));
            canv->sig_LeftMouseUp.connect(std::bind(&Spamer::OnCanvasLeftMouseUp, spamer_.get(), std::placeholders::_1));
            canv->sig_LeftMouseDown.connect(std::bind(&Spamer::OnCanvasLeftMouseDown, spamer_.get(), std::placeholders::_1));
            canv->sig_LeftDClick.connect(std::bind(&Spamer::OnCanvasLeftDClick, spamer_.get(), std::placeholders::_1));
            canv->sig_MiddleDown.connect(std::bind(&Spamer::OnCanvasMiddleDown, spamer_.get(), std::placeholders::_1));
            canv->sig_EnterWindow.connect(std::bind(&Spamer::OnCanvasEnter, spamer_.get(), std::placeholders::_1));
            canv->sig_LeaveWindow.connect(std::bind(&Spamer::OnCanvasLeave, spamer_.get(), std::placeholders::_1));
            canv->sig_ImageBufferItemAdd.connect(std::bind(&RootFrame::OnImageBufferItemAdd, this, std::placeholders::_1));
            canv->sig_ImageBufferItemUpdate.connect(std::bind(&RootFrame::OnImageBufferItemUpdate, this, std::placeholders::_1));
        }
    }
}

void RootFrame::ClickToolbox(wxCommandEvent& e, const int toolIndex)
{
    auto eo = e.GetEventObject();
    wxAuiToolBar* tbBar = dynamic_cast<wxAuiToolBar*>(eo);
    if (tbBar)
    {
        bool isChecked = e.IsChecked();
        ToggleToolboxPane(isChecked ? toolIndex : kSpam_TOOLBOX_GUARD);

        auto thisIdx = tbBar->GetToolIndex(e.GetId());
        auto numTools = static_cast<int>(tbBar->GetToolCount());
        for (int t = 0; t<numTools; ++t)
        {
            if (thisIdx != t)
            {
                auto toolItem = tbBar->FindToolByIndex(t);
                toolItem->SetState(0);
            }
        }
    }
}

CVImagePanel *RootFrame::FindImagePanelByStation(const SPStationNode &station) const
{
    auto stationNB = GetStationNotebook();
    if (stationNB && station)
    {
        wxString uuidName = station->GetUUIDTag();
        auto cPages = stationNB->GetPageCount();
        wxWindow *stationWnd = nullptr;
        for (decltype(cPages) idx = 0; idx<cPages; ++idx)
        {
            auto wnd = stationNB->GetPage(idx);
            if (wnd && uuidName == wnd->GetName())
            {
                stationWnd = wnd;
                break;
            }
        }

        if (stationWnd)
        {
            return dynamic_cast<CVImagePanel *>(stationWnd);
        }
    }

    return nullptr;
}