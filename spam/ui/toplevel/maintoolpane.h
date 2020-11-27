#ifndef SPAM_UI_TOP_LEVEL_MAIN_TOOL_PANE_H
#define SPAM_UI_TOP_LEVEL_MAIN_TOOL_PANE_H
#include <wx/wxprec.h>
#include <wx/nativewin.h>
#include <wx/aui/aui.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/filehistory.h>
class ProjPanel;
class CVImagePanel;

class MainToolPane : public wxPanel
{
public:
    MainToolPane(wxWindow* parent, wxFileHistory &fh);

private:
    wxToolBar *MakeMainToolBar();
    wxToolBar *MakeImageToolBar(wxFileHistory &fh);
    ProjPanel *GetProjPanel();

private:
    void OnNew(wxCommandEvent &cmd);
    void OnOpen(wxCommandEvent &cmd);

    void OnAllZoom(wxCommandEvent &e);
    void OnAllZoomIn(wxCommandEvent &e);
    void OnAllZoomOut(wxCommandEvent &e);
    void OnAllZoomExtent(wxCommandEvent &e);
    void OnAllZoomOriginal(wxCommandEvent &e);
    void OnAllZoomHalf(wxCommandEvent &e);
    void OnAllZoomDouble(wxCommandEvent &e);
    void OnAllEnterScale(wxCommandEvent& e);
    void OnAllSelectScale(wxCommandEvent &e);

private:
    void SaveProject(const wxString &dbPath);
    wxString GetNextProjectName();
    bool AskSaveModifiedProjectFirst();
    void GetAllImgPanelPages(std::vector<CVImagePanel *> &imgPanelPages);
    void ZoomAll(double (CVImagePanel::*zoomFun)(bool), bool changeIcon = true);

private:
    wxSize toolImageSize_;
    wxImageList toolImages_;
    int cProject_{ 0 };
    double gScale_{100.0};
};
#endif //SPAM_UI_TOP_LEVEL_MAIN_TOOL_PANE_H