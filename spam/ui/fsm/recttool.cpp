#include "recttool.h"
#include <pixmaps/cursor-rect.xpm>
#include <wx/log.h>
#include <wx/dcgraph.h>
#include <ui/spam.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/rectnode.h>
#include <algorithm>

RectTool::RectTool()
{
    wxLogMessage(wxT("RectTool Enter."));
}

RectTool::~RectTool()
{
    wxLogMessage(wxT("RectTool Quit."));
}

void RectTool::OnStartDraging(const EvLMouseDown &e)
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

void RectTool::OnDraging(const EvMouseMove &e)
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

        cav->DrawPathVector(pv, brect);
        rect.emplace(newRect);
    }
}

void RectTool::OnTracing(const EvMouseMove &e)
{
    OnDraging(e);
}

void RectTool::OnEndDraging(const EvLMouseUp &e)
{
    EndDraging(e.evData);
}

void RectTool::OnEndTracing(const EvLMouseDown &e)
{
    EndTracing(e.evData);
}

void RectTool::EndDraging(const wxMouseEvent &e)
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

        cav->DrawPathVector(pv, brect);

        if (newRect.width()>1 || newRect.height()>1)
        {
            RectData rd;
            rd.points[0][0] = newRect.left();
            rd.points[0][1] = newRect.top();
            rd.points[1][0] = newRect.right();
            rd.points[1][1] = newRect.top();
            rd.points[2][0] = newRect.right();
            rd.points[2][1] = newRect.bottom();
            rd.points[3][0] = newRect.left();
            rd.points[3][1] = newRect.bottom();
            rd.radii.fill({ 0, 0 });

            Geom::Affine ide = Geom::Affine::identity();
            for (int i = 0; i < rd.transform.size(); ++i)
            {
                rd.transform[i] = ide[i];
            }

            cav->AddRect(rd);
        }

        rect = Geom::OptRect();
    }
}
void RectTool::EndTracing(const wxMouseEvent &e)
{
    EndDraging(e);
}

void RectTool::OnCanvasEnter(const EvCanvasEnter &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        wxBitmap cursorImg;
        cursorImg.Create(32, 32, 32);
        wxMemoryDC memDC(cursorImg);
        wxGCDC dc(memDC);
        dc.SetBackground(*wxTRANSPARENT_BRUSH);
        dc.SetBackgroundMode(wxSOLID);
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
        dc.DrawRectangle(wxRect(13, 13, 16, 16));
        memDC.SelectObject(wxNullBitmap);

        auto img = cursorImg.ConvertToImage();
        img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 5);
        img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 5);
        cav->SetCursor(wxCursor(img));
    }
}

void RectTool::OnCanvasLeave(const EvCanvasLeave &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        cav->SetCursor(wxCURSOR_ARROW);
    }
}

void RectTool::OnReset(const EvReset &e)
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

RectIdle::RectIdle()
{
    wxLogMessage(wxT("RectIdle Enter."));
}

RectIdle::~RectIdle()
{
    wxLogMessage(wxT("RectIdle Quit."));
}

sc::result RectIdle::react(const EvToolQuit &e)
{
    return transit<NoTool>();
}

RectDraging::RectDraging()
{
    wxLogMessage(wxT("RectDraging Enter."));
}

RectDraging::~RectDraging()
{
    wxLogMessage(wxT("RectDraging Quit."));
}

sc::result RectDraging::react(const EvLMouseUp &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);
        Geom::Rect rect{ context<RectTool>().anchor , freePt };
        if (rect.width()<3 && rect.height()<3)
        {
            return transit<RectTracing>();
        }
        else
        {
            return transit<RectIdle>(&RectTool::OnEndDraging, e);
        }
    }
    else
    {
        return discard_event();
    }
}

RectTracing::RectTracing()
{
    wxLogMessage(wxT("RectTracing Enter."));
}

RectTracing::~RectTracing()
{
    wxLogMessage(wxT("RectTracing Quit."));
}