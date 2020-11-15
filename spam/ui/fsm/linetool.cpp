#include "linetool.h"
#include <wx/log.h>
#include <wx/dcgraph.h>
#include <ui/spam.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/rectnode.h>
#include <algorithm>

void LineTool::OnStartDraging(const EvLMouseDown &e)
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

void LineTool::OnDraging(const EvMouseMove &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);
        Geom::Rect newRect{ anchor , freePt };

        Geom::PathVector pv{ Geom::Path(newRect) };
        auto brect = pv.boundsFast();
        brect.unionWith(rect);

        pv.clear();
        Geom::PathBuilder pb(pv);
        pb.moveTo(anchor);
        pb.lineTo(freePt);
        pb.flush();
        cav->DrawPathVector(pv, brect);
        rect.emplace(newRect);
    }
}

void LineTool::OnTracing(const EvMouseMove &e)
{
    OnDraging(e);
}

void LineTool::OnEndDraging(const EvLMouseUp &e)
{
    EndDraging(e.evData);
}

void LineTool::OnEndTracing(const EvLMouseDown &e)
{
    EndTracing(e.evData);
}

void LineTool::EndDraging(const wxMouseEvent &e)
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
        
        Geom::PathVector pv{ Geom::Path(newRect) };
        auto brect = pv.boundsFast();
        brect.unionWith(rect);

        pv.clear();
        Geom::PathBuilder pb(pv);
        pb.moveTo(anchor);
        pb.lineTo(freePt);
        pb.flush();
        cav->DrawPathVector(pv, brect);

        if (newRect.width()>1 || newRect.height()>1)
        {
            LineData ld;
            ld.points[0][0] = anchor.x();
            ld.points[0][1] = anchor.y();
            ld.points[1][0] = freePt.x();
            ld.points[1][1] = freePt.y();

            Geom::Affine ide = Geom::Affine::identity();
            for (int i = 0; i < ld.transform.size(); ++i)
            {
                ld.transform[i] = ide[i];
            }

            cav->AddLine(ld);
        }

        rect = Geom::OptRect();
    }
}
void LineTool::EndTracing(const wxMouseEvent &e)
{
    EndDraging(e);
}

void LineTool::OnCanvasEnter(const EvCanvasEnter &e)
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
        dc.DrawLine(wxPoint(13, 13), wxPoint(16, 16));
        memDC.SelectObject(wxNullBitmap);

        auto img = cursorImg.ConvertToImage();
        img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 5);
        img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 5);
        cav->SetCursor(wxCursor(img));
    }
}

void LineTool::OnCanvasLeave(const EvCanvasLeave &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        cav->SetCursor(wxCURSOR_ARROW);
    }
}

void LineTool::OnReset(const EvReset &e)
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

sc::result LineIdle::react(const EvToolQuit &e)
{
    return transit<NoTool>();
}

sc::result LineDraging::react(const EvLMouseUp &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);
        Geom::Rect rect{ context<LineTool>().anchor , freePt };
        if (rect.width()<3 && rect.height()<3)
        {
            return transit<LineTracing>();
        }
        else
        {
            return transit<LineIdle>(&LineTool::OnEndDraging, e);
        }
    }
    else
    {
        return discard_event();
    }
}