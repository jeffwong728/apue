#include "initstep.h"
#include <wx/dcgraph.h>
#include <wx/artprov.h>
#include <wx/display.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>

InitStep::InitStep()
    : StepBase(wxString(wxT("Start")))
{
}

void InitStep::DrawInternal(wxGCDC &dc) const
{
    dc.DrawRoundedRectangle(posRect_, htSize_.GetHeight());
}
