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

void ProfileTool::OnStartDraging(const EvLMouseDown &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        if (!cav->HasCapture())
        {
            cav->CaptureMouse();
        }

        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        anchor = Geom::Point(imgPt.x, imgPt.y);
        rect = Geom::OptRect();
    }
}

void ProfileTool::OnDraging(const EvMouseMove &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);
        Geom::Rect newRect{ anchor , freePt };

        Geom::PathVector pv{ Geom::Path(newRect) };
        auto brect = pv.boundsFast();
        brect.unionWith(rect);

        pv.clear();
        Geom::PathBuilder pb(pv);
        pb.moveTo(anchor);
        pb.lineTo(freePt);
        pb.flush();
        cav->DrawPathVector(pv, brect);
        rect.emplace(newRect);

        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        if (frame && newRect.width() > 1 || newRect.height() > 1)
        {
            frame->RequestUpdateProfile(cav->GetUUID(), anchor, freePt);
        }
    }
}

void ProfileTool::OnTracing(const EvMouseMove &e)
{
    OnDraging(e);
}

void ProfileTool::OnEndDraging(const EvLMouseUp &e)
{
    EndDraging(e.evData);
}

void ProfileTool::OnEndTracing(const EvLMouseDown &e)
{
    EndTracing(e.evData);
}

void ProfileTool::EndDraging(const wxMouseEvent &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.GetEventObject());
    if (cav)
    {
        if (cav->HasCapture())
        {
            cav->ReleaseMouse();
        }

        auto imgPt = cav->ScreenToImage(e.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);
        Geom::Rect newRect{ anchor , freePt };

        Geom::PathVector pv{ Geom::Path(newRect) };
        auto brect = pv.boundsFast();
        brect.unionWith(rect);

        pv.clear();
        Geom::PathBuilder pb(pv);
        pb.moveTo(anchor);
        pb.lineTo(freePt);
        pb.flush();
        cav->DrawPathVector(Geom::PathVector(), brect);

        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        if (frame && newRect.width() > 1 || newRect.height() > 1)
        {
            frame->RequestUpdateProfile(cav->GetUUID(), anchor, freePt);
        }

        rect = Geom::OptRect();
    }
}
void ProfileTool::EndTracing(const wxMouseEvent &e)
{
    EndDraging(e);
}

void ProfileTool::OnCanvasEnter(const EvCanvasEnter &WXUNUSED(e))
{
}

void ProfileTool::OnCanvasLeave(const EvCanvasLeave &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        cav->SetCursor(wxCURSOR_ARROW);
    }
}

void ProfileTool::OnReset(const EvReset &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        if (cav->HasCapture())
        {
            cav->ReleaseMouse();
        }

        if (!rect.empty())
        {
            cav->DrawPathVector(Geom::PathVector(), rect);
            rect = Geom::OptRect();
        }
    }
}

sc::result ProfileIdle::react(const EvToolQuit &e)
{
    return transit<NoTool>();
}

sc::result ProfileDraging::react(const EvLMouseUp &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);
        Geom::Rect rect{ context<ProfileTool>().anchor , freePt };
        if (rect.width() < 3 && rect.height() < 3)
        {
            return transit<ProfileTracing>();
        }
        else
        {
            return transit<ProfileIdle>(&ProfileTool::OnEndDraging, e);
        }
    }
    else
    {
        return discard_event();
    }
}
