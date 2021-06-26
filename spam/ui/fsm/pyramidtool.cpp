#include "pyramidtool.h"
#include <wx/log.h>
#include <ui/spam.h>
#include <ui/toplevel/rootframe.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/drawablenode.h>

void PyramidTool::OnOptionChanged(const EvToolOption &e)
{
    const int toolId = boost::get<int>(e.toolOptions.at(cp_ToolId));
    if (kSpamID_TOOLBOX_PROC_PYRAMID == toolId)
    {
        toolOptions = e.toolOptions;
        BoxToolImpl::ResetTool();
    }
}

void PyramidTool::OnBoxingEnded(const EvBoxingEnded &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.mData.GetEventObject());
    if (cav)
    {
        cav->UpdatePyramid(*(e.boxRect), boost::get<int>(toolOptions[cp_ToolProcPyramidLevel]));
        uuids.insert(cav->GetUUID());
    }
}

void PyramidTool::OnImageClicked(const EvImageClicked &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        cav->UpdatePyramid(Geom::Rect(), boost::get<int>(toolOptions[cp_ToolProcPyramidLevel]));
        uuids.insert(cav->GetUUID());
    }
}

void PyramidTool::OnEntityClicked(const EvEntityClicked &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.e.GetEventObject());
    if (cav)
    {
        auto bbox = e.ent->GetBoundingBox();
        cav->UpdatePyramid(bbox ? *bbox : Geom::Rect(), boost::get<int>(toolOptions[cp_ToolProcPyramidLevel]));
        uuids.insert(cav->GetUUID());
    }
}

sc::result PyramidTool::react(const EvToolQuit &e)
{
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (frame)
    {
        for (const auto &uuid : uuids)
        {
            CairoCanvas *cav = frame->FindCanvasByUUID(uuid);
            if (cav)
            {
                cav->RemoveImageProcessData();
            }
        }
    }

    BoxToolT::QuitTool(e);
    return transit<NoTool>();
}
