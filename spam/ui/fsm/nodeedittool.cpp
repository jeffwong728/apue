#include "nodeedittool.h"
#include <wx/log.h>
#include <wx/dcgraph.h>
#include <ui/spam.h>
#include <ui/toplevel/rootframe.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/drawablenode.h>
#include <ui/projs/geomnode.h>

NodeEditTool::NodeEditTool()
{
    wxLogMessage(wxT("NodeEditTool Enter."));
}

NodeEditTool::~NodeEditTool()
{
    wxLogMessage(wxT("NodeEditTool Quit."));
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

void NodeEditTool::OnBoxingStart(const EvLMouseDown &e)
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

void NodeEditTool::OnBoxing(const EvMouseMove &e)
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

void NodeEditTool::OnBoxingEnd(const EvLMouseUp &e)
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

void NodeEditTool::OnCanvasEnter(const EvCanvasEnter &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
}

void NodeEditTool::OnCanvasLeave(const EvCanvasLeave &e)
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

void NodeEditTool::OnSafari(const EvMouseMove &e)
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

void NodeEditTool::OnAppQuit(const EvAppQuit &e)
{
    selData.clear();
}

void NodeEditTool::OnDrawableDelete(const EvDrawableDelete &e)
{
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (frame)
    {
        for (auto &selEnts : selData)
        {
            const std::string &uuid = selEnts.first;
            CairoCanvas *cav = frame->FindCanvasByUUID(uuid);
            if (cav)
            {
                SPDrawableNodeVector &ents = selEnts.second.ents;
                SPDrawableNodeVector residualEnts = Spam::Difference(ents, e.drawables);
                SPDrawableNodeVector deleteEnts   = Spam::Difference(ents, residualEnts);

                for (SPDrawableNode &delEnt : deleteEnts)
                {
                    delEnt->ClearSelection();
                }

                ents.swap(residualEnts);
            }
        }
    }
}

void NodeEditTool::OnDrawableSelect(const EvDrawableSelect &e)
{
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (frame)
    {
        for (const auto &dra : e.drawables)
        {
            selData[dra->GetParent()->GetUUIDTag()];
        }

        for (auto &selEnts : selData)
        {
            const std::string &uuid = selEnts.first;
            CairoCanvas *cav = frame->FindCanvasByUUID(uuid);
            if (cav)
            {
                SPDrawableNodeVector thisSelEnts;
                for (const auto &dra : e.drawables)
                {
                    if (dra->GetParent()->GetUUIDTag() == uuid)
                    {
                        thisSelEnts.push_back(dra);
                    }
                }

                SPDrawableNodeVector &ents = selEnts.second.ents;
                SPDrawableNodeVector deSelEnts    = Spam::Difference(ents, e.drawables);
                SPDrawableNodeVector stillSelEnts = Spam::Intersection(ents, deSelEnts);
                SPDrawableNodeVector newSelEnts   = Spam::Difference(thisSelEnts, stillSelEnts);

                Geom::OptRect refreshRect;
                for (SPDrawableNode &deSelEnt : deSelEnts)
                {
                    deSelEnt->ClearSelection();

                    Geom::PathVector pv;
                    deSelEnt->BuildPath(pv);
                    refreshRect.unionWith(pv.boundsFast());
                }

                for (SPDrawableNode &newSelEnt : newSelEnts)
                {
                    newSelEnt->SelectEntity();

                    Geom::PathVector pv;
                    newSelEnt->BuildPath(pv);
                    refreshRect.unionWith(pv.boundsFast());
                }
                ents.swap(thisSelEnts);
                cav->DrawPathVector(Geom::PathVector(), refreshRect);
            }
        }
    }
}

void NodeEditTool::ClearSelection(const std::string &uuid)
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

void NodeEditTool::OnBoxingReset(const EvReset &e)
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

void NodeEditTool::OnNodeEditingStart(const EvLMouseDown &e)
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
            selEnt->StartNodeEdit();
        }
    }
}

void NodeEditTool::OnNodeEditing(const EvMouseMove &e)
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
            selEnt->NodeEdit(anchor, freePt, deltPt.x(), deltPt.y());
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

void NodeEditTool::OnNodeEditingEnd(const EvLMouseUp &e)
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

            selEnt->EndNodeEdit();
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
            cav->DoNodeEdit(selEnts, selMementos);
        }

        selStates.clear();
        selMementos.clear();

        cav->DrawPathVector(Geom::PathVector(), refreshRect);
    }

    rect = Geom::OptRect();
}

void NodeEditTool::OnNodeEditingReset(const EvReset &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        EntitySelection &es = selData[cav->GetUUID()];
        auto &selEnts   = es.ents;
        auto &selStates = es.states;
        auto &selMementos = es.mementos;

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
            selEnt->ResetNodeEdit();

            Geom::PathVector opv;
            selEnt->BuildPath(opv);
            refreshRect.unionWith(opv.boundsFast());
        }

        selStates.clear();
        selMementos.clear();

        cav->DrawPathVector(Geom::PathVector(), refreshRect);
    }
}

NodeEditIdle::NodeEditIdle()
{
    wxLogMessage(wxT("NodeEditIdle Enter."));
}

NodeEditIdle::~NodeEditIdle()
{
    wxLogMessage(wxT("NodeEditIdle Quit."));
}

sc::result NodeEditIdle::react(const EvToolQuit &e)
{
    return transit<NoTool>();
}

sc::result NodeEditIdle::react(const EvLMouseDown &e)
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
            EntitySelection &es = context<NodeEditTool>().selData[uuid];
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
                    context<NodeEditTool>().ClearSelection(uuid);
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
                context<NodeEditTool>().ClearSelection(uuid);
                selEnts.push_back(drawable);
                selStates.push_back(sd.ss);

                drawable->selData_ = sd;
            }

            context<Spamer>().sig_EntityDesel(Spam::Difference(oldEnts, selEnts));
            context<Spamer>().sig_EntitySel(Spam::Difference(selEnts, oldEnts));

            cav->DrawPathVector(Geom::PathVector(), refreshRect);
            return transit<NodeEditing>(&NodeEditTool::OnNodeEditingStart, e);
        }
        else
        {
            return transit<NodeEditBoxSelecting>(&NodeEditTool::OnBoxingStart, e);
        }
    }
    else
    {
        return discard_event();
    }
}

NodeEditBoxSelecting::NodeEditBoxSelecting()
{
    wxLogMessage(wxT("NodeEditDraging Enter."));
}

NodeEditBoxSelecting::~NodeEditBoxSelecting()
{
    wxLogMessage(wxT("NodeEditDraging Quit."));
}

NodeEditing::NodeEditing()
{
    wxLogMessage(wxT("NodeEditing Enter."));
}

NodeEditing::~NodeEditing()
{
    wxLogMessage(wxT("NodeEditing Quit."));
}