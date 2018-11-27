#include "beziergontool.h"
#include <wx/log.h>
#include <wx/dcgraph.h>
#include <ui/spam.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/beziergonnode.h>

BeziergonTool::BeziergonTool()
    : beziergon(std::make_shared<BeziergonNode>())
{
    wxLogMessage(wxT("BeziergonTool Enter."));
}

BeziergonTool::~BeziergonTool()
{
    wxLogMessage(wxT("BeziergonTool Quit."));
}

void BeziergonTool::OnTracing(const EvMouseMove &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);

        Geom::PathVector opv;
        beziergon->BuildTracingPath(opv);

        beziergon->PopCorner();
        beziergon->AddCorner(freePt, freePt, freePt, BezierNodeType::kBezierNoneCtrl);

        Geom::PathVector pv;
        beziergon->BuildTracingPath(pv);

        auto orect = opv.boundsFast();
        auto rect = pv.boundsFast();
        rect.unionWith(orect);
        cav->DrawPathVector(pv, rect);
    }
}

void BeziergonTool::CompleteCreate(const wxMouseEvent &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.GetEventObject());
    if (cav)
    {
        if (cav->HasCapture())
        {
            cav->ReleaseMouse();
        }

        auto imgPt = cav->ScreenToImage(e.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);

        Geom::PathVector opv;
        beziergon->BuildTracingPath(opv);

        beziergon->PopCorner();
        beziergon->AddCorner(freePt, freePt, freePt, BezierNodeType::kBezierNoneCtrl);
        beziergon->Collapse();

        Geom::PathVector pv;
        if (beziergon->GetNumCorners()>2)
        {
            cav->AddBeziergon(beziergon->GetData());
            beziergon->BuildPath(pv);
        }

        auto orect = opv.boundsFast();
        auto rect = pv.boundsFast();
        rect.unionWith(orect);
        cav->DrawPathVector(pv, rect);
    }

    beziergon->Clear();
}

void BeziergonTool::OnCanvasEnter(const EvCanvasEnter &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
}

void BeziergonTool::OnCanvasLeave(const EvCanvasLeave &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
}

void BeziergonTool::OnInitDraging(const EvLMouseDown &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        if (!cav->HasCapture())
        {
            cav->CaptureMouse();
        }

        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);

        beziergon->AddCorner(freePt, freePt, freePt, BezierNodeType::kBezierNextCtrl);
    }
}

void BeziergonTool::OnStartDraging(const EvLMouseDown &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);

        Geom::Point corner, c0, c1;
        BezierNodeType t = beziergon->GetCorner(beziergon->GetNumCorners() - 1, corner, c0, c1);
        c0 = Geom::lerp(2, freePt, corner);

        Geom::PathVector opv;
        beziergon->BuildTracingPath(opv);

        beziergon->ReplaceBackCorner(corner, c0, freePt, BezierNodeType::kBezierNoneCtrl);

        Geom::PathVector pv;
        beziergon->BuildDragingPath(pv);

        auto orect = opv.boundsFast();
        auto rect = pv.boundsFast();
        rect.unionWith(orect);

        cav->DrawPathVector(pv, rect);
    }
}

void BeziergonTool::OnDraging(const EvMouseMove &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);

        Geom::Point corner, c0, c1;
        beziergon->GetCorner(beziergon->GetNumCorners()-1, corner, c0, c1);
        c0 = Geom::lerp(2, freePt, corner);

        Geom::PathVector opv;
        beziergon->BuildDragingPath(opv);

        beziergon->ReplaceBackCorner(corner, c0, freePt, BezierNodeType::kBezierBothCtrl);

        Geom::PathVector pv;
        beziergon->BuildDragingPath(pv);

        auto orect = opv.boundsFast();
        auto rect = pv.boundsFast();
        rect.unionWith(orect);

        cav->DrawPathVector(pv, rect);
    }
}

void BeziergonTool::OnEndDraging(const EvLMouseUp &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);

        Geom::Point corner, c0, c1;
        BezierNodeType t = beziergon->GetCorner(beziergon->GetNumCorners() - 1, corner, c0, c1);
        c0 = Geom::lerp(2, freePt, corner);

        if (Geom::distanceSq(freePt, corner) < Geom::EPSILON)
        {
            t = BezierNodeType::kBezierNoneCtrl;
        }

        Geom::PathVector opv;
        beziergon->BuildDragingPath(opv);

        beziergon->ReplaceBackCorner(corner, c0, freePt, t);
        beziergon->AddCorner(freePt, c1, freePt, BezierNodeType::kBezierNoneCtrl);

        Geom::PathVector pv;
        beziergon->BuildTracingPath(pv);

        auto orect = opv.boundsFast();
        auto rect = pv.boundsFast();
        rect.unionWith(orect);

        cav->DrawPathVector(pv, rect);
    }
}

void BeziergonTool::OnAddCorner(const EvLMouseDown &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);
    }
}

void BeziergonTool::OnMMouseDown(const EvMMouseDown &e)
{
    CompleteCreate(e.evData);
}

void BeziergonTool::OnLMouseDClick(const EvLMouseDClick &e)
{
    CompleteCreate(e.evData);
}

void BeziergonTool::OnReset(const EvReset &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        if (cav->HasCapture())
        {
            cav->ReleaseMouse();
        }
    }
}

BeziergonIdle::BeziergonIdle()
{
    wxLogMessage(wxT("BeziergonIdle Enter."));
}

BeziergonIdle::~BeziergonIdle()
{
    wxLogMessage(wxT("BeziergonIdle Quit."));
}

sc::result BeziergonIdle::react(const EvToolQuit &e)
{
    return transit<NoTool>();
}

BeziergonTracing::BeziergonTracing()
{
    wxLogMessage(wxT("BeziergonTracing Enter."));
}

BeziergonTracing::~BeziergonTracing()
{
    wxLogMessage(wxT("BeziergonTracing Quit."));
}

BeziergonDraging::BeziergonDraging()
{
    wxLogMessage(wxT("BeziergonDraging Enter."));
}

BeziergonDraging::~BeziergonDraging()
{
    wxLogMessage(wxT("BeziergonDraging Quit."));
}