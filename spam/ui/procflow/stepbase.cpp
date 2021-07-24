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
}

void StepBase::Draw(wxGCDC &dc) const
{
    dc.SetFont(*wxNORMAL_FONT);
    wxPoint htLineOffset = wxPoint(0, htSize_.GetHeight() + 3);

    wxPen outLinePen(wxColour(214, 219, 233), 2, wxPENSTYLE_SOLID);
    dc.SetPen(outLinePen);
    dc.DrawLine(posRect_.GetTopLeft() + htLineOffset, posRect_.GetTopRight() + htLineOffset);
    dc.SetBrush(wxNullBrush);
    DrawInternal(dc);
    dc.SetTextForeground(wxColour(214, 219, 233));
    dc.DrawText(typeName_, posRect_.GetTopLeft() + wxPoint((posRect_.GetWidth() - htSize_.GetWidth())/2, 2));
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

wxRect StepBase::GetBoundingBox() const
{
    const wxRect bbox(posRect_);
    return bbox.Inflate(2, 2);
}
