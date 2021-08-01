#include "endstep.h"
#include <wx/dcgraph.h>
#include <wx/artprov.h>
#include <wx/display.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>

EndStep::EndStep()
    : StepBase(wxString(wxT("End")))
{
}

void EndStep::Draw(wxGCDC &dc) const
{
    dc.SetFont(*wxNORMAL_FONT);
    wxPen outLinePen(IsHighlight() ? wxColour(0xF9, 0xA6, 0x02) : wxColour(214, 219, 233), 2, wxPENSTYLE_SOLID);
    dc.SetPen(outLinePen);
    dc.SetBrush(wxNullBrush);
    dc.DrawEllipse(posRect_.GetTopLeft(), posRect_.GetSize());
    dc.SetTextForeground(IsHighlight() ? wxColour(0xF9, 0xA6, 0x02) : wxColour(214, 219, 233));
    dc.DrawText(typeName_, posRect_.GetTopLeft() + wxPoint((posRect_.GetWidth() - htSize_.GetWidth()) / 2, (posRect_.GetHeight() - htSize_.GetHeight()) / 2));
}

