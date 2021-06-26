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
        const auto minGray = boost::get<int>(toolOptions[cp_ToolProcThresholdMin]);
        const auto maxGray = boost::get<int>(toolOptions[cp_ToolProcThresholdMax]);
        const auto channel = boost::get<int>(toolOptions[cp_ToolProcThresholdChannel]);
        cav->UpdateBinary(*(e.boxRect), minGray, maxGray, std::max(0, channel));
        frame->RequestUpdateThreshold(cav->GetUUID(), e.boxRect);
        uuids.insert(cav->GetUUID());
    }
}

void ThresholdTool::OnImageClicked(const EvImageClicked &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (cav && frame)
    {
        const auto minGray = boost::get<int>(toolOptions[cp_ToolProcThresholdMin]);
        const auto maxGray = boost::get<int>(toolOptions[cp_ToolProcThresholdMax]);
        const auto channel = boost::get<int>(toolOptions[cp_ToolProcThresholdChannel]);
        cav->UpdateBinary(Geom::Rect(), minGray, maxGray, std::max(0, channel));
        frame->RequestUpdateThreshold(cav->GetUUID(), Geom::OptRect());
        uuids.insert(cav->GetUUID());
    }
}

void ThresholdTool::OnEntityClicked(const EvEntityClicked &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.e.GetEventObject());
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (cav && frame)
    {
        const auto minGray = boost::get<int>(toolOptions[cp_ToolProcThresholdMin]);
        const auto maxGray = boost::get<int>(toolOptions[cp_ToolProcThresholdMax]);
        const auto channel = boost::get<int>(toolOptions[cp_ToolProcThresholdChannel]);
        Geom::PathVector pv;
        e.ent->BuildPath(pv);
        auto bbox = e.ent->GetBoundingBox();
        cav->UpdateBinary(bbox ? *bbox : Geom::Rect(), minGray, maxGray, std::max(0, channel));
        frame->RequestUpdateThreshold(cav->GetUUID(), pv);
        uuids.insert(cav->GetUUID());
    }
}

sc::result ThresholdTool::react(const EvToolQuit &e)
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
