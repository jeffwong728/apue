#include "transformtool.h"
#include <wx/log.h>
#include <wx/dcgraph.h>
#include <ui/spam.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/drawablenode.h>
#include <ui/projs/geomnode.h>

TransformTool::TransformTool()
{
    wxLogMessage(wxT("TransformTool Enter."));
}

TransformTool::~TransformTool()
{
    wxLogMessage(wxT("TransformTool Quit."));
}

void TransformTool::OnBoxingStart(const EvLMouseDown &e)
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

void TransformTool::OnBoxing(const EvMouseMove &e)
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

void TransformTool::OnBoxingEnd(const EvLMouseUp &e)
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
            Geom::OptRect oldRect;
            for (SPDrawableNode &selEnt : selEnts)
            {
                Geom::PathVector pv;
                selEnt->BuildPath(pv);
                oldRect.unionWith(pv.boundsFast());
                selEnt->ClearSelection();
            }

            selEnts.clear();
            cav->SelectDrawable(*rect, selEnts);

            for (SPDrawableNode &selEnt : selEnts)
            {
                Geom::PathVector pv;
                selEnt->BuildPath(pv);
                oldRect.unionWith(pv.boundsFast());
                selEnt->SelectEntity();
            }

            rect.unionWith(oldRect);
            cav->DrawBox(rect, Geom::OptRect());
        }
        else
        {
            Geom::OptRect oldRect;
            for (SPDrawableNode &selEnt : selEnts)
            {
                Geom::PathVector pv;
                selEnt->BuildPath(pv);
                oldRect.unionWith(pv.boundsFast());
                selEnt->ClearSelection();
            }

            selEnts.clear();
            auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
            Geom::Point freePt(imgPt.x, imgPt.y);
            auto fEnt = cav->FindDrawable(freePt);
            if (fEnt)
            {
                selEnts.push_back(fEnt);
            }

            for (SPDrawableNode &selEnt : selEnts)
            {
                Geom::PathVector pv;
                selEnt->BuildPath(pv);
                oldRect.unionWith(pv.boundsFast());
                selEnt->SelectEntity();
            }

            rect.unionWith(oldRect);
            cav->DrawBox(rect, Geom::OptRect());
        }
    }

    rect = Geom::OptRect();
}

void TransformTool::OnCanvasEnter(const EvCanvasEnter &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
}

void TransformTool::OnCanvasLeave(const EvCanvasLeave &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav && highlight)
    {
        highlight->ClearHighlight();
        cav->DimDrawable(highlight);
    }

    context<Spamer>().sig_EntityDim(highlight);
    highlight.reset();
}

void TransformTool::OnSafari(const EvMouseMove &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);

        auto drawable = cav->FindDrawable(freePt);
        if (drawable)
        {
            if (drawable != highlight)
            {
                if (highlight)
                {
                    highlight->ClearHighlight();
                    cav->DimDrawable(highlight);
                    context<Spamer>().sig_EntityDim(highlight);
                }

                drawable->HighlightFace();
                cav->HighlightDrawable(drawable);
                context<Spamer>().sig_EntityGlow(drawable);
            }
        }
        else
        {
            if (highlight)
            {
                highlight->ClearHighlight();
                cav->DimDrawable(highlight);
                context<Spamer>().sig_EntityDim(highlight);
            }
        }

        highlight = drawable;
    }
}

void TransformTool::OnBoxingReset(const EvReset &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        if (cav->HasCapture())
        {
            cav->ReleaseMouse();
        }

        Geom::OptRect hlRect;
        if (highlight)
        {
            highlight->ClearHighlight();
            Geom::PathVector pv;
            highlight->BuildPath(pv);
            hlRect = pv.boundsFast();
            context<Spamer>().sig_EntityDim(highlight);
        }

        if (!rect.empty())
        {
            rect.unionWith(hlRect);
            cav->DrawBox(rect, Geom::OptRect());
        }
    }

    rect = Geom::OptRect();
    highlight.reset();
}

void TransformTool::OnTransformingStart(const EvLMouseDown &e)
{

}

void TransformTool::OnTransforming(const EvMouseMove &e)
{

}

void TransformTool::OnTransformingEnd(const EvLMouseUp &e)
{

}

void TransformTool::OnTransformingReset(const EvReset &e)
{

}

TransformIdle::TransformIdle()
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
    return transit<TransformBoxSelecting>(&TransformTool::OnBoxingStart, e);
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