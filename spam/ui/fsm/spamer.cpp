// wxWidgets "Hello World" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include "spamer.h"
#include "probetool.h"
#include "recttool.h"
#include "transformtool.h"
#include "polygontool.h"
#include <wx/log.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/drawablenode.h>

Spamer::Spamer()
{
}

Spamer::~Spamer()
{
}

void Spamer::OnToolEnter(int toolId)
{
    process_event(EvToolEnter(toolId));
}

void Spamer::OnToolQuit(int toolId)
{
    process_event(EvToolQuit(toolId));
}

void Spamer::OnCanvasEnter(wxMouseEvent &e)
{
    process_event(EvCanvasEnter(e));
}

void Spamer::OnCanvasLeave(wxMouseEvent &e)
{
    process_event(EvCanvasLeave(e));
}

void Spamer::OnCanvasLeftMouseDown(wxMouseEvent &e)
{
    process_event(EvLMouseDown(e));
}

void Spamer::OnCanvasLeftMouseUp(wxMouseEvent &e)
{
    process_event(EvLMouseUp(e));
}

void Spamer::OnCanvasMouseMotion(wxMouseEvent &e)
{
    process_event(EvMouseMove(e));
}

void Spamer::OnCanvasLeftDClick(wxMouseEvent &e)
{
    process_event(EvLMouseDClick(e));
}

void Spamer::OnCanvasMiddleDown(wxMouseEvent &e)
{
    process_event(EvMMouseDown(e));
}

void Spamer::OnCanvasKeyDown(wxKeyEvent &e)
{

}

void Spamer::OnCanvasKeyUp(wxKeyEvent &e)
{

}

void Spamer::OnCanvasChar(wxKeyEvent &e)
{
    if (WXK_ESCAPE == e.GetKeyCode())
    {
        process_event(EvReset(e));
    }
}

NoTool::NoTool() 
{
    wxLogMessage(wxT("NoTool Enter."));
}

NoTool::~NoTool()
{
    wxLogMessage(wxT("NoTool Quit."));
}

void NoTool::OnStartDraging(const EvLMouseDown &e)
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

void NoTool::OnDraging(const EvMouseMove &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);
        Geom::Rect newRect{ anchor , freePt };
        cav->DrawBox(rect, newRect);
        rect.emplace(newRect);
    }
}

void NoTool::OnEndDraging(const EvLMouseUp &e)
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
            cav->DrawBox(rect, Geom::OptRect());
        }
    }

    rect = Geom::OptRect();
}

void NoTool::OnReset(const EvReset &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        if (cav->HasCapture())
        {
            cav->ReleaseMouse();
        }

        Geom::OptRect hlRect;
        if (highlight_)
        {
            highlight_->ClearHighlight();
            Geom::PathVector pv;
            highlight_->BuildPath(pv);
            hlRect = pv.boundsFast();
            context<Spamer>().sig_EntityDim(highlight_);
        }

        if (!rect.empty())
        {
            rect.unionWith(hlRect);
            cav->DrawBox(rect, Geom::OptRect());
        }
    }

    rect = Geom::OptRect();
    highlight_.reset();
}

void NoTool::OnSafari(const EvMouseMove &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);

        auto drawable = cav->FindDrawable(freePt);
        if (drawable)
        {
            if (drawable != highlight_)
            {
                if (highlight_)
                {
                    highlight_->ClearHighlight();
                    cav->DimDrawable(highlight_);
                    context<Spamer>().sig_EntityDim(highlight_);
                }

                drawable->HighlightFace();
                cav->HighlightDrawable(drawable);
                context<Spamer>().sig_EntityGlow(drawable);
            }
        }
        else
        {
            if (highlight_)
            {
                highlight_->ClearHighlight();
                cav->DimDrawable(highlight_);
                context<Spamer>().sig_EntityDim(highlight_);
            }
        }

        highlight_ = drawable;
    }
}

void NoTool::OnCanvasLeave(const EvCanvasLeave &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav && highlight_)
    {
        highlight_->ClearHighlight();
        cav->DimDrawable(highlight_);
        context<Spamer>().sig_EntityDim(highlight_);
    }

    highlight_.reset();
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
    if (kSpamID_TOOLBOX_PROBE_SELECT == e.toolId)
    {
        return transit<ProbeTool>();
    }
    if (kSpamID_TOOLBOX_GEOM_RECT == e.toolId)
    {
        return transit<RectTool>();
    }
    if (kSpamID_TOOLBOX_GEOM_POLYGON == e.toolId)
    {
        return transit<PolygonTool>();
    }
    if (kSpamID_TOOLBOX_GEOM_SELECT == e.toolId)
    {
        return transit<TransformTool>();
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