// wxWidgets "Hello World" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include "spamer.h"
#include "probetool.h"
#include "recttool.h"
#include "transformtool.h"
#include "nodeedittool.h"
#include "polygontool.h"
#include <wx/log.h>
#include <ui/spam.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/geomnode.h>
#include <ui/projs/drawablenode.h>
#include <ui/toplevel/rootframe.h>

Spamer::Spamer()
{
}

Spamer::~Spamer()
{
}

void Spamer::OnAppQuit()
{
    process_event(EvAppQuit());
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

void Spamer::OnGeomDelete(const SPModelNodeVector &geoms)
{
    SPDrawableNodeVector dras;
    for (const auto &geom : geoms)
    {
        auto dra = std::dynamic_pointer_cast<DrawableNode>(geom);
        if (dra)
        {
            dras.push_back(dra);
        }
    }
    process_event(EvDrawableDelete(dras));
}

void Spamer::OnDrawableSelect(const SPDrawableNodeVector &dras)
{
    process_event(EvDrawableSelect(dras));
}

NoTool::NoTool() 
{
    wxLogMessage(wxT("NoTool Enter."));
}

NoTool::~NoTool()
{
    wxLogMessage(wxT("NoTool Quit."));
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
                for (SPDrawableNode &selEnt : selEnts.second)
                {
                    selEnt->ClearSelection();

                    Geom::PathVector pv;
                    selEnt->BuildPath(pv);
                    refreshRect.unionWith(pv.boundsFast());
                }

                context<Spamer>().sig_EntityDesel(selEnts.second);
                cav->DrawPathVector(Geom::PathVector(), refreshRect);
            }
        }
    }
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

        SPDrawableNodeVector &selEnts = selData[cav->GetUUID()];

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
            if (fEnt) newSelEnts.push_back(fEnt);
        }

        for (SPDrawableNode &selEnt : newSelEnts)
        {
            Geom::PathVector pv;
            selEnt->BuildPath(pv);
            oldRect.unionWith(pv.boundsFast());
            selEnt->Select(-1);
        }

        context<Spamer>().sig_EntityDesel(Spam::Difference(selEnts, newSelEnts));
        context<Spamer>().sig_EntitySel(Spam::Difference(newSelEnts, selEnts));

        selEnts.swap(newSelEnts);
        rect.unionWith(oldRect);
        cav->DrawBox(rect, Geom::OptRect());
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

void NoTool::OnAppQuit(const EvAppQuit &e)
{
    selData.clear();
}

void NoTool::OnDrawableDelete(const EvDrawableDelete &e)
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
                SPDrawableNodeVector &ents = selEnts.second;
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

void NoTool::OnDrawableSelect(const EvDrawableSelect &e)
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

                SPDrawableNodeVector &ents = selEnts.second;
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
                    newSelEnt->Select(-1);

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
    if (kSpamID_TOOLBOX_GEOM_TRANSFORM == e.toolId)
    {
        return transit<TransformTool>();
    }
    if (kSpamID_TOOLBOX_GEOM_EDIT == e.toolId)
    {
        return transit<NodeEditTool>();
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