#include "stepbase.h"
#include <wx/dcgraph.h>
#include <wx/artprov.h>
#include <wx/display.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

StepBase::StepBase(wxString &&typeName)
    : uuid_(boost::uuids::to_string(boost::uuids::random_generator()()))
    , typeName_(typeName)
{
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create());
    if (gc)
    {
        wxDouble width{ 0 };
        wxDouble height{ 0 };
        gc->SetFont(*wxNORMAL_FONT, *wxCYAN);
        gc->GetTextExtent(typeName_, &width, &height);
        htSize_.Set(wxRound(width), wxRound(height));
        SetRect(wxRect());
    }
}

StepBase::StepBase(const wxString &typeName)
    : uuid_(boost::uuids::to_string(boost::uuids::random_generator()()))
    , typeName_(typeName)
{
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create());
    if (gc)
    {
        wxDouble width{ 0 };
        wxDouble height{ 0 };
        gc->SetFont(*wxNORMAL_FONT, *wxCYAN);
        gc->GetTextExtent(typeName_, &width, &height);
        htSize_.Set(wxRound(width), wxRound(height));
        SetRect(wxRect());
    }
}

void StepBase::Draw(wxGCDC &dc) const
{
    dc.SetFont(*wxNORMAL_FONT);
    wxPoint htLineOffset = wxPoint(0, htSize_.GetHeight() + 3);

    wxPen outLinePen(IsHighlight() ? wxColour(0xF9, 0xA6, 0x02) : wxColour(214, 219, 233), 2, wxPENSTYLE_SOLID);
    dc.SetPen(outLinePen);
    dc.DrawLine(posRect_.GetTopLeft() + htLineOffset, posRect_.GetTopRight() + htLineOffset);
    dc.SetBrush(wxNullBrush);
    DrawInternal(dc);
    dc.SetTextForeground(IsHighlight() ? wxColour(0xF9, 0xA6, 0x02) : wxColour(214, 219, 233));
    dc.DrawText(typeName_, posRect_.GetTopLeft() + wxPoint((posRect_.GetWidth() - htSize_.GetWidth())/2, 2));
}

void StepBase::DrawHandles(wxGCDC &dc, const wxAffineMatrix2D &affMat) const
{
    if (IsSelected())
    {
        const wxSize offsetSize(-3, -3);
        const wxSize handleSize(7, 7);
        const wxPoint2DDouble minPt = affMat.TransformPoint(posRect_.GetLeftTop());
        const wxPoint2DDouble maxPt = affMat.TransformPoint(posRect_.GetBottomRight());
        const wxRect tbbox{ wxRound(minPt.m_x), wxRound(minPt.m_y), wxRound(maxPt.m_x - minPt.m_x), wxRound(maxPt.m_y - minPt.m_y) };
        wxPen handlePen(wxColour(0xFF, 0x00, 0x00), 1, wxPENSTYLE_DOT);

        dc.SetPen(handlePen);
        dc.SetBrush(wxNullBrush);
        dc.DrawRectangle(tbbox.GetTopLeft() + offsetSize, handleSize);
        dc.DrawRectangle(tbbox.GetTopRight() + offsetSize, handleSize);
        dc.DrawRectangle(tbbox.GetBottomLeft() + offsetSize, handleSize);
        dc.DrawRectangle(tbbox.GetBottomRight() + offsetSize, handleSize);
        dc.DrawRectangle((tbbox.GetTopLeft() + tbbox.GetTopRight()) / 2 + offsetSize, handleSize);
        dc.DrawRectangle((tbbox.GetTopRight() + tbbox.GetBottomRight()) / 2 + offsetSize, handleSize);
        dc.DrawRectangle((tbbox.GetBottomLeft() + tbbox.GetBottomRight()) / 2 + offsetSize, handleSize);
        dc.DrawRectangle((tbbox.GetTopLeft() + tbbox.GetBottomLeft()) / 2 + offsetSize, handleSize);
    }
}

void StepBase::DrawInternal(wxGCDC &dc) const
{
    dc.DrawRectangle(posRect_);
}

void StepBase::SetRect(const wxRect &rc)
{
    posRect_ = rc;
    if (posRect_.GetWidth() <= 0)
    {
        posRect_.SetWidth(htSize_.GetWidth() + htSize_.GetHeight() * 2);
    }
    if (posRect_.GetHeight() <= 0)
    {
        posRect_.SetHeight(htSize_.GetHeight()*3 + 5);
    }
}

const wxRect StepBase::GetBoundingBox() const
{
    const wxRect bbox(posRect_);
    return bbox.Inflate(3, 3);
}

const wxRect StepBase::GetBoundingBox(const wxAffineMatrix2D &affMat) const
{
    const wxPoint2DDouble minPt = affMat.TransformPoint(posRect_.GetLeftTop());
    const wxPoint2DDouble maxPt = affMat.TransformPoint(posRect_.GetBottomRight());
    const wxRect tbbox{ wxRound(minPt.m_x - 5), wxRound(minPt.m_y - 5), wxRound(maxPt.m_x - minPt.m_x + 11), wxRound(maxPt.m_y - minPt.m_y + 11) };
    return tbbox;
}

