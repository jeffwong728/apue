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

class CreateBeziergonCmd : public GeomCmd
{
public:
    CreateBeziergonCmd(ProjTreeModel *model, SPStationNode &station, const wxString &wouldTitle, const BezierData &data);

public:
    void Do() wxOVERRIDE;
    void Undo() wxOVERRIDE;
    void Redo() wxOVERRIDE;

public:
    wxString GetDescription() const wxOVERRIDE;

private:
    wxString wouldTitle_;
    BezierData data_;
};

class BoolCmd : public SpamCmd
{
protected:
    enum {UnionOp, IntersectionOp};
public:
    BoolCmd(ProjTreeModel *model, const SPGeomNodeVector &geoms, const wxString &wouldTitle);

public:
    void Undo() wxOVERRIDE;
    void Redo() wxOVERRIDE;

protected:
    void BoolOp(const int op);

private:
    wxString         wouldTitle_;
    ProjTreeModel   *model_;
    SPStationNode    station_;
    SPGeomNode       uGeom_;
    SPGeomNodeVector geoms_;
};

class UnionGeomsCmd : public BoolCmd
{
public:
    UnionGeomsCmd(ProjTreeModel *model, const SPGeomNodeVector &geoms, const wxString &wouldTitle) : BoolCmd(model, geoms, wouldTitle) {}

public:
    void Do() wxOVERRIDE { BoolOp(BoolCmd::UnionOp); }

public:
    wxString GetDescription() const wxOVERRIDE;
};

class IntersectionGeomsCmd : public BoolCmd
{
public:
    IntersectionGeomsCmd(ProjTreeModel *model, const SPGeomNodeVector &geoms, const wxString &wouldTitle) : BoolCmd(model, geoms, wouldTitle) {}

public:
    void Do() wxOVERRIDE { BoolOp(BoolCmd::IntersectionOp); }

public:
    wxString GetDescription() const wxOVERRIDE;
};

class BinaryBoolGeomsCmd : public SpamCmd
{
protected:
    enum { DiffOp, XOROp };

public:
    BinaryBoolGeomsCmd(ProjTreeModel *model, const SPGeomNode &geom1, const SPGeomNode &geom2, const wxString &wouldTitle);

public:
    void Undo() wxOVERRIDE;
    void Redo() wxOVERRIDE;

protected:
    void BoolOp(const int op);

private:
    wxString         wouldTitle_;
    ProjTreeModel   *model_;
    SPStationNode    station_;
    SPGeomNode       dGeom_;
    SPGeomNode       geom1_;
    SPGeomNode       geom2_;
};

class DiffGeomsCmd : public BinaryBoolGeomsCmd
{
public:
    DiffGeomsCmd(ProjTreeModel *model, const SPGeomNode &geom1, const SPGeomNode &geom2, const wxString &wouldTitle) : BinaryBoolGeomsCmd(model, geom1, geom2, wouldTitle) {}

public:
    void Do() wxOVERRIDE { BoolOp(BinaryBoolGeomsCmd::DiffOp); }

public:
    wxString GetDescription() const wxOVERRIDE;
};

class XORGeomsCmd : public BinaryBoolGeomsCmd
{
public:
    XORGeomsCmd(ProjTreeModel *model, const SPGeomNode &geom1, const SPGeomNode &geom2, const wxString &wouldTitle) : BinaryBoolGeomsCmd(model, geom1, geom2, wouldTitle) {}

public:
    void Do() wxOVERRIDE { BoolOp(BinaryBoolGeomsCmd::XOROp); }

public:
    wxString GetDescription() const wxOVERRIDE;
};

#endif //SPAM_UI_CMDS_GEOM_CMD_H