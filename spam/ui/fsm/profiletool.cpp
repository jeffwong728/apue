#include "profiletool.h"
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
        uuids.insert(cav->GetUUID());
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

        cav->UpdateProfileNode(anchor, freePt);

        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        if (frame && newRect.width() > 1 || newRect.height() > 1)
        {
            std::pair<Geom::Point, Geom::Point> lineSeg(anchor, freePt);
            frame->UpdateToolboxUI(kSpamID_TOOLBOX_PROBE, kSpamID_TOOLBOX_PROBE_PROFILE, cav->GetUUID(), lineSeg);
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

        cav->UpdateProfileNode(anchor, freePt);

        auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
        if (frame && newRect.width() > 1 || newRect.height() > 1)
        {
            std::pair<Geom::Point, Geom::Point> lineSeg(anchor, freePt);
            frame->UpdateToolboxUI(kSpamID_TOOLBOX_PROBE, kSpamID_TOOLBOX_PROBE_PROFILE, cav->GetUUID(), lineSeg);
        }
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

        cav->RemoveProfileNode();
    }
}

sc::result ProfileIdle::react(const EvToolQuit &e)
{
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (frame)
    {
        for (const auto &uuid : context<ProfileTool>().uuids)
        {
            CairoCanvas *cav = frame->FindCanvasByUUID(uuid);
            if (cav)
            {
                cav->RemoveProfileNode();
            }
        }
    }
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
