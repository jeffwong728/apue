#include "maintoolpane.h"
#include "projpanel.h"
#include "rootframe.h"
#include <ui/spam.h>
#include <ui/cmndef.h>
#include <ui/misc/percentvalidator.h>
#include <ui/dlg/openprojdlg.h>
#include <ui/cv/cvimagepanel.h>
#include <ui/cv/cairocanvas.h>
#include <wx/artprov.h>
#include <wx/wrapsizer.h>
#include <wx/toolbook.h>
#include <wx/filepicker.h>
#include <helper/commondef.h>
#include <boost/thread.hpp>
#include <boost/locale.hpp>
#include <helper/h5db.h>

MainToolPane::MainToolPane(wxWindow* parent, wxFileHistory &fh)
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
, toolImageSize_(16, 16)
{
    // Root sizer, vertical
    wxSizer * const sizerRoot = new wxBoxSizer(wxHORIZONTAL);

    // Some toolbars in a wrap sizer
    sizerRoot->Add(MakeMainToolBar(), wxSizerFlags().Expand().Border(wxALL, 0))->SetId(kSpamGlobalMainToolBar);
    sizerRoot->Add(MakeImageToolBar(fh), wxSizerFlags().Expand().Border(wxALL, 0))->SetId(kSpamGlobalImageToolBar);

    // Set sizer for the panel
    SetSizer(sizerRoot);
    GetSizer()->SetSizeHints(this);
    Show();
}

wxToolBar *MainToolPane::MakeMainToolBar()
{
    wxToolBar *tb = new wxToolBar(this, kSpamGlobalMainToolBar, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
    tb->SetToolBitmapSize(toolImageSize_);
    tb->AddTool(wxID_NEW, wxT("New"), wxArtProvider::GetBitmap(wxART_NEW, wxART_TOOLBAR, toolImageSize_));
    tb->AddTool(wxID_OPEN, wxT("Open"), wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR, toolImageSize_));
    tb->AddSeparator();
    tb->AddTool(wxID_SAVE, wxT("Save"), wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR, toolImageSize_));
    tb->AddTool(wxID_SAVEAS, wxT("Save As"), wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_TOOLBAR, toolImageSize_));
    tb->AddSeparator();
    tb->AddTool(wxID_UNDO, wxT("Undo"), wxArtProvider::GetBitmap(wxART_UNDO, wxART_TOOLBAR, toolImageSize_));
    tb->AddTool(wxID_REDO, wxT("Redo"), wxArtProvider::GetBitmap(wxART_REDO, wxART_TOOLBAR, toolImageSize_));
    tb->Realize();

    tb->Bind(wxEVT_TOOL, &MainToolPane::OnNew,    this, wxID_NEW);
    tb->Bind(wxEVT_TOOL, &MainToolPane::OnOpen,   this, wxID_OPEN);
    tb->Bind(wxEVT_TOOL, &MainToolPane::OnSave,   this, wxID_SAVE);
    tb->Bind(wxEVT_TOOL, &MainToolPane::OnSaveAs, this, wxID_SAVEAS);
    tb->Bind(wxEVT_TOOL, &MainToolPane::OnUndo,   this, wxID_UNDO);
    tb->Bind(wxEVT_TOOL, &MainToolPane::OnRedo,   this, wxID_REDO);

    return tb;
}

