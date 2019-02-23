#include "proctool.h"
#include <wx/log.h>
#include <ui/spam.h>
#include <ui/toplevel/rootframe.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/drawablenode.h>

void ThresholdTool::OnOptionChanged(const EvToolOption &e)
{
    const int toolId = boost::get<int>(e.toolOptions.at(cp_ToolId));
    if (kSpamID_TOOLBOX_PROC_THRESHOLD == toolId)
    {
        toolOptions = e.toolOptions;
        BoxToolImpl::ResetTool();
    }
}

void ThresholdTool::OnBoxingEnded(const EvBoxingEnded &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.mData.GetEventObject());
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (cav && frame)
    {
        frame->RequestUpdateThreshold(cav->GetUUID(), e.boxRect);
    }
}

void ThresholdTool::OnImageClicked(const EvImageClicked &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (cav && frame)
    {
        frame->RequestUpdateThreshold(cav->GetUUID(), Geom::OptRect());
    }
}

void ThresholdTool::OnEntityClicked(const EvEntityClicked &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.e.GetEventObject());
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (cav && frame)
    {
        Geom::PathVector pv;
        e.ent->BuildPath(pv);
        frame->RequestUpdateThreshold(cav->GetUUID(), pv);
    }
}