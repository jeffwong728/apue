#include "transformtool.h"
#include <wx/log.h>
#include <wx/dcgraph.h>
#include <ui/spam.h>
#include <ui/toplevel/rootframe.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/drawablenode.h>
#include <ui/projs/geomnode.h>

TransformTool::TransformTool()
    : EditToolT(*this)
{
    wxLogMessage(wxT("TransformTool Enter."));
}

TransformTool::~TransformTool()
{
}

TransformIdle::TransformIdle()
    : IdleT(*this)
{
    wxLogMessage(wxT("TransformIdle Enter."));
}

TransformIdle::~TransformIdle()
{
    wxLogMessage(wxT("TransformIdle Quit."));
}

sc::result TransformIdle::react(const EvToolQuit &e)
{
    return transit<NoTool>();
}

sc::result TransformIdle::react(const EvLMouseDown &e)
{
    return reactLMouseDown(e);
}

TransformBoxSelecting::TransformBoxSelecting()
{
    wxLogMessage(wxT("TransformDraging Enter."));
}

TransformBoxSelecting::~TransformBoxSelecting()
{
    wxLogMessage(wxT("TransformDraging Quit."));
}

Transforming::Transforming()
{
    wxLogMessage(wxT("Transforming Enter."));
}

Transforming::~Transforming()
{
    wxLogMessage(wxT("Transforming Quit."));
}