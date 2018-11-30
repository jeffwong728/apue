#include "notool.h"
#include "probetool.h"
#include "recttool.h"
#include "ellipsetool.h"
#include "transformtool.h"
#include "nodeedittool.h"
#include "polygontool.h"
#include "beziergontool.h"
#include <wx/log.h>
#include <ui/spam.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/geomnode.h>
#include <ui/projs/drawablenode.h>
#include <ui/toplevel/rootframe.h>

NoTool::NoTool()
    : NoBoxTool(*this)
{
    wxLogMessage(wxT("NoTool Enter."));
}

NoTool::~NoTool()
{
    wxLogMessage(wxT("NoTool Quit."));
}

NoToolIdle::NoToolIdle()
{
    wxLogMessage(wxT("NoToolIdle Enter."));
}

NoToolIdle::~NoToolIdle()
{
    wxLogMessage(wxT("NoToolIdle Quit."));
}

sc::result NoToolIdle::react(const EvToolEnter &e)
{
    const int toolId = e.toolId;
    if (kSpamID_TOOLBOX_PROBE_SELECT == toolId)
    {
        return transit<ProbeTool>();
    }
    if (kSpamID_TOOLBOX_GEOM_RECT == toolId)
    {
        return transit<RectTool>();
    }
    if (kSpamID_TOOLBOX_GEOM_ELLIPSE == toolId)
    {
        return transit<EllipseTool>();
    }
    if (kSpamID_TOOLBOX_GEOM_POLYGON == toolId)
    {
        return transit<PolygonTool>();
    }
    if (kSpamID_TOOLBOX_GEOM_BEZIERGON == toolId)
    {
        return transit<BeziergonTool>();
    }
    if (kSpamID_TOOLBOX_GEOM_TRANSFORM == toolId)
    {
        return transit<TransformTool>();
    }
    if (kSpamID_TOOLBOX_GEOM_EDIT == toolId)
    {
        return transit<NodeEditTool>();
    }
    else
    {
        return forward_event();
    }
}

NoToolDraging::NoToolDraging()
{
    wxLogMessage(wxT("NoToolDraging Enter."));
}

NoToolDraging::~NoToolDraging()
{
    wxLogMessage(wxT("NoToolDraging Quit."));
}