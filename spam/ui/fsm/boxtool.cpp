#include "boxtool.h"
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

BoxToolImpl::~BoxToolImpl()
{
}

void BoxToolImpl::StartBoxing(const EvLMouseDown &e)
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

void BoxToolImpl::ContinueBoxing(const EvMouseMove &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);
        Geom::Rect  newRect{ anchor , freePt };
        SpamEntitySelectionMode selMode = Spam::GetSelectionFilter()->GetEntitySelectionMode();
        if (SpamEntitySelectionMode::kESM_MULTIPLE == selMode || SpamEntitySelectionMode::kESM_BOX_SINGLE == selMode)
        {
            cav->DrawBox(rect, newRect);
        }
        rect.emplace(newRect);
    }
}

void BoxToolImpl::EndBoxing(const EvLMouseUp &e)
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
            oldRect.unionWith(selEnt->GetBoundingBox());
        }

        SPDrawableNodeVector newSelEnts;

        if (SpamEntitySelectionMode::kESM_MULTIPLE == Spam::GetSelectionFilter()->GetEntitySelectionMode())
        {
            if (!rect.empty())
            {
                cav->SelectDrawable(*rect, newSelEnts);
            }
            else
            {
                auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
                Geom::Point freePt(imgPt.x, imgPt.y);
                auto s = 1 / cav->GetMatScale();
                SelectionData sd{ SelectionState::kSelNone, HitState::kHsNone, -1, -1, 0 };
                SPDrawableNode fEnt = cav->FindDrawable(freePt, s, s, sd);
                if (fEnt)
                {
                    newSelEnts.push_back(fEnt);
                }
            }
        }
        else
        {
            if (rect.empty())
            {
                auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
                Geom::Point freePt(imgPt.x, imgPt.y);
                auto s = 1 / cav->GetMatScale();
                SelectionData sd{ SelectionState::kSelNone, HitState::kHsNone, -1, -1, 0 };
                SPDrawableNode fEnt = cav->FindDrawable(freePt, s, s, sd);
                if (fEnt)
                {
                    newSelEnts.push_back(fEnt);
                    FireClickEntity(fEnt, e.evData, freePt, sd);
                }
                else
                {
                    cv::Ptr<cv::mvlab::Region> rgn = cav->FindRegion(freePt);
                    if (rgn)
                    {
                        FireClickRegion(e.evData, rgn);
                    }
                    else
                    {
                        cv::Ptr<cv::mvlab::Contour> contr = cav->FindContour(freePt);
                        if (contr)
                        {
                            FireClickContour(e.evData, contr);
                        }
                        else
                        {
                            FireClickImage(e.evData);
                        }
                    }
                }
            }
            else
            {
                FireEndBoxing(rect, e.evData);
            }
        }

        for (SPDrawableNode &selEnt : newSelEnts)
        {
            oldRect.unionWith(selEnt->GetBoundingBox());
        }

        if (e.evData.ControlDown() &&
            SpamEntitySelectionMode::kESM_MULTIPLE == Spam::GetSelectionFilter()->GetEntitySelectionMode())
        {
            SPDrawableNodeVector deselEnts = Spam::Intersection(selEnts, newSelEnts);
            SPDrawableNodeVector newselEnts = Spam::Difference(newSelEnts, deselEnts);
            SPDrawableNodeVector residulselEnts = Spam::Difference(selEnts, deselEnts);

            for (auto &deselEnt : deselEnts)
            {
                deselEnt->ClearSelection();
            }

            for (auto &mewselEnt : newselEnts)
            {
                mewselEnt->Select(toolId);
            }

            if (!deselEnts.empty()) FireDeselectEntity(deselEnts);
            if (!newselEnts.empty()) FireSelectEntity(newselEnts);

            newselEnts.insert(newselEnts.end(), residulselEnts.cbegin(), residulselEnts.cend());
            selEnts.swap(newselEnts);
        }
        else
        {
            SPDrawableNodeVector deselEnts = Spam::Difference(selEnts, newSelEnts);
            SPDrawableNodeVector newselEnts = Spam::Difference(newSelEnts, selEnts);

            for (auto &deselEnt : deselEnts)
            {
                deselEnt->ClearSelection();
            }

            for (auto &mewselEnt : newselEnts)
            {
                mewselEnt->Select(toolId);
            }

            if (!deselEnts.empty()) FireDeselectEntity(deselEnts);
            if (!newselEnts.empty()) FireSelectEntity(newselEnts);

            selEnts.swap(newSelEnts);
        }

        rect.unionWith(oldRect);
        Spam::InvalidateCanvasRect(cav->GetUUID(), rect);
    }

    Spam::RequestRefreshAllCanvases();
    rect = Geom::OptRect();
}

void BoxToolImpl::ResetBoxing(const EvReset &e)
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
            hlRect = highlight->GetBoundingBox();
            FireDimEntity(highlight);
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

void BoxToolImpl::EnterCanvas(const EvCanvasEnter &e)
{

}

void BoxToolImpl::LeaveCanvas(const EvCanvasLeave &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        if (highlight)
        {
            highlight->ClearHighlight();
            cav->DimDrawable(highlight);
        }

        cav->SetCursor(wxCURSOR_ARROW);
    }

    FireDimEntity(highlight);
    highlight.reset();
    ClearHighlightData();
}

void BoxToolImpl::Safari(const EvMouseMove &e)
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
                    FireDimEntity(highlight);
                }

                drawable->SetHighlightData(hlState);
                cav->HighlightDrawable(drawable);
                FireGlowEntity(drawable);
            }
        }
        else
        {
            if (highlight)
            {
                highlight->ClearHighlight();
                cav->DimDrawable(highlight);
                FireDimEntity(highlight);
            }
        }

        highlight = drawable;
        hlData = hlState;
    }
}

void BoxToolImpl::QuitApp(const EvAppQuit &e)
{
    selData.clear();
}

void BoxToolImpl::QuitTool(const EvToolQuit &e)
{
    ResetTool();
}

void BoxToolImpl::ResetTool()
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
                    refreshRect.unionWith(selEnt->GetBoundingBox());
                }

                FireDeselectEntity(selEnts.second.ents);
                Spam::InvalidateCanvasRect(uuid, refreshRect);

                cav->ClearSelectRegions();
                cav->ClearSelectContours();
            }
        }

        Spam::RequestRefreshAllCanvases();
    }

    rect = Geom::OptRect();
    highlight.reset();
    ClearHighlightData();
    selData.clear();
}

void BoxToolImpl::DeleteDrawable(const EvDrawableDelete &e)
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

void BoxToolImpl::SelectDrawable(const EvDrawableSelect &e)
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
                    refreshRect.unionWith(deSelEnt->GetBoundingBox());
                }

                for (SPDrawableNode &newSelEnt : newSelEnts)
                {
                    newSelEnt->Select(toolId);
                    refreshRect.unionWith(newSelEnt->GetBoundingBox());
                }
                ents.swap(thisSelEnts);
                cav->DrawPathVector(Geom::PathVector(), refreshRect);
            }
        }
    }
}

void BoxToolImpl::ClearSelection(const std::string &uuid)
{
    EntitySelection &es = selData[uuid];
    auto &selEnts = es.ents;

    for (SPDrawableNode &selEnt : selEnts)
    {
        selEnt->ClearSelection();
    }

    selEnts.clear();
    es.states.clear();
    es.mementos.clear();
    es.delayDelEnts.clear();
}

void BoxToolImpl::ClearHighlightData() 
{
    hlData.hls = HighlightState::kHlNone;
    hlData.id = -1;
    hlData.subid = -1;
}