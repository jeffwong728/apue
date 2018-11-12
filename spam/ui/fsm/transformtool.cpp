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

void TransformTool::ClearSelection()
{
    for (SPDrawableNode &selEnt : selEnts)
    {
        selEnt->selData_.ss = SelectionState::kSelNone;
        selEnt->selData_.hs = HitState::kHsNone;
        selEnt->selData_.id = -1;
        selEnt->selData_.subid = -1;
    }

    selEnts.clear();
    selStates.clear();
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
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        if (!cav->HasCapture())
        {
            cav->CaptureMouse();
        }

        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        anchor = Geom::Point(imgPt.x, imgPt.y);
        last = anchor;
        rect = Geom::OptRect();
    }
}

void TransformTool::OnTransforming(const EvMouseMove &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);
        Geom::Rect newRect{ anchor , freePt };
        Geom::Point deltPt = freePt - last;

        Geom::OptRect refreshRect;
        for (SPDrawableNode &selEnt : selEnts)
        {
            Geom::PathVector pv;
            selEnt->BuildPath(pv);
            refreshRect.unionWith(pv.boundsFast());
            selEnt->Transform(anchor, freePt, deltPt.x(), deltPt.y());
        }

        for (SPDrawableNode &selEnt : selEnts)
        {
            Geom::PathVector pv;
            selEnt->BuildPath(pv);
            refreshRect.unionWith(pv.boundsFast());
        }

        cav->DrawPathVector(Geom::PathVector(), refreshRect);

        last = freePt;
        rect.emplace(newRect);
    }
}

void TransformTool::OnTransformingEnd(const EvLMouseUp &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        if (cav->HasCapture())
        {
            cav->ReleaseMouse();
        }

        auto &selEnts = context<TransformTool>().selEnts;
        auto &selStates = context<TransformTool>().selStates;

        int s = 0;
        Geom::OptRect refreshRect;
        for (SPDrawableNode &selEnt : selEnts)
        {
            selEnt->selData_.ss = selStates[s++];
            selEnt->selData_.hs = HitState::kHsNone;
            selEnt->selData_.id = -1;
            selEnt->selData_.subid = -1;

            Geom::PathVector pv;
            selEnt->BuildPath(pv);
            refreshRect.unionWith(pv.boundsFast());
        }

        if (rect.empty())
        {
            for (SPDrawableNode &selEnt : selEnts)
            {
                selEnt->SwitchSelectionState();
            }
        }

        cav->DrawPathVector(Geom::PathVector(), refreshRect);
    }

    rect = Geom::OptRect();
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
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);

        auto s = cav->GetMatScale();
        SelectionData sd{ SelectionState::kSelNone, HitState::kHsNone, -1, -1 };
        SPDrawableNode drawable = cav->FindDrawable(freePt, s, s, sd);

        if (drawable)
        {
            auto &selEnts   = context<TransformTool>().selEnts;
            auto &selStates = context<TransformTool>().selStates;

            Geom::PathVector dpv;
            drawable->BuildPath(dpv);
            Geom::OptRect refreshRect = dpv.boundsFast();
            for (const SPDrawableNode &selEnt : selEnts)
            {
                Geom::PathVector pv;
                selEnt->BuildPath(pv);
                refreshRect.unionWith(pv.boundsFast());
            }

            if (sd.hs == HitState::kHsFace)
            {
                bool newSel = true;
                for (const SPDrawableNode &oldSelEnt : selEnts)
                {
                    if (drawable == oldSelEnt)
                    {
                        newSel = false; break;
                    }
                }

                if (newSel)
                {
                    context<TransformTool>().ClearSelection();
                    selEnts.push_back(drawable);
                    selStates.push_back(SelectionState::kSelScale);
                }
                else
                {
                    selStates.clear();
                    for (const SPDrawableNode &selEnt : selEnts)
                    {
                        selStates.push_back(selEnt->selData_.ss);
                    }
                }

                for (SPDrawableNode &selEnt : selEnts)
                {
                    selEnt->selData_.ss    = SelectionState::kSelState;
                    selEnt->selData_.hs    = HitState::kHsFace;
                    selEnt->selData_.id    = 0;
                    selEnt->selData_.subid = 0;
                }
            }
            else
            {
                context<TransformTool>().ClearSelection();
                selEnts.push_back(drawable);
                selStates.push_back(sd.ss);

                drawable->selData_ = sd;
            }

            cav->DrawPathVector(Geom::PathVector(), refreshRect);
            return transit<Transforming>(&TransformTool::OnTransformingStart, e);
        }
        else
        {
            return transit<TransformBoxSelecting>(&TransformTool::OnBoxingStart, e);
        }
    }
    else
    {
        return discard_event();
    }
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