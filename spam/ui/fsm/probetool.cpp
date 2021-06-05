#include "probetool.h"
#include <wx/log.h>
#include <wx/dcgraph.h>
#include <ui/spam.h>
#include <ui/toplevel/rootframe.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/drawablenode.h>
#include <ui/projs/fixednode.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <2geom/path-sink.h>
#pragma warning( pop )

void ProbeTool::OnOptionChanged(const EvToolOption &e)
{
    const int toolId = boost::get<int>(e.toolOptions.at(cp_ToolId));
    if (kSpamID_TOOLBOX_PROBE_SELECT == toolId)
    {
        toolOptions = e.toolOptions;
        BoxToolImpl::ResetTool();
    }
}

void ProbeTool::OnImageClicked(const EvImageClicked &e)
{
    const int probeMode = boost::get<int>(toolOptions.at(cp_ToolProbeMode));
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (kSpamID_TOOLBOX_PROBE_IMAGE == probeMode)
    {
        if (cav)
        {
            cav->PopupImageInfomation(e.evData.GetPosition());
        }
    }

    if (cav)
    {
        cav->ClearSelectRegions();
        cav->ClearSelectContours();
    }
}

void ProbeIdle::OnSafari(const EvMouseMove &e)
{
    context<ProbeTool>().Safari(e);
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        const int probeMode = boost::get<int>(context<ProbeTool>().toolOptions.at(cp_ToolProbeMode));
        if (kSpamID_TOOLBOX_PROBE_PIXEL == probeMode)
        {
            if (cav->IsInImageRect(e.evData.GetPosition()))
            {
                cav->DismissInstructionTip();
                cav->ShowPixelValue(e.evData.GetPosition());
            }
            else
            {
                cav->StopInstructionTip();
            }
        }
    }
}

void ProbeIdle::OnLeaveCanvas(const EvCanvasLeave &e)
{
    context<ProbeTool>().LeaveCanvas(e);
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        cav->StopInstructionTip();
    }
}

void HistogramTool::OnOptionChanged(const EvToolOption &e)
{
    const int toolId = boost::get<int>(e.toolOptions.at(cp_ToolId));
    if (kSpamID_TOOLBOX_PROBE_HISTOGRAM == toolId)
    {
        toolOptions = e.toolOptions;
        BoxToolImpl::ResetTool();
    }
}

void HistogramTool::OnBoxingEnded(const EvBoxingEnded &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.mData.GetEventObject());
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (cav && frame)
    {
        frame->RequestUpdateHistogram(cav->GetUUID(), e.boxRect);
    }
}

void HistogramTool::OnImageClicked(const EvImageClicked &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (cav && frame)
    {
        frame->RequestUpdateHistogram(cav->GetUUID(), Geom::OptRect());
    }
}

void HistogramTool::OnEntityClicked(const EvEntityClicked &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.e.GetEventObject());
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (cav && frame)
    {
        Geom::PathVector pv;
        e.ent->BuildPath(pv);
        frame->RequestUpdateHistogram(cav->GetUUID(), pv);
    }
}

void RegionTool::OnOptionChanged(const EvToolOption &e)
{
    const int toolId = boost::get<int>(e.toolOptions.at(cp_ToolId));
    if (kSpamID_TOOLBOX_PROBE_REGION == toolId)
    {
        toolOptions = e.toolOptions;
        BoxToolImpl::ResetTool();
    }
}

void RegionTool::OnRegionClicked(const EvEntityClicked &e)
{
    auto spFixed = std::dynamic_pointer_cast<FixedNode>(e.ent);
    if (spFixed)
    {
        spFixed->SetFeatures(boost::get<uint64_t>(toolOptions.at(cp_ToolProbeRegionMask)));
    }
}

void RegionTool::OnRegionBoxed(const EvEntityBoxed &e)
{
    for (auto &ent : e.ents)
    {
        auto spFixed = std::dynamic_pointer_cast<FixedNode>(ent);
        if (spFixed)
        {
            spFixed->SetFeatures(boost::get<uint64_t>(toolOptions.at(cp_ToolProbeRegionMask)));
        }
    }
}

void RegionTool::OnRegionHighlight(const EvEntityHighlight &e)
{
    auto spFixed = std::dynamic_pointer_cast<FixedNode>(e.ent);
    if (spFixed)
    {
        spFixed->SetHighlightFeatures(boost::get<uint64_t>(toolOptions.at(cp_ToolProbeRegionMask)));
    }
}

void RegionTool::OnRegionLoseHighlight(const EvEntityLoseHighlight &e)
{
    auto spFixed = std::dynamic_pointer_cast<FixedNode>(e.ent);
    if (spFixed)
    {
        spFixed->SetHighlightFeatures(0ULL);
    }
}
