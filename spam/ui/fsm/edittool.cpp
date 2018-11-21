#include "edittool.h"
#include <ui/spam.h>
#include <ui/toplevel/rootframe.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/drawablenode.h>
#include <ui/projs/geomnode.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )
#include <unordered_map>

void EditToolImpl::StartEditing(const EvLMouseDown &e)
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
            selEnt->StartEdit(toolId);
        }
    }
}

void EditToolImpl::ContinueEditing(const EvMouseMove &e)
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
            selEnt->Edit(toolId, anchor, freePt, deltPt.x(), deltPt.y());
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

void EditToolImpl::EndEditing(const EvLMouseUp &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        if (cav->HasCapture())
        {
            cav->ReleaseMouse();
        }

        EntitySelection &es = selData[cav->GetUUID()];
        auto &selEnts = es.ents;
        auto &selStates = es.states;
        auto &selMementos = es.mementos;
        auto &delayDelEnts = es.delayDelEnts;

        int s = 0;
        Geom::OptRect refreshRect;
        for (SPDrawableNode &selEnt : selEnts)
        {
            selEnt->SetSelectionData(selStates[s++]);

            Geom::PathVector pv;
            selEnt->BuildPath(pv);
            refreshRect.unionWith(pv.boundsFast());

            selEnt->EndEdit(toolId);
        }

        if (rect.empty())
        {
            for (auto &delayDelEnt : delayDelEnts)
            {
                delayDelEnt->ClearSelection();
            }

            FireDeselectEntity(delayDelEnts);

            selEnts = Spam::Difference(selEnts, delayDelEnts);
            for (SPDrawableNode &selEnt : selEnts)
            {
                selEnt->SwitchSelectionState(toolId);
            }
        }
        else
        {
            cav->DoEdit(toolId, selEnts, selMementos);
        }

        selStates.clear();
        selMementos.clear();
        delayDelEnts.clear();

        cav->DrawPathVector(Geom::PathVector(), refreshRect);
    }

    rect = Geom::OptRect();
}

void EditToolImpl::ResetEditing(const EvReset &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        EntitySelection &es = selData[cav->GetUUID()];
        auto &selEnts = es.ents;
        auto &selStates = es.states;

        int s = 0;
        Geom::OptRect refreshRect;
        for (SPDrawableNode &selEnt : selEnts)
        {
            Geom::PathVector cpv;
            selEnt->BuildPath(cpv);
            refreshRect.unionWith(cpv.boundsFast());

            selEnt->SetSelectionData(selStates[s++]);
            selEnt->ResetEdit(toolId);

            Geom::PathVector opv;
            selEnt->BuildPath(opv);
            refreshRect.unionWith(opv.boundsFast());
        }

        selStates.clear();
        es.mementos.clear();
        es.delayDelEnts.clear();

        cav->DrawPathVector(Geom::PathVector(), refreshRect);
    }
}

EditIdleImpl::NextAction EditIdleImpl::GetNextAction(const EvLMouseDown &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);

        auto s = 1 / cav->GetMatScale();
        SelectionData sd{ SelectionState::kSelNone, HitState::kHsNone, -1, -1, 0 };
        SPDrawableNode drawable = cav->FindDrawable(freePt, s, s, sd);

        if (drawable)
        {
            const std::string &uuid = cav->GetUUID();
            EntitySelection &es = GetSelections(uuid);
            auto &selEnts = es.ents;
            auto &selStates = es.states;
            auto &delayDelEnts = es.delayDelEnts;
            SPDrawableNodeVector oldEnts = selEnts;
            delayDelEnts.clear();

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

                    if (!e.evData.ControlDown())
                    {
                        ClearSelections(uuid);
                    }

                    selEnts.push_back(drawable);
                    drawable->SetSelectionData({ DrawableNode::GetInitialSelectState(toolId), HitState::kHsFace, 0, 0, 0 });
                }
                else
                {
                    if (e.evData.ControlDown())
                    {
                        delayDelEnts.push_back(drawable);
                    }
                    else
                    {
                        SelectionData sd = drawable->GetSelectionData();
                        ClearSelections(uuid);
                        drawable->SetSelectionData(sd);
                        selEnts.push_back(drawable);
                    }
                }

                selStates.clear();
                for (const SPDrawableNode &selEnt : selEnts)
                {
                    selStates.push_back(selEnt->GetSelectionData());
                    selEnt->SetSelectionData({ SelectionState::kSelState, HitState::kHsFace, 0, 0, 0 });
                }
            }
            else
            {
                sd.master = drawable->GetSelectionData().master;
                ClearSelections(uuid);
                selEnts.push_back(drawable);
                selStates.push_back(sd);

                drawable->SetSelectionData(sd);
            }

            FireDeselectEntity(Spam::Difference(oldEnts, selEnts));
            FireSelectEntity(Spam::Difference(selEnts, oldEnts));

            cav->DrawPathVector(Geom::PathVector(), refreshRect);
            return editing;
        }
        else
        {
            return boxing;
        }
    }
    else
    {
        return discard;
    }
}