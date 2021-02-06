#ifndef SPAM_UI_TOP_LEVEL_ROOT_FRAME_H
#define SPAM_UI_TOP_LEVEL_ROOT_FRAME_H
#include <wx/wxprec.h>
#include <wx/nativewin.h>
#include <wx/aui/aui.h>
#include <wx/dataview.h>
#include <wx/filehistory.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/popupwin.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <ui/cmndef.h>
#include <ui/projs/modelfwd.h>
#include <string>
#include <vector>
#include <map>
class ProjPanel;
class ModelEvent;
class ProjTreeModel;
class DropImageEvent;
class CVImagePanel;
class CairoCanvas;
class SelectionFilter;
struct Spamer;

namespace cv
{
    class Mat;
}

namespace Geom 
{
    class OptRect;
    class Path;
}

class RootFrame : public wxFrame
{
    enum {
        kSpam_TOOLBOX_PROBE,
        kSpam_TOOLBOX_GEOM,
        kSpam_TOOLBOX_PROC,
        kSpam_TOOLBOX_MATCH,
        kSpam_TOOLBOX_STYLE,

        kSpam_TOOLBOX_GUARD
    };
    friend struct Spamer;
public:
    RootFrame();
    ~RootFrame();

private:
    void CreateMenu();
    void CreateAuiPanes();

public:
    SelectionFilter *GetSelectionFilter() { return selFilter_.get(); }
    ProjPanel *GetProjPanel();
    void SaveProject(const wxString &dbPath);
    wxAuiNotebook *GetStationNotebook() const;
    ProjTreeModel *GetProjTreeModel();
    CairoCanvas *FindCanvasByUUID(const std::string &uuidTag) const;
    void RequestUpdateHistogram(const std::string &uuidTag, const boost::any &roi);
    void RequestUpdateThreshold(const std::string &uuidTag, const boost::any &roi);
    void AddDirtRect(const std::string &uuidTag, const Geom::OptRect &dirtRect);
    void RequestRefreshCanvas();
    void SyncScale(double scale, wxAuiNotebook *nb, wxWindow *page);
    void SetStatusText(const wxString &text, int number = 0) wxOVERRIDE;
    void SetBitmapStatus(const StatusIconType iconType, const wxString &text);

private:
    void OnHello(wxCommandEvent& e);
    void OnExit(wxCommandEvent& e);
    void OnClose(wxCloseEvent& e);
    void OnLoadImage(wxCommandEvent& e);
    void OnLoadPy3(wxCommandEvent& e);
    void OnAddStations(const SPModelNodeVector &stations);
    void OnDeleteStations(const SPModelNodeVector &stations);
    void OnAddGeoms(const SPModelNodeVector &geoms);
    void OnDeleteGeoms(const SPModelNodeVector &geoms);
    void OnDrawableShapeChange(const SPDrawableNodeVector &drawables, const Geom::OptRect &rect);
    void OnGlowGeom(const SPModelNode &geom);
    void OnDimGeom(const SPModelNode &geom);
    void OnNewProject(ModelEvent& e);
    void OnLoadProject(ModelEvent& e);
    void OnViewMainTool(wxCommandEvent& e);
    void OnViewImage(wxCommandEvent& e);
    void OnViewDefaultLayout(wxCommandEvent& e);
    void OnUpdateUI(wxUpdateUIEvent& e);
    void OnStationActivated(wxDataViewEvent &e);
    void OnSetTileLayout(wxCommandEvent& e);
    void OnTileLayout(wxCommandEvent& e);
    void OnStackLayout(wxCommandEvent& e);
    void OnDropImage(DropImageEvent& e);
    void OnTabExtension(wxAuiNotebookEvent& e);
    void OnPageSelect(wxAuiNotebookEvent& e);
    void OnAuiPageClosed(wxAuiManagerEvent& e);
    void OnZoom(wxCommandEvent &cmd);
    void OnZoomIn(wxCommandEvent &cmd);
    void OnZoomOut(wxCommandEvent &cmd);
    void OnZoomExtent(wxCommandEvent &cmd);
    void OnZoomOriginal(wxCommandEvent &cmd);
    void OnZoomHalf(wxCommandEvent &cmd);
    void OnZoomDouble(wxCommandEvent &cmd);
    void OnEnterScale(wxCommandEvent& e);
    void OnSelectScale(wxCommandEvent &e);
    void OnToolboxInfo(wxCommandEvent& e);
    void OnToolboxGeom(wxCommandEvent& e);
    void OnToolboxProc(wxCommandEvent& e);
    void OnToolboxMatch(wxCommandEvent& e);
    void OnToolboxStyle(wxCommandEvent& e);
    void OnSelectEntity(const SPDrawableNodeVector &des);
    void OnImageBufferItemAdd(const ImageBufferItem &ibi);
    void OnImageBufferItemUpdate(const ImageBufferItem &ibi);

public:
    static void file_import_image_cb(GtkWidget *menuitem, gpointer user_data);
    static void file_export_image_cb(GtkWidget *menuitem, gpointer user_data);
    static void file_import_py3_cb(GtkWidget *menuitem, gpointer user_data);
    static void file_export_py3_cb(GtkWidget *menuitem, gpointer user_data);
    static void file_quit_cb(GtkWidget *menuitem, gpointer user_data);
    static void file_save_cb(GtkWidget *menuitem, gpointer user_data);
    static void file_save_as_cb(GtkWidget *menuitem, gpointer user_data);
    static void undo_cb(GtkWidget *widget, gpointer user_data);
    static void redo_cb(GtkWidget *widget, gpointer user_data);
    static void help_about_cb(GtkWidget *widget, gpointer user_data);
    static void view_project_cb(GtkWidget *widget, gpointer user_data);
    static void view_entity_cb(GtkWidget *widget, gpointer user_data);
    static void view_toolbox_cb(GtkWidget *widget, gpointer user_data);
    static void view_log_cb(GtkWidget *widget, gpointer user_data);
    static void view_pyeditor_cb(GtkWidget *widget, gpointer user_data);
    static void preferences_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);

