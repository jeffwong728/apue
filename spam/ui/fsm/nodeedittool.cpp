#include "nodeedittool.h"
#include <wx/log.h>
#include <wx/dcgraph.h>
#include <ui/spam.h>
#include <ui/toplevel/rootframe.h>
#include <ui/cv/cairocanvas.h>
#include <ui/projs/drawablenode.h>
#include <ui/projs/geomnode.h>

NodeEditTool::NodeEditTool()
    : NodeEditToolImpl(*this)
{
    wxLogMessage(wxT("NodeEditTool Enter."));
}

NodeEditTool::~NodeEditTool()
{
}

void NodeEditTool::OnOptionChanged(const EvToolOption &e)
{
    const int toolId = boost::get<int>(e.toolOptions.at(cp_ToolId));
    if (kSpamID_TOOLBOX_GEOM_EDIT == toolId)
    {
        toolOptions = e.toolOptions;
        BoxToolImpl::ResetTool();
    }
}

void NodeEditTool::OnEnterCanvas(const EvCanvasEnter &e)
{
    const int editMode = boost::get<int>(toolOptions.at(cp_ToolGeomVertexEditMode));
    const SpamIconPurpose ip = kICON_PURPOSE_CURSOR;
    wxBitmap curIcon;
    switch (editMode)
    {
    case kSpamID_TOOLBOX_NODE_MOVE:      curIcon = Spam::GetBitmap(ip, bm_NodeMove); break;
    case kSpamID_TOOLBOX_NODE_ADD:       curIcon = Spam::GetBitmap(ip, bm_NodeAdd); break;
    case kSpamID_TOOLBOX_NODE_DELETE:    curIcon = Spam::GetBitmap(ip, bm_NodeDelete); break;
    case kSpamID_TOOLBOX_NODE_SMOOTH:    curIcon = Spam::GetBitmap(ip, bm_NodeSmooth); break;
    case kSpamID_TOOLBOX_NODE_CUSP:      curIcon = Spam::GetBitmap(ip, bm_NodeCusp); break;
    case kSpamID_TOOLBOX_NODE_SYMMETRIC: curIcon = Spam::GetBitmap(ip, bm_NodeSymmetric); break;

    default:
        break;
    }

    if (curIcon.IsOk())
    {
        wxBitmap cursorImg;
        cursorImg.Create(32, 32);
        //cursorImg.UseAlpha();
        wxMemoryDC memDC(cursorImg);
        wxGCDC dc(memDC);
        dc.SetBackground(*wxTRANSPARENT_BRUSH);

        dc.SetPen(*wxLIGHT_GREY_PEN);
        dc.SetBrush(*wxBLACK_BRUSH);
        const wxPoint points[] = { {0, 0},{4, 8},{5, 5}, {8, 4} };
        dc.DrawPolygon(4, points);
        dc.DrawBitmap(curIcon, wxPoint(8, 8));

        memDC.SelectObject(wxNullBitmap);

        auto img = cursorImg.ConvertToImage();
        img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
        img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);

        CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
        cav->SetCursor(wxCursor(img));
    }

    BoxToolT::EnterCanvas(e);
}

void NodeEditTool::OnEntityClicked(const EvEntityClicked &e)
{
    const int editMode = boost::get<int>(toolOptions.at(cp_ToolGeomVertexEditMode));
    if (kSpamID_TOOLBOX_NODE_MOVE != editMode)
    {
        CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.e.GetEventObject());
        if (cav)
        {
            cav->ModifyDrawable(e.ent, e.pt, e.sd, editMode);
        }
    }
}

NodeEditIdle::NodeEditIdle()
    : EditIdleT(*this)
{
    wxLogMessage(wxT("NodeEditIdle Enter."));
}

NodeEditIdle::~NodeEditIdle()
{
    wxLogMessage(wxT("NodeEditIdle Quit."));
}

sc::result NodeEditIdle::react(const EvLMouseDown &e)
{
    const int editMode = boost::get<int>(context<NodeEditTool>().toolOptions.at(cp_ToolGeomVertexEditMode));
    if (kSpamID_TOOLBOX_NODE_MOVE == editMode)
    {
        return reactLMouseDown(e);
    }
    else
    {
        return transit<NodeEditBoxSelecting>(&NodeEditTool::BoxToolT::StartBoxing, e);
    }
}

NodeEditBoxSelecting::NodeEditBoxSelecting()
{
    wxLogMessage(wxT("NodeEditDraging Enter."));
}

NodeEditBoxSelecting::~NodeEditBoxSelecting()
{
    wxLogMessage(wxT("NodeEditDraging Quit."));
}

NodeEditing::NodeEditing()
{
    wxLogMessage(wxT("NodeEditing Enter."));
}

NodeEditing::~NodeEditing()
{
    wxLogMessage(wxT("NodeEditing Quit."));
}