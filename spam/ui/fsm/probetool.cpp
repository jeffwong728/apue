#include "probetool.h"
#include <wx/log.h>
#include <ui/cv/cairocanvas.h>

void ProbeTool::FireClickEntity(const SPDrawableNode &ent, const wxMouseEvent &e, const Geom::Point &pt, const SelectionData &sd) const
{
}

void ProbeIdle::OnSafari(const EvMouseMove &e)
{
    context<ProbeTool>().Safari(e);
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        if (cav->IsInImageRect(e.evData.GetPosition()))
        {
            cav->DismissInstructionTip();
            cav->ShowPixelValue(e.evData.GetPosition());
        }
        else
        {
            cav->StopInstructionTip();
        }
    }
}

void ProbeIdle::OnLeaveCanvas(const EvCanvasLeave &e)
{
    context<ProbeTool>().LeaveCanvas(e);
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        cav->StopInstructionTip();
    }
}
