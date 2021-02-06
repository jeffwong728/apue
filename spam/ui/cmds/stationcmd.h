#ifndef SPAM_UI_CMDS_STATIONS_CMD_H
#define SPAM_UI_CMDS_STATIONS_CMD_H
#include "spamcmd.h"
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <string>
#include <boost/core/noncopyable.hpp>
class ProjTreeModel;

class CreateStationCmd : public SpamCmd
{
public:
    CreateStationCmd(ProjTreeModel *model, const wxString &wouldTitle);

public:
    void Do() wxOVERRIDE;
    void Undo() wxOVERRIDE;
    void Redo() wxOVERRIDE;

public:
    wxString GetDescription() const wxOVERRIDE;
    WPStationNode GetStation() const { return station_; }

private:
    ProjTreeModel *model_;
    wxString wouldTitle_;
    SPStationNode station_;
    SPStationNode currentStation_;
};

class DeleteStationsCmd : public SpamCmd
{
public:
    DeleteStationsCmd(ProjTreeModel *model, const SPStationNodeVector &stations);

public:
    void Do() wxOVERRIDE;
    void Undo() wxOVERRIDE;
    void Redo() wxOVERRIDE;

public:
    wxString GetDescription() const wxOVERRIDE;

private:
    ProjTreeModel *model_;
    SPStationNodeVector stations_;
};

#endif //SPAM_UI_CMDS_STATIONS_CMD_H