wxToolBar *MainToolPane::MakeImageToolBar(wxFileHistory &fh)
{
    wxToolBar *tb = new wxToolBar(this, kSpamGlobalImageToolBar, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
    tb->SetToolBitmapSize(toolImageSize_);

    const SpamIconPurpose ip = kICON_PURPOSE_TOOLBAR;
    wxBitmap zoomInBM = Spam::GetBitmap(ip, bm_ZoomIn);
    wxBitmap zoomExBM = Spam::GetBitmap(ip, bm_ZoomExtent);
    wxBitmap zoom11BM = Spam::GetBitmap(ip, bm_ZoomOriginal);
    wxBitmap zoom12BM = Spam::GetBitmap(ip, bm_ZoomHalf);
    wxBitmap zoom21BM = Spam::GetBitmap(ip, bm_ZoomDouble);
    wxBitmap importBM = Spam::GetBitmap(ip, bm_ImageImport);
    wxBitmap exportBM = Spam::GetBitmap(ip, bm_ImageExport);

    wxMenu* fhMenu = new wxMenu;
    fh.UseMenu(fhMenu);
    tb->AddTool(spamID_LOAD_IMAGE, wxT("Import Image"), importBM, wxEmptyString, wxITEM_DROPDOWN);
    tb->SetDropdownMenu(spamID_LOAD_IMAGE, fhMenu);
    tb->AddTool(spamID_EXPORT_IMAGE, wxT("Export Image"), exportBM);
    tb->AddSeparator();

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

    PercentValidator<double> val(3, &gScale_, wxNUM_VAL_NO_TRAILING_ZEROES);
    val.SetRange(0.01, 10000);

    auto choice = new wxComboBox(tb, kSpamID_MAIN_SCALE_CHOICE, wxEmptyString, wxDefaultPosition, wxSize(72, 20), 10, choices, wxCB_DROPDOWN | wxTE_PROCESS_ENTER, val);
    choice->SetSelection(5);
    choice->SetMargins(0, 0);

    tb->AddControl(choice, wxT("Scale Factor"));
    tb->AddSeparator();
    tb->AddTool(kSpamID_MAIN_ZOOM_OUT, wxT("Zoom Out"), Spam::GetBitmap(ip, bm_ZoomOut), wxNullBitmap, wxITEM_DROPDOWN);

    wxMenu* zoomMenu = new wxMenu;
    zoomMenu->AppendCheckItem(kSpamID_MAIN_ZOOM_IN, wxT("Zoom In"))->SetBitmaps(zoomInBM, zoomInBM);
    zoomMenu->AppendCheckItem(kSpamID_MAIN_ZOOM_EXTENT, wxT("Zoom Extent"))->SetBitmaps(zoomExBM, zoomExBM);
    zoomMenu->AppendSeparator();
    zoomMenu->AppendCheckItem(kSpamID_MAIN_ZOOM_ORIGINAL, wxT("Zoom 1:1"))->SetBitmaps(zoom11BM, zoom11BM);
    zoomMenu->AppendCheckItem(kSpamID_MAIN_ZOOM_HALF, wxT("Zoom Half"))->SetBitmaps(zoom12BM, zoom12BM);
    zoomMenu->AppendCheckItem(kSpamID_MAIN_ZOOM_DOUBLE, wxT("Zoom Double"))->SetBitmaps(zoom21BM, zoom21BM);
    tb->SetDropdownMenu(kSpamID_MAIN_ZOOM_OUT, zoomMenu);
    tb->Realize();

    tb->Bind(wxEVT_TOOL,           &MainToolPane::OnAllZoomOut,      this, kSpamID_MAIN_ZOOM_OUT);
    zoomMenu->Bind(wxEVT_TOOL,     &MainToolPane::OnAllZoomIn,       this, kSpamID_MAIN_ZOOM_IN);
    zoomMenu->Bind(wxEVT_TOOL,     &MainToolPane::OnAllZoomExtent,   this, kSpamID_MAIN_ZOOM_EXTENT);
    zoomMenu->Bind(wxEVT_TOOL,     &MainToolPane::OnAllZoomOriginal, this, kSpamID_MAIN_ZOOM_ORIGINAL);
    zoomMenu->Bind(wxEVT_TOOL,     &MainToolPane::OnAllZoomHalf,     this, kSpamID_MAIN_ZOOM_HALF);
    zoomMenu->Bind(wxEVT_TOOL,     &MainToolPane::OnAllZoomDouble,   this, kSpamID_MAIN_ZOOM_DOUBLE);
    choice->Bind(wxEVT_TEXT_ENTER, &MainToolPane::OnAllEnterScale,   this, kSpamID_MAIN_SCALE_CHOICE);
    choice->Bind(wxEVT_COMBOBOX,   &MainToolPane::OnAllSelectScale,  this, kSpamID_MAIN_SCALE_CHOICE);

    return tb;
}

ProjPanel *MainToolPane::GetProjPanel()
{
    RootFrame *rf = dynamic_cast<RootFrame *>(GetParent());
    return rf ? dynamic_cast<ProjPanel *>(rf->GetProjPanel()) : nullptr;
}

void MainToolPane::OnNew(wxCommandEvent &cmd)
{
    bool canNew = AskSaveModifiedProjectFirst();
    if (canNew)
    {
        wxTextEntryDialog dialog(wxGetTopLevelParent(this), wxT("Please enter a new project name"),
            wxT("New Project"), GetNextProjectName(), wxOK | wxCANCEL);

        ProjPanel *projPanel = GetProjPanel();
        if (projPanel && dialog.ShowModal() == wxID_OK)
        {
            wxString dbPath = SpamConfig::Get<wxString>(CommonDef::GetDBPathCfgPath());
            const auto &projNames = H5DB::GetSpamProjects(dbPath);
            const auto &newProjName = dialog.GetValue();

            if (std::any_of(projNames.cbegin(), projNames.cend(), [&newProjName](const wxString &prj) { return newProjName==prj; }))
            {
                wxMessageDialog confirmDlg(wxGetTopLevelParent(this), wxT("Project name conflicted. Do you want to proceed anyway?"),
                    wxT("Project Name Conflict"), wxCENTER | wxNO_DEFAULT | wxYES_NO | wxICON_WARNING);
                int r = confirmDlg.ShowModal();
                if (wxID_YES == r)
                {
                    projPanel->NewProject(newProjName);
                    SpamConfig::Set(CommonDef::GetProjCfgPath(), newProjName);
                }
            }
            else
            {
                projPanel->NewProject(newProjName);
                SpamConfig::Set(CommonDef::GetProjCfgPath(), newProjName);
            }
        }
    }
}

void MainToolPane::OnOpen(wxCommandEvent &cmd)
{
    bool canOpen = AskSaveModifiedProjectFirst();
    if (canOpen)
    {
        wxString dbPath = SpamConfig::Get<wxString>(CommonDef::GetDBPathCfgPath());
        wxString selProj = SpamConfig::Get<wxString>(CommonDef::GetProjCfgPath());
        OpenProjDlg dlg(wxGetTopLevelParent(this), dbPath, selProj);
        if (dlg.ShowModal() == wxID_OK)
        {
            SpamConfig::Set(CommonDef::GetDBPathCfgPath(), dlg.dbPath_);
            SpamConfig::Set(CommonDef::GetProjCfgPath(), dlg.selProj_);

            ProjPanel *projPanel = GetProjPanel();
            if (projPanel)
            {
                if (dbPath != dlg.dbPath_ || selProj != dlg.selProj_)
                {
                    projPanel->LoadProject(dlg.dbPath_, dlg.selProj_);
                    projPanel->SetProjectModified(false);
                }
            }
        }
    }
}

void MainToolPane::OnSave(wxCommandEvent &cmd)
{
    wxString dbPath = SpamConfig::Get<wxString>(CommonDef::GetDBPathCfgPath());
    boost::filesystem::path p(dbPath.ToStdWstring());
    boost::system::error_code ec;
    if (boost::filesystem::exists(p, ec) && boost::filesystem::is_regular_file(p, ec))
    {
        SaveProject(dbPath);
    }
    else
    {
        OnSaveAs(cmd);
    }
}

void MainToolPane::OnSaveAs(wxCommandEvent &cmd)
{
    wxString wildCard{ "Spam DB files (*.spam_db)|*.spam_db" };
    wildCard.Append("|HDF5 files (*.h5)|*.h5");

    wxFileDialog saveFileDialog(wxGetTopLevelParent(this), _("Open Spam DB file"), "", "", wildCard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() != wxID_CANCEL)
    {
        auto fullPath = saveFileDialog.GetPath();
        SaveProject(fullPath);
    }
}

void MainToolPane::OnUndo(wxCommandEvent &cmd)
{
    SpamUndoRedo::Undo();
}

void MainToolPane::OnRedo(wxCommandEvent &cmd)
{
    SpamUndoRedo::Redo();
}

void MainToolPane::OnAllZoomIn(wxCommandEvent &e)
{
    ZoomAll(&CVImagePanel::ZoomIn);
}

void MainToolPane::OnAllZoomOut(wxCommandEvent &e)
{
    ZoomAll(&CVImagePanel::ZoomOut);
}

void MainToolPane::OnAllZoomExtent(wxCommandEvent &e)
{
    ZoomAll(&CVImagePanel::ZoomExtent);
}

void MainToolPane::OnAllZoomOriginal(wxCommandEvent &e)
{
    ZoomAll(&CVImagePanel::ZoomOriginal);
}

void MainToolPane::OnAllZoomHalf(wxCommandEvent &e)
{
    ZoomAll(&CVImagePanel::ZoomHalf);
}

void MainToolPane::OnAllZoomDouble(wxCommandEvent &e)
{
    ZoomAll(&CVImagePanel::ZoomDouble);
}

void MainToolPane::OnAllEnterScale(wxCommandEvent& e)
{
    wxSizerItem* sizerItem = GetSizer()->GetItemById(kSpamGlobalImageToolBar);
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

    std::vector<CVImagePanel *> imgPanelPages;
    GetAllImgPanelPages(imgPanelPages);

    RootFrame *rf = dynamic_cast<RootFrame *>(GetParent());
    auto noteBook = rf->GetStationNotebook();

    for (auto imgPanelPage : imgPanelPages)
    {
        if (imgPanelPage->HasImage())
        {
            imgPanelPage->SetScale(gScale_, false);
            rf->SyncScale(gScale_, noteBook, imgPanelPage);
        }
    }
}

void MainToolPane::OnAllSelectScale(wxCommandEvent &e)
{
    OnAllEnterScale(e);
}

void MainToolPane::SaveProject(const wxString &dbPath)
{
    ProjPanel *projPanel = GetProjPanel();
    if (projPanel)
    {
        projPanel->SaveProject(dbPath);
        SpamConfig::Set(CommonDef::GetDBPathCfgPath(), dbPath);
        SpamConfig::Set(CommonDef::GetProjCfgPath(), projPanel->GetProjectName());
        SpamConfig::Save();
    }
}

wxString MainToolPane::GetNextProjectName()
{
    auto cStr = wxString::FromDouble(cProject_++);
    return wxString(wxT("project")) + cStr;
}

bool MainToolPane::AskSaveModifiedProjectFirst()
{
    bool needProceed = true;
    ProjPanel *projPanel = GetProjPanel();
    if (projPanel && projPanel->IsProjectModified())
    {
        wxMessageDialog dialog(wxGetTopLevelParent(this), wxT("Project have modified. Do you want to save first?"),
            wxT("Project Modified"), wxCENTER | wxNO_DEFAULT | wxYES_NO | wxCANCEL | wxICON_WARNING);
        int r = dialog.ShowModal();
        if (wxID_YES == r)
        {
            OnSave(wxCommandEvent());
            needProceed = true;
        }
        else if (wxID_NO == r)
        {
            needProceed = true;
        }
        else
        {
            needProceed = false;
        }
    }

    return needProceed;
}

void MainToolPane::GetAllImgPanelPages(std::vector<CVImagePanel *> &imgPanelPages)
{
    RootFrame *rf = dynamic_cast<RootFrame *>(GetParent());
    auto noteBook = rf->GetStationNotebook();
    if (noteBook)
    {
        auto cPages = noteBook->GetPageCount();
        imgPanelPages.reserve(cPages);
        for (decltype(cPages) idx = 0; idx<cPages; ++idx)
        {
            auto imgPanelPage = dynamic_cast<CVImagePanel *>(noteBook->GetPage(idx));
            if (imgPanelPage)
            {
                imgPanelPages.push_back(imgPanelPage);
            }
        }
    }
}

void MainToolPane::ZoomAll(double (CVImagePanel::*zoomFun)(bool))
{
    std::vector<CVImagePanel *> imgPanelPages;
    GetAllImgPanelPages(imgPanelPages);

    RootFrame *rf = dynamic_cast<RootFrame *>(GetParent());
    auto noteBook = rf->GetStationNotebook();

    for (auto imgPanelPage : imgPanelPages)
    {
        if (imgPanelPage->HasImage())
        {
            double scale = (imgPanelPage->*zoomFun)(false);
            rf->SyncScale(scale, noteBook, imgPanelPage);
        }
    }
}