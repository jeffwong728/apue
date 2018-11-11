#include "polygontool.h"
#include <wx/log.h>
#include <wx/dcgraph.h>
#include <ui/spam.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/polygonnode.h>

PolygonTool::PolygonTool()
    : polygon(std::make_shared<PolygonNode>())
{
    wxLogMessage(wxT("PolygonTool Enter."));
}

PolygonTool::~PolygonTool()
{
    wxLogMessage(wxT("PolygonTool Quit."));
}

void PolygonTool::OnStartTracing(const EvLMouseDown &e)
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
        polygon->AddCorner(freePt);
        polygon->AddCorner(freePt);
    }
}

void PolygonTool::OnTracing(const EvMouseMove &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);

        Geom::PathVector opv;
        polygon->BuildOpenPath(opv);

        polygon->PopCorner();
        polygon->AddCorner(freePt);

        Geom::PathVector pv;
        polygon->BuildOpenPath(pv);

        auto orect = opv.boundsFast();
        auto rect  = pv.boundsFast();
        rect.unionWith(orect);

        cav->DrawPathVector(pv, rect);
    }
}

void PolygonTool::EndTracing(const wxMouseEvent &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.GetEventObject());
    if (cav)
    {
        if (cav->HasCapture())
        {
            cav->ReleaseMouse();
        }
        
        if (polygon->GetNumCorners()>1)
        {
            Geom::Point pt0, pt1;
            polygon->GetCorner(polygon->GetNumCorners() - 1, pt0);
            polygon->GetCorner(polygon->GetNumCorners() - 2, pt1);
            if (Geom::are_near(pt0, pt1))
            {
                polygon->PopCorner();
            }
        }

        Geom::PathVector pv;
        polygon->BuildPath(pv);

        auto rect = pv.boundsFast();
        cav->DrawPathVector(pv, rect);

        if (polygon->GetNumCorners()>2)
        {
            cav->AddPolygon(polygon->data_);
        }
    }

    polygon->Clear();
}

void PolygonTool::OnCanvasEnter(const EvCanvasEnter &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
}

void PolygonTool::OnCanvasLeave(const EvCanvasLeave &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
}

void PolygonTool::OnAddCorner(const EvLMouseDown &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);
        polygon->AddCorner(freePt);
    }
}

void PolygonTool::OnMMouseDown(const EvMMouseDown &e)
{
    EndTracing(e.evData);
}

void PolygonTool::OnLMouseDClick(const EvLMouseDClick &e)
{
    EndTracing(e.evData);
}

void PolygonTool::OnReset(const EvReset &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        if (cav->HasCapture())
        {
            cav->ReleaseMouse();
        }

        Geom::PathVector opv;
        polygon->BuildOpenPath(opv);

        cav->DrawPathVector(Geom::PathVector(), opv.boundsFast());
    }
}

PolygonIdle::PolygonIdle()
{
    wxLogMessage(wxT("PolygonIdle Enter."));
}

PolygonIdle::~PolygonIdle()
{
    wxLogMessage(wxT("PolygonIdle Quit."));
}

sc::result PolygonIdle::react(const EvToolQuit &e)
{
    return transit<NoTool>();
}

PolygonTracing::PolygonTracing()
{
    wxLogMessage(wxT("PolygonTracing Enter."));
}

PolygonTracing::~PolygonTracing()
{
    wxLogMessage(wxT("PolygonTracing Quit."));
}