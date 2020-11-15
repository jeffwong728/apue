#include "ellipsetool.h"
#include <pixmaps/cursor-rect.xpm>
#include <wx/log.h>
#include <wx/dcgraph.h>
#include <ui/spam.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/rectnode.h>
#include <algorithm>

EllipseTool::EllipseTool()
{
    wxLogMessage(wxT("EllipseTool Enter."));
}

EllipseTool::~EllipseTool()
{
    wxLogMessage(wxT("EllipseTool Quit."));
}

void EllipseTool::OnStartDraging(const EvLMouseDown &e)
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

void EllipseTool::OnDraging(const EvMouseMove &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);
        Geom::Rect newRect{ anchor , freePt };

        Geom::PathVector pv;
        if (newRect.area()>Geom::EPSILON)
        {
            Geom::Point c = newRect.midpoint();
            Geom::Point r{ newRect.width() / 2, newRect.height() / 2 };
            pv.push_back(Geom::Path(Geom::Ellipse(c, r, 0)));
        }

        auto brect = pv.boundsFast();
        brect.unionWith(rect);

        cav->DrawPathVector(pv, brect);
        rect.emplace(newRect);
    }
}

void EllipseTool::OnTracing(const EvMouseMove &e)
{
    OnDraging(e);
}

void EllipseTool::OnEndDraging(const EvLMouseUp &e)
{
    EndDraging(e.evData);
}

void EllipseTool::OnEndTracing(const EvLMouseDown &e)
{
    EndTracing(e.evData);
}

void EllipseTool::EndDraging(const wxMouseEvent &e)
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
        Geom::Rect newRect{ anchor , freePt };
        
        Geom::PathVector pv;
        if (newRect.area()>Geom::EPSILON)
        {
            Geom::Point c = newRect.midpoint();
            Geom::Point r{ newRect.width() / 2, newRect.height() / 2 };
            pv.push_back(Geom::Path(Geom::Ellipse(c, r, 0)));
        }

        auto brect = pv.boundsFast();
        brect.unionWith(rect);

        cav->DrawPathVector(pv, brect);

        if (newRect.width()>1 || newRect.height()>1)
        {
            GenericEllipseArcData ed;
            ed.points[0][0] = newRect.left();
            ed.points[0][1] = newRect.top();
            ed.points[1][0] = newRect.right();
            ed.points[1][1] = newRect.top();
            ed.points[2][0] = newRect.right();
            ed.points[2][1] = newRect.bottom();
            ed.points[3][0] = newRect.left();
            ed.points[3][1] = newRect.bottom();
            ed.angles[0] = 0;
            ed.angles[1] = 360;
            ed.type = GenericEllipseArcType::kAtSlice;

            Geom::Affine ide = Geom::Affine::identity();
            for (int i = 0; i < ed.transform.size(); ++i)
            {
                ed.transform[i] = ide[i];
            }

            cav->AddEllipse(ed);
        }
    }

    rect = Geom::OptRect();
}
void EllipseTool::EndTracing(const wxMouseEvent &e)
{
    EndDraging(e);
}

void EllipseTool::OnCanvasEnter(const EvCanvasEnter &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        wxBitmap cursorImg;
        cursorImg.Create(32, 32);
        //cursorImg.UseAlpha();
        wxMemoryDC memDC(cursorImg);
        wxGCDC dc(memDC);
        dc.SetBackground(*wxTRANSPARENT_BRUSH);
        wxColour strokeColor;
        strokeColor.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomStrokePaint, wxBLUE->GetRGBA()));
        wxColour fillColor;
        fillColor.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomFillPaint, wxBLUE->GetRGBA()));
        wxBrush brush(fillColor);
        wxPen pen(strokeColor);
        dc.SetPen(pen);
        dc.SetBrush(brush);
        dc.DrawLine(0, 5, 10, 5);
        dc.DrawLine(5, 0, 5, 10);
        dc.DrawEllipse(wxRect(13, 13, 16, 16));
        memDC.SelectObject(wxNullBitmap);

        auto img = cursorImg.ConvertToImage();
        img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 5);
        img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 5);
        cav->SetCursor(wxCursor(img));
    }
}

void EllipseTool::OnCanvasLeave(const EvCanvasLeave &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        cav->SetCursor(wxCURSOR_ARROW);
    }
}

void EllipseTool::OnReset(const EvReset &e)
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
            cav->DrawPathVector(Geom::PathVector(), rect);
            rect = Geom::OptRect();
        }
    }
}

EllipseIdle::EllipseIdle()
{
    wxLogMessage(wxT("EllipseIdle Enter."));
}

EllipseIdle::~EllipseIdle()
{
    wxLogMessage(wxT("EllipseIdle Quit."));
}

sc::result EllipseIdle::react(const EvToolQuit &e)
{
    return transit<NoTool>();
}

EllipseDraging::EllipseDraging()
{
    wxLogMessage(wxT("EllipseDraging Enter."));
}

EllipseDraging::~EllipseDraging()
{
    wxLogMessage(wxT("EllipseDraging Quit."));
}

sc::result EllipseDraging::react(const EvLMouseUp &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);
        Geom::Rect rect{ context<EllipseTool>().anchor , freePt };
        if (rect.width()<3 && rect.height()<3)
        {
            return transit<EllipseTracing>();
        }
        else
        {
            return transit<EllipseIdle>(&EllipseTool::OnEndDraging, e);
        }
    }
    else
    {
        return discard_event();
    }
}

EllipseTracing::EllipseTracing()
{
    wxLogMessage(wxT("EllipseTracing Enter."));
}

EllipseTracing::~EllipseTracing()
{
    wxLogMessage(wxT("EllipseTracing Quit."));
}