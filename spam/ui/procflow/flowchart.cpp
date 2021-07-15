#include "flowchart.h"
#include "stepbase.h"
#include <wx/artprov.h>
#include <wx/display.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <algorithm>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/2geom.h>
#pragma warning( pop )

#include "initstep.h"
#include "endstep.h"
#include "cvtstep.h"
#include "threshstep.h"

FlowChart::FlowChart(wxWindow* parent)
{
    wxScrolledCanvas::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    InheritAttributes();

    Bind(wxEVT_ENTER_WINDOW, &FlowChart::OnEnterWindow,   this, wxID_ANY);
    Bind(wxEVT_LEAVE_WINDOW, &FlowChart::OnLeaveWindow,   this, wxID_ANY);
    Bind(wxEVT_LEFT_DOWN,    &FlowChart::OnLeftMouseDown, this, wxID_ANY);
    Bind(wxEVT_LEFT_UP,      &FlowChart::OnLeftMouseUp,   this, wxID_ANY);
    Bind(wxEVT_MOTION,       &FlowChart::OnMouseMotion,   this, wxID_ANY);
    Bind(wxEVT_PAINT,        &FlowChart::OnPaint,         this, wxID_ANY);

    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create());
    if (gc)
    {
        wxDouble width{0};
        wxDouble height{0};
        gc->SetFont(*wxNORMAL_FONT, *wxCYAN);
        gc->GetTextExtent(wxT("123"), &width, &height);
        gapX_ = wxRound(height);
        gapY_ = wxRound(height);
    }

    steps_.push_back(std::make_shared<InitStep>());
    steps_.push_back(std::make_shared<CvtStep>());
    steps_.push_back(std::make_shared<ThreshStep>());
    steps_.push_back(std::make_shared<EndStep>());

    SetVirtualSize(wxSize(100, 240));
    SetScrollRate(6, 6);
}

void FlowChart::OnEnterWindow(wxMouseEvent &e)
{
}

void FlowChart::OnLeaveWindow(wxMouseEvent &e)
{
}

void FlowChart::OnLeftMouseDown(wxMouseEvent &e)
{
}

void FlowChart::OnLeftMouseUp(wxMouseEvent &e)
{
}

void FlowChart::OnMouseMotion(wxMouseEvent &e)
{
}

void FlowChart::OnPaint(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(this);
    PrepareDC(dc);
    wxGCDC gcdc(dc);

    dc.SetBrush(wxBrush(wxColour(55, 56, 58)));
    dc.SetPen(wxNullPen);
    wxRect cRect = GetClientRect();
    dc.DrawRectangle(cRect);

    int y = 10;
    for (SPStepBase &step : steps_)
    {
        step->SetRect(wxRect((cRect.GetWidth()-120)/2, y, 120, 70));
        y += 90;

        step->Draw(gcdc);
    }
}

void FlowChart::DrawBackground() const
{
    wxGCDC dc;
    wxColour backgroundColour = GetBackgroundColour();
    dc.SetBrush(wxBrush(backgroundColour));
    dc.SetPen(wxNullPen);
    wxRect cRect = GetClientRect();
    dc.DrawRectangle(cRect);
}
