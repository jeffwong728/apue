#include "nodeedittool.h"
#include <wx/log.h>
#include <wx/dcgraph.h>
#include <ui/spam.h>
#include <ui/toplevel/rootframe.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/drawablenode.h>
#include <ui/projs/geomnode.h>

NodeEditTool::NodeEditTool()
    : NodeEditToolImpl(*this)
{
    wxLogMessage(wxT("NodeEditTool Enter."));
}

NodeEditTool::~NodeEditTool()
{
}

NodeEditIdle::NodeEditIdle()
    : EditIdleT(*this)
{
    wxLogMessage(wxT("NodeEditIdle Enter."));
}

NodeEditIdle::~NodeEditIdle()
{
    wxLogMessage(wxT("NodeEditIdle Quit."));
}

sc::result NodeEditIdle::react(const EvLMouseDown &e)
{
    return reactLMouseDown(e);
}

NodeEditBoxSelecting::NodeEditBoxSelecting()
{
    wxLogMessage(wxT("NodeEditDraging Enter."));
}

NodeEditBoxSelecting::~NodeEditBoxSelecting()
{
    wxLogMessage(wxT("NodeEditDraging Quit."));
}

NodeEditing::NodeEditing()
{
    wxLogMessage(wxT("NodeEditing Enter."));
}

NodeEditing::~NodeEditing()
{
    wxLogMessage(wxT("NodeEditing Quit."));
}