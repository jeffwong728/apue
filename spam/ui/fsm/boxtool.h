#ifndef SPAM_UI_FSM_BOX_TOOL_H
#define SPAM_UI_FSM_BOX_TOOL_H
#include "spamer.h"
#include <wx/log.h>
#include <wx/dcgraph.h>
#include <ui/projs/modelfwd.h>
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

struct EntitySelection
{
    SPDrawableNodeVector ents;
    SpamMany  mementos;
    std::vector<SelectionData> states;
};

using Selections = std::unordered_map<std::string, EntitySelection>;

template<typename ToolT, int ToolId>
struct BoxTool
{
    BoxTool(ToolT &t) : tool(t){}
    ~BoxTool()
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
                    Geom::OptRect refreshRect;
                    for (SPDrawableNode &selEnt : selEnts.second.ents)
                    {
                        selEnt->ClearSelection();

                        Geom::PathVector pv;
                        selEnt->BuildPath(pv);
                        refreshRect.unionWith(pv.boundsFast());
                    }

                    tool.context<Spamer>().sig_EntityDesel(selEnts.second.ents);
                    cav->DrawPathVector(Geom::PathVector(), refreshRect);
                }
            }
        }
    }

    void StartBoxing(const EvLMouseDown &e)
    {
        CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
        if (cav)
        {
            if (!cav->HasCapture())
            {
                cav->CaptureMouse();
            }

            wxRealPoint imgPt = cav->ScreenToImage(e.evData.GetPosition());
            anchor = Geom::Point(imgPt.x, imgPt.y);
            rect   = Geom::OptRect();
        }
    }

    void ContinueBoxing(const EvMouseMove &e)
    {
        CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
        if (cav)
        {
            auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
            Geom::Point freePt(imgPt.x, imgPt.y);
            Geom::Rect  newRect{ anchor , freePt };
            cav->DrawBox(rect, newRect);
            rect.emplace(newRect);
        }
    }

    void EndBoxing(const EvLMouseUp &e)
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
                selEnt->Select(ToolId);
            }

            tool.context<Spamer>().sig_EntityDesel(Spam::Difference(selEnts, newSelEnts));
            tool.context<Spamer>().sig_EntitySel(Spam::Difference(newSelEnts, selEnts));

            selEnts.swap(newSelEnts);
            rect.unionWith(oldRect);
            cav->DrawBox(rect, Geom::OptRect());
        }

        rect = Geom::OptRect();
    }

    void ResetBoxing(const EvReset &e)
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
                tool.context<Spamer>().sig_EntityDim(highlight);
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

    void EnterCanvas(const EvCanvasEnter &e) {}
    void LeaveCanvas(const EvCanvasLeave &e) 
    {
        CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
        if (cav && highlight)
        {
            highlight->ClearHighlight();
            cav->DimDrawable(highlight);
        }

        tool.context<Spamer>().sig_EntityDim(highlight);
        highlight.reset();
        ClearHighlightData();
    }

    void Safari(const EvMouseMove &e)
    {
        CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
        if (cav)
        {
            auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
            Geom::Point freePt(imgPt.x, imgPt.y);

            auto s = 1 / cav->GetMatScale();
            SelectionData sd{ SelectionState::kSelNone, HitState::kHsNone, -1, -1, 0 };
            auto drawable = cav->FindDrawable(freePt, s, s, sd);
            auto hlState = DrawableNode::MapSelectionToHighlight(sd);
            if (drawable)
            {
                if (drawable != highlight || DrawableNode::IsHighlightChanged(hlState, hlData))
                {
                    if (highlight)
                    {
                        highlight->ClearHighlight();
                        cav->DimDrawable(highlight);
                        tool.context<Spamer>().sig_EntityDim(highlight);
                    }

                    drawable->SetHighlightData(hlState);
                    cav->HighlightDrawable(drawable);
                    tool.context<Spamer>().sig_EntityGlow(drawable);
                }
            }
            else
            {
                if (highlight)
                {
                    highlight->ClearHighlight();
                    cav->DimDrawable(highlight);
                    tool.context<Spamer>().sig_EntityDim(highlight);
                }
            }

            highlight = drawable;
            hlData = hlState;
        }
    }

    void QuitApp(const EvAppQuit &e)
    {
        selData.clear();
    }

    void DeleteDrawable(const EvDrawableDelete &e)
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
                    SPDrawableNodeVector deleteEnts = Spam::Difference(ents, residualEnts);

                    for (SPDrawableNode &delEnt : deleteEnts)
                    {
                        delEnt->ClearSelection();
                    }

                    ents.swap(residualEnts);
                }
            }
        }
    }

    void SelectDrawable(const EvDrawableSelect &e)
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
                    SPDrawableNodeVector deSelEnts = Spam::Difference(ents, e.drawables);
                    SPDrawableNodeVector stillSelEnts = Spam::Intersection(ents, deSelEnts);
                    SPDrawableNodeVector newSelEnts = Spam::Difference(thisSelEnts, stillSelEnts);

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
                        newSelEnt->Select(ToolId);

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

    void ClearSelection(const std::string &uuid)
    {
        EntitySelection &es = selData[uuid];
        auto &selEnts = es.ents;
        auto &selStates = es.states;

        for (SPDrawableNode &selEnt : selEnts)
        {
            selEnt->ClearSelection();
        }

        selEnts.clear();
        selStates.clear();
    }

    void ClearHighlightData() 
    {
        hlData.hls = HighlightState::kHlNone;
        hlData.id = -1;
        hlData.subid = -1;
    }

    Geom::Point    anchor;
    Geom::OptRect  rect;
    SPDrawableNode highlight;
    HighlightData  hlData;
    Selections     selData;

    ToolT &tool;
};

#endif //SPAM_UI_FSM_BOX_TOOL_H