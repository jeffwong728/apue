#ifndef SPAM_UI_CMDS_TRANSFORM_CMD_H
#define SPAM_UI_CMDS_TRANSFORM_CMD_H
#include "spamcmd.h"
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <string>
#include <boost/core/noncopyable.hpp>
#include <map>
class ProjTreeModel;

class TransformCmd : public SpamCmd
{
public:
    TransformCmd(ProjTreeModel *model, SPStationNode &station, const SPDrawableNodeVector &drawables, const SpamMany &mementos);

public:
    void Do() wxOVERRIDE;
    void Undo() wxOVERRIDE;
    void Redo() wxOVERRIDE;

public:
    wxString GetDescription() const wxOVERRIDE;

private:
    ProjTreeModel *model_;
    SPStationNode station_;
    SPDrawableNodeVector drawables_;
    SpamMany mementos_;
};

class NodeEditCmd : public TransformCmd
{
public:
    NodeEditCmd(ProjTreeModel *model, SPStationNode &station, const SPDrawableNodeVector &drawables, const SpamMany &mementos)
    : TransformCmd(model, station, drawables, mementos) {}

public:
    wxString GetDescription() const wxOVERRIDE;
};

class NodeModifyCmd : public SpamCmd
{
public:
    NodeModifyCmd(ProjTreeModel *model, SPStationNode &station, const SPDrawableNode &drawable, const boost::any &memento);

public:
    void Do() wxOVERRIDE;
    void Undo() wxOVERRIDE;
    void Redo() wxOVERRIDE;

public:
    wxString GetDescription() const wxOVERRIDE;

private:
    ProjTreeModel * model_;
    SPStationNode station_;
    SPDrawableNode drawable_;
    boost::any memento_;
};

#endif //SPAM_UI_CMDS_TRANSFORM_CMD_H