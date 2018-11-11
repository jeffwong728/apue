#include "probetool.h"
#include <wx/log.h>
#include <ui/cv/cairocanvas.h>

ProbeTool::ProbeTool()
{
    wxLogMessage(wxT("ProbeTool Enter."));
}

ProbeTool::~ProbeTool()
{
    wxLogMessage(wxT("ProbeTool Quit."));
}

void ProbeTool::OnStartDraging(const EvLMouseDown &e)
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
    }
}

void ProbeTool::OnDraging(const EvMouseMove &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto imgPt = cav->ScreenToImage(e.evData.GetPosition());
        Geom::Point freePt(imgPt.x, imgPt.y);
        Geom::Rect rect{ anchor , freePt };
        cav->DrawBox(Geom::Path(rect));
    }
}

void ProbeTool::OnEndDraging(const EvLMouseUp &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        if (cav->HasCapture())
        {
            cav->ReleaseMouse();
        }

        cav->DrawBox(Geom::Path());
    }
}

void ProbeTool::OnReset(const EvReset &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        if (cav->HasCapture())
        {
            cav->ReleaseMouse();
        }

        cav->DrawBox(Geom::Path());
    }
}

ProbeIdle::ProbeIdle()
{
    wxLogMessage(wxT("ProbeIdle Enter."));
}

ProbeIdle::~ProbeIdle()
{
    wxLogMessage(wxT("ProbeIdle Quit."));
}

sc::result ProbeIdle::react(const EvToolQuit &e)
{
    return transit<NoTool>();
}

ProbeDraging::ProbeDraging()
{
    wxLogMessage(wxT("ProbeDraging Enter."));
}

ProbeDraging::~ProbeDraging()
{
    wxLogMessage(wxT("ProbeDraging Quit."));
}