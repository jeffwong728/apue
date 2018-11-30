// wxWidgets "Hello World" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include "spamer.h"
#include <wx/log.h>
#include <ui/projs/geomnode.h>
#include <ui/projs/drawablenode.h>

Spamer::Spamer()
{
}

Spamer::~Spamer()
{
}

void Spamer::OnAppQuit()
{
    process_event(EvAppQuit());
}

void Spamer::OnToolEnter(const ToolOptions &tos)
{
    const int toolId = boost::get<int>(tos.at(cp_ToolId));
    process_event(EvToolEnter(toolId));
    process_event(EvToolOption(tos));
}

void Spamer::OnToolQuit(int toolId)
{
    process_event(EvToolQuit(toolId));
}

void Spamer::OnCanvasEnter(wxMouseEvent &e)
{
    process_event(EvCanvasEnter(e));
}

void Spamer::OnCanvasLeave(wxMouseEvent &e)
{
    process_event(EvCanvasLeave(e));
}

void Spamer::OnCanvasLeftMouseDown(wxMouseEvent &e)
{
    process_event(EvLMouseDown(e));
}

void Spamer::OnCanvasLeftMouseUp(wxMouseEvent &e)
{
    process_event(EvLMouseUp(e));
}

void Spamer::OnCanvasMouseMotion(wxMouseEvent &e)
{
    process_event(EvMouseMove(e));
}

void Spamer::OnCanvasLeftDClick(wxMouseEvent &e)
{
    process_event(EvLMouseDClick(e));
}

void Spamer::OnCanvasMiddleDown(wxMouseEvent &e)
{
    process_event(EvMMouseDown(e));
}

void Spamer::OnCanvasKeyDown(wxKeyEvent &e)
{

}

void Spamer::OnCanvasKeyUp(wxKeyEvent &e)
{

}

void Spamer::OnCanvasChar(wxKeyEvent &e)
{
    if (WXK_ESCAPE == e.GetKeyCode())
    {
        process_event(EvReset(e));
    }
}

void Spamer::OnGeomDelete(const SPModelNodeVector &geoms)
{
    SPDrawableNodeVector dras;
    for (const auto &geom : geoms)
    {
        auto dra = std::dynamic_pointer_cast<DrawableNode>(geom);
        if (dra)
        {
            dras.push_back(dra);
        }
    }
    process_event(EvDrawableDelete(dras));
}

void Spamer::OnDrawableSelect(const SPDrawableNodeVector &dras)
{
    process_event(EvDrawableSelect(dras));
}
