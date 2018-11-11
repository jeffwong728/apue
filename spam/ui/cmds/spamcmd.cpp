#include "spamcmd.h"
#include <ui/spam.h>

SpamCmd::SpamCmd()
: where_(kSpamUndoStack)
{
}

void MacroCmd::Do()
{
    for (auto &cmd : cmds_)
    {
        cmd->Do();
    }
}

void MacroCmd::Undo()
{
    for (auto &cmd : cmds_)
    {
        cmd->Undo();
    }
}

void MacroCmd::Redo()
{
    for (auto &cmd : cmds_)
    {
        cmd->Redo();
    }
}

wxString MacroCmd::GetDescription() const
{
    return wxString(wxT("Delete Multiple Entities"));
}