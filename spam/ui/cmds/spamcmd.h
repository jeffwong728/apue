#ifndef SPAM_UI_CMDS_SPAM_CMD_H
#define SPAM_UI_CMDS_SPAM_CMD_H
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <string>
#include <boost/core/noncopyable.hpp>
#include <ui/projs/modelfwd.h>
#include "cmdsfwd.h"

class SpamCmd : private boost::noncopyable
{
    enum CmdWhere
    {
        kSpamUndoStack,
        kSpamRedoStack
    };

public:
    SpamCmd();
    virtual ~SpamCmd() {}

public:
    virtual void Do() = 0;
    virtual void Undo() = 0;
    virtual void Redo() = 0;

public:
    virtual wxString GetDescription() const = 0;

protected:
    CmdWhere where_;
};

class MacroCmd : public SpamCmd
{
public:
    MacroCmd(const SPSpamCmdVector &cmds) : SpamCmd(), cmds_(cmds) {}

public:
    void Do() wxOVERRIDE;
    void Undo() wxOVERRIDE;
    void Redo() wxOVERRIDE;

public:
    wxString GetDescription() const wxOVERRIDE;

private:
    SPSpamCmdVector cmds_;
};



#endif //SPAM_UI_CMDS_SPAM_CMD_H