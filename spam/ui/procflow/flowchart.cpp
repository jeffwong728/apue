#include "flowchart.h"
#include "stepbase.h"
#include <wx/artprov.h>
#include <wx/display.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <wx/dnd.h>
#include <algorithm>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/2geom.h>
#pragma warning( pop )

#include "initstep.h"
#include "endstep.h"
#include "cvtstep.h"
#include "threshstep.h"

class DnDText : public wxTextDropTarget
{
public:
    DnDText(FlowChart *pOwner) { m_pOwner = pOwner; }

    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& text) wxOVERRIDE;

private:
    FlowChart *m_pOwner;
};

bool DnDText::OnDropText(wxCoord x, wxCoord y, const wxString& text)
{
    m_pOwner->AppendStep(x, y, text);
    return true;
}

FlowChart::FlowChart(wxWindow* parent)
{
    wxWindow::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    InheritAttributes();

    Bind(wxEVT_ENTER_WINDOW, &FlowChart::OnEnterWindow,     this, wxID_ANY);
    Bind(wxEVT_LEAVE_WINDOW, &FlowChart::OnLeaveWindow,     this, wxID_ANY);
    Bind(wxEVT_LEFT_DOWN,    &FlowChart::OnLeftMouseDown,   this, wxID_ANY);
    Bind(wxEVT_LEFT_UP,      &FlowChart::OnLeftMouseUp,     this, wxID_ANY);
    Bind(wxEVT_RIGHT_DOWN,   &FlowChart::OnRightMouseDown,  this, wxID_ANY);
    Bind(wxEVT_RIGHT_UP,     &FlowChart::OnRightMouseUp,    this, wxID_ANY);
    Bind(wxEVT_MOTION,       &FlowChart::OnMouseMotion,     this, wxID_ANY);
    Bind(wxEVT_MOUSEWHEEL,   &FlowChart::OnMouseWheel,      this, wxID_ANY);
    Bind(wxEVT_PAINT,        &FlowChart::OnPaint,           this, wxID_ANY);

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

    SetDropTarget(new DnDText(this));
}

void FlowChart::AppendStep(wxCoord x, wxCoord y, const wxString& stepType)
{
    bool needRefresh = true;
    if (wxT("initstep") == stepType)
    {
        steps_.push_back(std::make_shared<InitStep>());
    }
    else if (wxT("endstep") == stepType)
    {
        steps_.push_back(std::make_shared<EndStep>());
    }
    else if (wxT("cvtstep") == stepType)
    {
        steps_.push_back(std::make_shared<CvtStep>());
    }
    else if (wxT("threshstep") == stepType)
    {
        steps_.push_back(std::make_shared<ThreshStep>());
    }
    else
    {
        needRefresh = false;
    }

    if (needRefresh)
    {
        Refresh(false);
    }
}

void FlowChart::OnEnterWindow(wxMouseEvent &e)
{
}

void FlowChart::OnLeaveWindow(wxMouseEvent &e)
{
}

void FlowChart::OnLeftMouseDown(wxMouseEvent &e)
{
    if (e.LeftIsDown() && e.RightIsDown())
    {
        affMat_ = wxAffineMatrix2D();
        Refresh(false);
    }
}

void FlowChart::OnRightMouseDown(wxMouseEvent &e)
{
    lastPos_ = e.GetPosition();
    if (e.LeftIsDown() && e.RightIsDown())
    {
        affMat_ = wxAffineMatrix2D();
        Refresh(false);
    }
}

void FlowChart::OnRightMouseUp(wxMouseEvent &e)
{

}

void FlowChart::OnLeftMouseUp(wxMouseEvent &e)
{
}

void FlowChart::OnMouseMotion(wxMouseEvent &e)
{
    if (e.Dragging() && e.RightIsDown())
    {
        wxAffineMatrix2D invMat = affMat_;
        invMat.Invert();
        const auto deltaPos = invMat.TransformDistance(e.GetPosition() - lastPos_);
        affMat_.Translate(deltaPos.m_x, deltaPos.m_y);
        Refresh(false);
    }
    lastPos_ = e.GetPosition();
}

void FlowChart::OnMouseWheel(wxMouseEvent &e)
{
    const auto tPt = affMat_.TransformPoint(wxPoint2DDouble(e.GetPosition()));
    affMat_.Translate(tPt.m_x, tPt.m_y);
    const double s = e.GetWheelRotation() > 0 ? 1.1 : 0.9;
    affMat_.Scale(s, s);
    affMat_.Translate(-tPt.m_x, -tPt.m_y);
    Refresh(false);
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
    gcdc.SetTransformMatrix(affMat_);
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
