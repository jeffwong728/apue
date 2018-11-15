#include "transformtool.h"
#include <wx/log.h>
#include <wx/dcgraph.h>
#include <ui/spam.h>
#include <ui/toplevel/rootframe.h>
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
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (frame)
    {
        for (auto &selEnts : selData)
        {
            const std::string &uuid = selEnts.first;
            CairoCanvas *cav = frame->FindCanvasByUUID(uuid);
            if (cav)
            {
                Geom::OptRect refreshRect;
                for (SPDrawableNode &selEnt : selEnts.second.ents)
                {
                    selEnt->ClearSelection();

                    Geom::PathVector pv;
                    selEnt->BuildPath(pv);
                    refreshRect.unionWith(pv.boundsFast());
                }

                context<Spamer>().sig_EntityDesel(selEnts.second.ents);
                cav->DrawPathVector(Geom::PathVector(), refreshRect);
            }
        }
    }
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

        EntitySelection &es = selData[cav->GetUUID()];
        SPDrawableNodeVector &selEnts = es.ents;

        Geom::OptRect oldRect;
        for (SPDrawableNode &selEnt : selEnts)
        {
            Geom::PathVector pv;
            selEnt->BuildPath(pv);
            oldRect.unionWith(pv.boundsFast());
            selEnt->ClearSelection();
        }

        SPDrawableNodeVector newSelEnts;
        if (!rect.empty())
        {
            cav->SelectDrawable(*rect, newSelEnts);
        }
        else
        {
            auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
            Geom::Point freePt(imgPt.x, imgPt.y);
            auto fEnt = cav->FindDrawable(freePt);
            if (fEnt)
            {
                newSelEnts.push_back(fEnt);
            }
        }

        for (SPDrawableNode &selEnt : newSelEnts)
        {
            Geom::PathVector pv;
            selEnt->BuildPath(pv);
            oldRect.unionWith(pv.boundsFast());
            selEnt->SelectEntity();
        }

        context<Spamer>().sig_EntityDesel(Spam::Difference(selEnts, newSelEnts));
        context<Spamer>().sig_EntitySel(Spam::Difference(newSelEnts, selEnts));

        selEnts.swap(newSelEnts);
        rect.unionWith(oldRect);
        cav->DrawBox(rect, Geom::OptRect());
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
    ClearHighlightData();
}

void TransformTool::OnSafari(const EvMouseMove &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);

        auto s = 1/cav->GetMatScale();
        SelectionData sd{ SelectionState::kSelNone, HitState::kHsNone, -1, -1 };
        auto drawable = cav->FindDrawable(freePt, s, s, sd);
        auto hlState  = DrawableNode::MapSelectionToHighlight(sd);
        if (drawable)
        {
            if (drawable != highlight || DrawableNode::IsHighlightChanged(hlState, hlData))
            {
                if (highlight)
                {
                    highlight->ClearHighlight();
                    cav->DimDrawable(highlight);
                    context<Spamer>().sig_EntityDim(highlight);
                }

                drawable->SetHighlight(hlState);
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
        hlData    = hlState;
    }
}

void TransformTool::OnAppQuit(const EvAppQuit &e)
{
    selData.clear();
}

void TransformTool::ClearSelection(const std::string &uuid)
{
    EntitySelection &es = selData[uuid];
    auto &selEnts = es.ents;
    auto &selStates = es.states;

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
    ClearHighlightData();
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

        EntitySelection &es = selData[cav->GetUUID()];
        SPDrawableNodeVector &selEnts = es.ents;
        SpamMany             &selmementos = es.mementos;
        selmementos.clear();

        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        anchor = Geom::Point(imgPt.x, imgPt.y);
        last = anchor;
        rect = Geom::OptRect();

        for (SPDrawableNode &selEnt : selEnts)
        {
            selmementos.push_back(selEnt->CreateMemento());
            selEnt->StartTransform();
        }
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

        EntitySelection &es = selData[cav->GetUUID()];
        SPDrawableNodeVector &selEnts = es.ents;

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

        EntitySelection &es = selData[cav->GetUUID()];
        auto &selEnts     = es.ents;
        auto &selStates   = es.states;
        auto &selMementos = es.mementos;

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

            selEnt->EndTransform();
        }

        if (rect.empty())
        {
            for (SPDrawableNode &selEnt : selEnts)
            {
                selEnt->SwitchSelectionState();
            }
        }
        else
        {
            cav->DoTransform(selEnts, selMementos);
        }

        cav->DrawPathVector(Geom::PathVector(), refreshRect);
    }

    rect = Geom::OptRect();
}

void TransformTool::OnTransformingReset(const EvReset &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        EntitySelection &es = selData[cav->GetUUID()];
        auto &selEnts   = es.ents;
        auto &selStates = es.states;

        int s = 0;
        Geom::OptRect refreshRect;
        for (SPDrawableNode &selEnt : selEnts)
        {
            Geom::PathVector cpv;
            selEnt->BuildPath(cpv);
            refreshRect.unionWith(cpv.boundsFast());

            selEnt->selData_.ss = selStates[s++];
            selEnt->selData_.hs = HitState::kHsNone;
            selEnt->selData_.id = -1;
            selEnt->selData_.subid = -1;
            selEnt->ResetTransform();

            Geom::PathVector opv;
            selEnt->BuildPath(opv);
            refreshRect.unionWith(opv.boundsFast());
        }

        cav->DrawPathVector(Geom::PathVector(), refreshRect);
    }
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

        auto s = 1/cav->GetMatScale();
        SelectionData sd{ SelectionState::kSelNone, HitState::kHsNone, -1, -1 };
        SPDrawableNode drawable = cav->FindDrawable(freePt, s, s, sd);

        if (drawable)
        {
            const std::string &uuid = cav->GetUUID();
            EntitySelection &es = context<TransformTool>().selData[uuid];
            auto &selEnts = es.ents;
            auto &selStates = es.states;
            SPDrawableNodeVector oldEnts = selEnts;

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
                    context<TransformTool>().ClearSelection(uuid);
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
                context<TransformTool>().ClearSelection(uuid);
                selEnts.push_back(drawable);
                selStates.push_back(sd.ss);

                drawable->selData_ = sd;
            }

            context<Spamer>().sig_EntityDesel(Spam::Difference(oldEnts, selEnts));
            context<Spamer>().sig_EntitySel(Spam::Difference(selEnts, oldEnts));

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