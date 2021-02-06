#ifndef SPAM_UI_TOP_LEVEL_PROJ_PANEL_H
#define SPAM_UI_TOP_LEVEL_PROJ_PANEL_H
#include <wx/wxprec.h>
#include <wx/nativewin.h>
#include <wx/aui/aui.h>
#include <wx/dataview.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <ui/cmndef.h>
#include <ui/projs/modelfwd.h>
#include <boost/signals2.hpp>
namespace bs2 = boost::signals2;

class ProjPanel : public wxPanel
{
public:
    typedef bs2::keywords::mutex_type<bs2::dummy_mutex> bs2_dummy_mutex;
    bs2::signal_type<void(const SPModelNode &), bs2_dummy_mutex>::type sig_EntityGlow;
    bs2::signal_type<void(const SPModelNode &), bs2_dummy_mutex>::type sig_EntityDim;
    bs2::signal_type<void(const SPDrawableNodeVector &), bs2_dummy_mutex>::type sig_EntitySelect;

public:
    ProjPanel(wxWindow* parent);

public:
    wxString GetProjectName() const;
    ProjTreeModel *GetProjTreeModel() const;
    void NewProject(const wxString &projName);
    void SaveProject(const wxString& dbPath);
    void LoadProject(const wxString& dbPath, const wxString &projName);
    void SetProjectModified(bool modified=true);
    bool IsProjectModified() const;
    WPStationNode CreateStation();
    void GlowEntity(const SPDrawableNode &de);
    void DimEntity(const SPDrawableNode &de);
    void SelectEntity(const SPDrawableNodeVector &des);
    void DeselectEntity(const SPDrawableNodeVector &des);

private:
    void AddToolBarButton(wxToolBar *tb, const wxString& label, const wxString& artid);
    wxToolBar *MakeToolBar();
    wxDataViewCtrl *MakeProjView();
    wxDataViewCtrl *GetProjView() const;

private:
    void OnAddStation(wxCommandEvent &cmd);
    void OnDeleteEntities(wxCommandEvent &cmd);
    void OnShowEntities(wxCommandEvent &cmd);
    void OnHideEntities(wxCommandEvent &cmd);
    void OnShowOnlyEntities(wxCommandEvent &cmd);
    void OnShowReverseEntities(wxCommandEvent &cmd);
    void OnShowAllEntities(wxCommandEvent &cmd);
    void OnHideAllEntities(wxCommandEvent &cmd);
    void OnExpandNode(const SPModelNodeVector &nodes);
    void OnSelectionChanged(wxDataViewEvent &e);
    void OnContextMenu(wxDataViewEvent &e);
    void OnModelBrowserMouseMotion(wxMouseEvent &e);
    void OnLeaveModelBrowser(wxMouseEvent &e);

private:
    wxSize toolImageSize_;
    wxImageList toolImages_;
    int cStation_;
};
#endif //SPAM_UI_TOP_LEVEL_PROJ_PANEL_H