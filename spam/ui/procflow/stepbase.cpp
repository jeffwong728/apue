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
}

StepBase::StepBase(const wxString &typeName)
    : uuid_(boost::uuids::to_string(boost::uuids::random_generator()()))
    , typeName_(typeName)
{
}

void StepBase::Draw(wxGCDC &dc) const
{
    dc.SetFont(*wxNORMAL_FONT);
    wxSize htSize = dc.GetTextExtent(typeName_) + wxSize(5, 5);
    wxPoint htLineOffset = wxPoint(0, htSize.GetHeight());

    wxPen outLinePen(wxColour(214, 219, 233), 2, wxPENSTYLE_SOLID);
    dc.SetPen(outLinePen);
    dc.DrawLine(posRect_.GetTopLeft() + htLineOffset, posRect_.GetTopRight() + htLineOffset);
    dc.SetBrush(wxNullBrush);
    dc.DrawRoundedRectangle(posRect_, htSize.GetHeight());
    dc.SetTextForeground(wxColour(214, 219, 233));
    dc.DrawText(typeName_, posRect_.GetTopLeft() + wxPoint((posRect_.GetWidth() - htSize.GetWidth())/2, 2));
}

wxRect StepBase::GetBoundingBox() const
{
    const wxRect bbox(posRect_);
    return bbox.Inflate(2, 2);
}