private:
    void ReplaceTitleBar(void);
    wxAuiNotebook *CreateStationNotebook();
    std::string GetNextCVStationWinName();
    std::vector<std::string> GetAllTabPaneNames(const std::string &perspective);
    wxToolBar *MakeStationToolBar(wxWindow *parent);
    void Zoom(double (CVImagePanel::*zoomFun)(bool), wxCommandEvent &cmd, bool changeIcon = true);
    void UpdateGeoms(void (CairoCanvas::*updateFun)(const SPDrawableNodeVector &), const SPModelNodeVector &geoms);
    void SyncScale(wxControl *extCtrl);
    void EnablePageImageTool(wxControl *extCtrl, bool bEnable);
    void SetStationImage(const wxString &uuidStation, const cv::Mat &img);
    wxPoint GetToolboxPopupPosition(const wxSize &popUpSize);
    void ToggleToolboxPane(const int boxShow);
    void ConnectCanvas(CVImagePanel *imgPanel);
    void ClickToolbox(wxCommandEvent& e, const int toolIndex);
    CVImagePanel *FindImagePanelByStation(const SPStationNode &station) const;

private:
    mutable wxAuiManager wxAuiMgr_;
    const wxString mainToolPanelName_;
    const wxString stationNotebookName_;
    const wxString projPanelName_;
    const wxString pyEditorName_;
    const wxString consolePanelName_;
    const wxString imagesZonePanelName_;
    const wxString toolBoxBarName_;
    wxFileHistory  imageFileHistory_;
    int cCVStation_{ 0 };
    double scale_{ 100.0 };
    wxString initialPerspective_;
    const wxString toolBoxLabels[kSpam_TOOLBOX_GUARD];
    std::unique_ptr<Spamer> spamer_;
    std::unique_ptr<SelectionFilter> selFilter_;
    std::unique_ptr<std::map<std::string, Geom::OptRect>> cavDirtRects_;

private:
    std::vector<std::pair<int, GtkWidget *>> widgets_;
};
#endif //SPAM_UI_TOP_LEVEL_ROOT_FRAME_H