#ifndef SPAM_UI_CMDS_GEOM_CMD_H
#define SPAM_UI_CMDS_GEOM_CMD_H
#include "spamcmd.h"
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <string>
#include <boost/core/noncopyable.hpp>
#include <map>
class ProjTreeModel;

class GeomCmd : public SpamCmd
{
public:
    GeomCmd(ProjTreeModel *model, SPStationNode &station);
    GeomCmd(ProjTreeModel *model, SPStationNode &station, SPGeomNode &geom);

protected:
    ProjTreeModel *model_;
    SPStationNode station_;
    SPGeomNode    geom_;
};

class DeleteGeomsCmd : public SpamCmd
{
public:
    DeleteGeomsCmd(ProjTreeModel *model, const SPGeomNodeVector &geoms);

public:
    void Do() wxOVERRIDE;
    void Undo() wxOVERRIDE;
    void Redo() wxOVERRIDE;

public:
    wxString GetDescription() const wxOVERRIDE;

private:
    ProjTreeModel *model_;
    std::map<wxString, std::pair<SPStationNode, SPGeomNodeVector>> geoms_;
};

class CreateRectCmd : public GeomCmd
{
public:
    CreateRectCmd(ProjTreeModel *model, SPStationNode &station, const wxString &wouldTitle, const RectData &data);

public:
    void Do() wxOVERRIDE;
    void Undo() wxOVERRIDE;
    void Redo() wxOVERRIDE;

public:
    wxString GetDescription() const wxOVERRIDE;

private:
    wxString wouldTitle_;
    RectData data_;
};

class CreatePolygonCmd : public GeomCmd
{
public:
    CreatePolygonCmd(ProjTreeModel *model, SPStationNode &station, const wxString &wouldTitle, const PolygonData &data);

public:
    void Do() wxOVERRIDE;
    void Undo() wxOVERRIDE;
    void Redo() wxOVERRIDE;

public:
    wxString GetDescription() const wxOVERRIDE;

private:
    wxString wouldTitle_;
    PolygonData data_;
};

class CreateEllipseCmd : public GeomCmd
{
public:
    CreateEllipseCmd(ProjTreeModel *model, SPStationNode &station, const wxString &wouldTitle, const GenericEllipseArcData &data);

public:
    void Do() wxOVERRIDE;
    void Undo() wxOVERRIDE;
    void Redo() wxOVERRIDE;

public:
    wxString GetDescription() const wxOVERRIDE;

private:
    wxString wouldTitle_;
    GenericEllipseArcData data_;
};

#endif //SPAM_UI_CMDS_GEOM_CMD_H