#ifndef SPAM_UI_PROJS_PROJ_TREE_MODEL_H
#define SPAM_UI_PROJS_PROJ_TREE_MODEL_H
#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <wx/dataview.h>
#include <boost/signals2.hpp>
namespace bs2 = boost::signals2;
#include "modelfwd.h"
#include "modelnode.h"
#include "stationnode.h"

class ProjTreeModel : public wxDataViewModel
{
public:
    enum
    {
        kStation_LABEL_COL,
        kStation_VISIBLE_COL,
        kStation_DRAW_STYLE_COL,
        kStation_LOCK_COL,
        kStation_GUARD_COL
    };

public:
    typedef bs2::keywords::mutex_type<bs2::dummy_mutex> bs2_dummy_mutex;
    bs2::signal_type<void(const SPModelNodeVector &), bs2_dummy_mutex>::type sig_EntityCreate;
    bs2::signal_type<void(const SPModelNodeVector &), bs2_dummy_mutex>::type sig_EntityAdd;
    bs2::signal_type<void(const SPModelNodeVector &), bs2_dummy_mutex>::type sig_EntityDelete;
    bs2::signal_type<void(const SPModelNodeVector &), bs2_dummy_mutex>::type sig_StationAdd;
    bs2::signal_type<void(const SPModelNodeVector &), bs2_dummy_mutex>::type sig_StationDelete;
    bs2::signal_type<void(const SPModelNodeVector &), bs2_dummy_mutex>::type sig_GeomCreate;
    bs2::signal_type<void(const SPModelNodeVector &), bs2_dummy_mutex>::type sig_GeomAdd;
    bs2::signal_type<void(const SPModelNodeVector &), bs2_dummy_mutex>::type sig_GeomDelete;
    bs2::signal_type<void(const SPDrawableNodeVector &, const Geom::OptRect &rect), bs2_dummy_mutex>::type sig_DrawableShapeChange;

public:
    ProjTreeModel();
    ProjTreeModel(const wxString &title);
    ~ProjTreeModel();

public:
    wxString GetTitle(const wxDataViewItem &item) const;
    wxString GetProjectName() const;
    SPProjNode GetProject() const;
    SPStationNode GetCurrentStation() const { return currentStation_.lock(); }
    void SetCurrentStation(const SPStationNode &station);
    bool GetVisible(const wxDataViewItem &item) const;
    SPStationNode CreateStation(const wxString &title);
    void AddStation(const SPStationNode &station);
    void AddStations(const SPStationNodeVector &stations);
    void DeleteStation(const SPStationNode &station, bool fireEvent);
    void AddToStation(SPStationNode &station, SPGeomNode &geom, bool fireEvent);
    void AddToStations(SPStationNode &station, SPGeomNodeVector &geoms, bool fireEvent);
    SPGeomNode CreateToStation(SPStationNode &station, const RectData &rd);
    SPGeomNode CreateToStation(SPStationNode &station, const LineData &ld);
    SPGeomNode CreateToStation(SPStationNode &station, const GenericEllipseArcData &ed);
    SPGeomNode CreateToStation(SPStationNode &station, const PolygonData &pd);
    SPGeomNode CreateToStation(SPStationNode &station, const BezierData &bd);
    void Delete(const wxDataViewItem &item, bool fireEvent);
    void Delete(const wxDataViewItemArray &items, bool fireEvent);
    void NewCurrentStation();
    void NewProject(const wxString &projName);
    void Save(const wxString &path) const;
    void LoadProject(const wxString& dbPath, const wxString &projName);
    void SetModified(bool modified=true) const { modified_ = modified; }
    bool IsModified() const { return modified_; }
    SPStationNodeVector GetAllStations() const;
    SPStationNode FindStationByUUID(const std::string &uuidTag) const;

    void RestoreTransform(SPDrawableNodeVector &drawables, const SpamMany &mementos, const bool fireEvent);

public:
    unsigned int GetColumnCount() const wxOVERRIDE { return kStation_GUARD_COL; }
    wxString GetColumnType(unsigned int col) const wxOVERRIDE;
    void GetValue(wxVariant &variant, const wxDataViewItem &item, unsigned int col) const wxOVERRIDE;
    bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col) wxOVERRIDE;
    bool IsEnabled(const wxDataViewItem &item, unsigned int col) const wxOVERRIDE;
    wxDataViewItem GetParent(const wxDataViewItem &item) const wxOVERRIDE;
    bool IsContainer(const wxDataViewItem &item) const wxOVERRIDE;
    bool HasContainerColumns(const wxDataViewItem& WXUNUSED(item)) const { return false; }
    unsigned int GetChildren(const wxDataViewItem &parent, wxDataViewItemArray &array) const wxOVERRIDE;
    bool GetAttr(const wxDataViewItem &item, unsigned int col, wxDataViewItemAttr &attr) const wxOVERRIDE;

private:
    bool IsNameExisting(const SPModelNode &parent, const wxString &baseName);
    wxString GetUnusedName(const SPModelNode &parent, const wxString &seedName);
    void ScanSetCurrentSattion(void);
    void FireDeleteSignal(const SPModelNodeVector &ents) const;

private:
    SPModelNode   root_;
    WPStationNode currentStation_;
private:
    mutable bool modified_;
};

#endif //SPAM_UI_PROJS_PROJ_TREE_MODEL_H