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

void StepBase::SetPortStatus(const int portIndex, const bool inPort, const bool matching)
{
    statusFlags_ |= kSDSF_PORT_HIGHLIGHT;
    statusFlags_ &= ~kSDSF_PORT_INDEX;
    statusFlags_ |= ((0x1F & static_cast<uint64_t>(portIndex)) << 59);
    if (matching)
    {
        statusFlags_ |= kSDSF_PORT_MATCH;
    }
    else
    {
        statusFlags_ &= ~kSDSF_PORT_MATCH;
    }

    if (inPort)
    {
        statusFlags_ |= kSDSF_PORT_INOUT;
    }
    else
    {
        statusFlags_ &= ~kSDSF_PORT_INOUT;
    }
}
void StepBase::ClearPortStatus()
{
    statusFlags_ &= ~kSDSF_PORT_HIGHLIGHT;
}
void StepBase::TogglePortStatus()
{
    statusFlags_ ^= kSDSF_PORT_HIGHLIGHT;
}
const std::tuple<int, bool, bool, bool> StepBase::GetPortStatus() const
{
    const int portIndex = static_cast<int>((statusFlags_ & kSDSF_PORT_INDEX) >> 59);
    return std::make_tuple(portIndex, statusFlags_ & kSDSF_PORT_HIGHLIGHT, statusFlags_ & kSDSF_PORT_MATCH, statusFlags_ & kSDSF_PORT_INOUT);
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

void StepBase::DrawConnectionMarks(wxGCDC &dc, const wxAffineMatrix2D &affMat) const
{
    if (IsConnectionMarks())
    {
        const wxSize offsetSize(-3, -3);
        const wxSize handleSize(7, 7);
        const wxPoint2DDouble minPt = affMat.TransformPoint(posRect_.GetLeftTop());
        const wxPoint2DDouble maxPt = affMat.TransformPoint(posRect_.GetBottomRight());
        const wxRect tbbox{ wxRound(minPt.m_x), wxRound(minPt.m_y), wxRound(maxPt.m_x - minPt.m_x), wxRound(maxPt.m_y - minPt.m_y) };
        wxPen handlePen(wxColour(0xFF, 0x00, 0x00), 1, wxPENSTYLE_SOLID);
        wxPen matchPen(wxColour(0x00, 0xFF, 0x00), 1, wxPENSTYLE_SOLID);

        dc.SetPen(handlePen);
        dc.SetBrush(wxNullBrush);

        constexpr int dl = 3;
        for (int ii = 0; ii < GetInPortCount(); ++ii)
        {
            const int x = tbbox.GetLeft() + tbbox.GetWidth() * (ii+1) / (GetInPortCount() + 1);
            dc.DrawLine(x - dl, tbbox.GetTop() - dl, x + dl, tbbox.GetTop() + dl);
            dc.DrawLine(x - dl, tbbox.GetTop() + dl, x + dl, tbbox.GetTop() - dl);
        }

        for (int ii = 0; ii < GetOutPortCount(); ++ii)
        {
            const int x = tbbox.GetLeft() + tbbox.GetWidth() * (ii + 1) / (GetOutPortCount() + 1);
            dc.DrawLine(x - dl, tbbox.GetBottom() - dl, x + dl, tbbox.GetBottom() + dl);
            dc.DrawLine(x - dl, tbbox.GetBottom() + dl, x + dl, tbbox.GetBottom() - dl);
        }

        const auto[portIndex, portHighlight, portMatch, inPort] = GetPortStatus();
        if (portHighlight)
        {
            if (portMatch)
            {
                dc.SetPen(matchPen);
            }

            const wxSize offsetSize(-4, -4);
            const wxSize handleSize(9, 9);

            if (inPort && portIndex >= 0 && portIndex < GetInPortCount())
            {
                const int x = tbbox.GetLeft() + tbbox.GetWidth() * (portIndex + 1) / (GetInPortCount() + 1);
                dc.DrawRectangle(wxPoint(x, tbbox.GetTop()) + offsetSize, handleSize);
            }

            if (!inPort && portIndex >= 0 && portIndex < GetOutPortCount())
            {
                const int x = tbbox.GetLeft() + tbbox.GetWidth() * (portIndex + 1) / (GetOutPortCount() + 1);
                dc.DrawRectangle(wxPoint(x, tbbox.GetBottom()) + offsetSize, handleSize);
            }
        }
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

const wxRect StepBase::GetInPortBoundingBox(const int portIndex, const wxAffineMatrix2D &affMat) const
{
    const wxSize offsetSize(-3, -3);
    const wxSize handleSize(7, 7);
    const wxPoint2DDouble minPt = affMat.TransformPoint(posRect_.GetLeftTop());
    const wxPoint2DDouble maxPt = affMat.TransformPoint(posRect_.GetBottomRight());
    const wxRect tbbox{ wxRound(minPt.m_x), wxRound(minPt.m_y), wxRound(maxPt.m_x - minPt.m_x), wxRound(maxPt.m_y - minPt.m_y) };

    wxRect portBox;
    if (portIndex >= 0 && portIndex < GetInPortCount())
    {
        const int x = tbbox.GetLeft() + tbbox.GetWidth() * (portIndex + 1) / (GetInPortCount() + 1);
        portBox.SetPosition(wxPoint(x, tbbox.GetTop()) + offsetSize);
        portBox.SetSize(handleSize);
    }

    return portBox;
}

const wxRect StepBase::GetOutPortBoundingBox(const int portIndex, const wxAffineMatrix2D &affMat) const
{
    const wxSize offsetSize(-4, -4);
    const wxSize handleSize(9, 9);
    const wxPoint2DDouble minPt = affMat.TransformPoint(posRect_.GetLeftTop());
    const wxPoint2DDouble maxPt = affMat.TransformPoint(posRect_.GetBottomRight());
    const wxRect tbbox{ wxRound(minPt.m_x), wxRound(minPt.m_y), wxRound(maxPt.m_x - minPt.m_x), wxRound(maxPt.m_y - minPt.m_y) };

    wxRect portBox;
    if (portIndex >= 0 && portIndex < GetOutPortCount())
    {
        const int x = tbbox.GetLeft() + tbbox.GetWidth() * (portIndex + 1) / (GetOutPortCount() + 1);
        portBox.SetPosition(wxPoint(x, tbbox.GetBottom()) + offsetSize);
        portBox.SetSize(handleSize);
    }

    return portBox;
}

