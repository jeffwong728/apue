#ifndef SPAM_UI_FSM_EDIT_TOOL_H
#define SPAM_UI_FSM_EDIT_TOOL_H
#include "boxtool.h"

template<typename ToolT, int ToolId>
struct EditTool : BoxTool<ToolT, ToolId>
{
    EditTool(ToolT &t) : BoxTool<ToolT, ToolId>(t) {}
    ~EditTool() {}

    void StartEditing(const EvLMouseDown &e)
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
                selEnt->StartEdit(ToolId);
            }
        }
    }

    void ContinueEditing(const EvMouseMove &e)
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
                selEnt->Edit(ToolId, anchor, freePt, deltPt.x(), deltPt.y());
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

    void EndEditing(const EvLMouseUp &e)
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

            int s = 0;
            Geom::OptRect refreshRect;
            for (SPDrawableNode &selEnt : selEnts)
            {
                selEnt->SetSelectionData(selStates[s++]);

                Geom::PathVector pv;
                selEnt->BuildPath(pv);
                refreshRect.unionWith(pv.boundsFast());

                selEnt->EndEdit(ToolId);
            }

            if (rect.empty())
            {
                for (SPDrawableNode &selEnt : selEnts)
                {
                    selEnt->SwitchSelectionState(ToolId);
                }
            }
            else
            {
                cav->DoEdit(ToolId, selEnts, selMementos);
            }

            selStates.clear();
            selMementos.clear();

            cav->DrawPathVector(Geom::PathVector(), refreshRect);
        }

        rect = Geom::OptRect();
    }

    void ResetEditing(const EvReset &e)
    {
        CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
        if (cav)
        {
            EntitySelection &es = selData[cav->GetUUID()];
            auto &selEnts = es.ents;
            auto &selStates = es.states;
            auto &selMementos = es.mementos;

            int s = 0;
            Geom::OptRect refreshRect;
            for (SPDrawableNode &selEnt : selEnts)
            {
                Geom::PathVector cpv;
                selEnt->BuildPath(cpv);
                refreshRect.unionWith(cpv.boundsFast());

                selEnt->SetSelectionData(selStates[s++]);
                selEnt->ResetEdit(ToolId);

                Geom::PathVector opv;
                selEnt->BuildPath(opv);
                refreshRect.unionWith(opv.boundsFast());
            }

            selStates.clear();
            selMementos.clear();

            cav->DrawPathVector(Geom::PathVector(), refreshRect);
        }
    }

    Geom::Point last;
};

template<typename IdleT, typename ToolT, typename EditingT, typename BoxingT, int ToolId>
struct EditIdle
{
    EditIdle(IdleT &idl) : idle(idl) {}

    sc::result reactLMouseDown(const EvLMouseDown &e)
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
                EntitySelection &es = idle.context<ToolT>().selData[uuid];
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
                        idle.context<ToolT>().ClearSelection(uuid);
                        selEnts.push_back(drawable);
                        selStates.push_back({ DrawableNode::GetInitialSelectState(ToolId), HitState::kHsFace, 0, 0, 0 });
                    }
                    else
                    {
                        selStates.clear();
                        for (const SPDrawableNode &selEnt : selEnts)
                        {
                            selStates.push_back(selEnt->GetSelectionData());
                        }
                    }

                    for (SPDrawableNode &selEnt : selEnts)
                    {
                        selEnt->SetSelectionData({ SelectionState::kSelState, HitState::kHsFace, 0, 0, 0 });
                    }
                }
                else
                {
                    sd.master = drawable->GetSelectionData().master;
                    idle.context<ToolT>().ClearSelection(uuid);
                    selEnts.push_back(drawable);
                    selStates.push_back(sd);

                    drawable->SetSelectionData(sd);
                }

                idle.context<Spamer>().sig_EntityDesel(Spam::Difference(oldEnts, selEnts));
                idle.context<Spamer>().sig_EntitySel(Spam::Difference(selEnts, oldEnts));

                cav->DrawPathVector(Geom::PathVector(), refreshRect);
                return idle.transit<EditingT>(&ToolT::EditToolT::StartEditing, e);
            }
            else
            {
                return idle.transit<BoxingT>(&ToolT::BoxToolT::StartBoxing, e);
            }
        }
        else
        {
            return idle.discard_event();
        }
    }

    IdleT &idle;
};

#endif //SPAM_UI_FSM_EDIT_TOOL_H