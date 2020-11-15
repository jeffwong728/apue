#include "booltool.h"
#include <wx/log.h>
#include <wx/dcgraph.h>
#include <ui/spam.h>
#include <ui/cv/cairocanvas.h>
#include <ui/toplevel/rootframe.h>

void CanvasCursorManipulator::EnterCanvas(const EvCanvasEnter &e, const std::string &bmName)
{
    const SpamIconPurpose ip = kICON_PURPOSE_CURSOR;
    wxBitmap curIcon = Spam::GetBitmap(ip, bmName);

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
        const wxPoint points[] = { { 0, 0 },{ 4, 8 },{ 5, 5 },{ 8, 4 } };
        dc.DrawPolygon(4, points);
        dc.DrawBitmap(curIcon, wxPoint(8, 8));

        memDC.SelectObject(wxNullBitmap);

        auto img = cursorImg.ConvertToImage();
        img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
        img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);

        CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
        cav->SetCursor(wxCursor(img));
    }
}

void UnionTool::OnMMouseDown(const EvMMouseDown &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        EntitySelection &es = selData[cav->GetUUID()];
        SPDrawableNodeVector &selEnts = es.ents;
        if (selEnts.size()>1)
        {
            cav->DoUnion(selEnts);
        }
    }
}

void UnionTool::OnEnterCanvas(const EvCanvasEnter &e)
{
    CanvasCursorManipulator::EnterCanvas(e, bm_PathUnion);
    BoxToolT::EnterCanvas(e);
}

void IntersectionTool::OnMMouseDown(const EvMMouseDown &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        EntitySelection &es = selData[cav->GetUUID()];
        SPDrawableNodeVector &selEnts = es.ents;
        if (selEnts.size()>1)
        {
            cav->DoIntersection(selEnts);
        }
    }
}

void IntersectionTool::OnEnterCanvas(const EvCanvasEnter &e)
{
    CanvasCursorManipulator::EnterCanvas(e, bm_PathInter);
    BoxToolT::EnterCanvas(e);
}

void BinaryBoolOperatorDef::invalidate_operands::operator()(evt_quit_tool const& e, BinaryBoolOperatorDef&, Wait2ndOperand &s, Wait2ndOperand& t)
{
    Spam::InvalidateCanvasRect(e.uuid, s.operand1st->GetBoundingBox());
}

void BinaryBoolOperatorDef::invalidate_operands::operator()(evt_quit_tool const& e, BinaryBoolOperatorDef&, ReadyGo &s, ReadyGo& t)
{
    Spam::InvalidateCanvasRect(e.uuid, s.operand1st->GetBoundingBox());
    Spam::InvalidateCanvasRect(e.uuid, s.operand2nd->GetBoundingBox());
}

void BinaryBoolOperatorDef::wrap_operand::operator()(evt_entity_selected const& e, BinaryBoolOperatorDef &dop, ReadyGo &s, ReadyGo& t)
{
    Spam::InvalidateCanvasRect(e.uuid, s.operand1st->GetBoundingBox());
    s.operand1st->RestoreColor();
    t.operand1st = s.operand2nd;
    t.operand2nd = e.ent;
    t.operand1st->ChangeColorToSelected();
    t.operand2nd->ChangeColorToSelected();
}

void BinaryBoolOperatorDef::do_diff::operator()(const evt_apply & e, BinaryBoolOperatorDef &fsm, BinaryBoolOperatorDef::ReadyGo& s, BinaryBoolOperator::Wait1stOperand& t)
{
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (frame)
    {
        CairoCanvas *cav = frame->FindCanvasByUUID(e.uuid);
        if (cav)
        {
            switch (fsm.binaryBoolOpType)
            {
            case BinaryBoolOperator::BinaryBooleanType::DiffOp:
                cav->DoDifference(s.operand1st, s.operand2nd);
                break;

            case BinaryBoolOperator::BinaryBooleanType::XOROp:
                cav->DoXOR(s.operand1st, s.operand2nd);
                break;

            default:
                break;
            }
        }
    }
}
bool BinaryBoolOperatorDef::valid_operand::operator()(evt_entity_selected const& evt, BinaryBoolOperatorDef&, Wait1stOperand&, Wait2ndOperand&)
{
    return true;
}

bool BinaryBoolOperatorDef::valid_operand::operator()(evt_entity_selected const& evt, BinaryBoolOperatorDef&, Wait2ndOperand &s, ReadyGo &t)
{
    return s.operand1st->GetUUIDTag() != evt.ent->GetUUIDTag();
}

bool BinaryBoolOperatorDef::valid_operand::operator()(evt_entity_selected const& evt, BinaryBoolOperatorDef&, ReadyGo &s, ReadyGo &t)
{
    return s.operand1st->GetUUIDTag() != evt.ent->GetUUIDTag() && s.operand2nd->GetUUIDTag() != evt.ent->GetUUIDTag();
}

DiffTool::DiffTool()
    : DiffBoxTool(*this) 
{ 
}

DiffTool::~DiffTool()
{
    for (auto &d : differs)
    {
        d.second.process_event(evt_quit_tool(d.first));
        d.second.stop();
    }
}

void DiffTool::OnMMouseDown(const EvMMouseDown &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto fIt = differs.find(cav->GetUUID());
        if (fIt != differs.cend())
        {
            fIt->second.process_event(evt_apply(cav->GetUUID()));
        }
        else
        {
            auto &differ = differs[cav->GetUUID()];
            differ.start(BinaryBoolOperatorDef::BinaryBooleanType::DiffOp);
            differ.process_event(evt_apply(cav->GetUUID()));
        }
    }
}

void DiffTool::OnEnterCanvas(const EvCanvasEnter &e)
{
    CanvasCursorManipulator::EnterCanvas(e, bm_PathDiff);
    BoxToolT::EnterCanvas(e);
}

void DiffTool::OnEntityClicked(const EvEntityClicked &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.e.GetEventObject());
    if (cav)
    {
        auto fIt = differs.find(cav->GetUUID());
        if (fIt != differs.cend())
        {
            fIt->second.process_event(evt_entity_selected(cav->GetUUID(), e.ent));
        }
        else
        {
            auto &differ = differs[cav->GetUUID()];
            differ.start(BinaryBoolOperatorDef::BinaryBooleanType::DiffOp);
            differ.process_event(evt_entity_selected(cav->GetUUID(), e.ent));
        }
    }
}

XORTool::XORTool()
    : XORBoxTool(*this)
{
}

XORTool::~XORTool()
{
    for (auto &d : XORers)
    {
        d.second.process_event(evt_quit_tool(d.first));
        d.second.stop();
    }
}

void XORTool::OnMMouseDown(const EvMMouseDown &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.evData.GetEventObject());
    if (cav)
    {
        auto fIt = XORers.find(cav->GetUUID());
        if (fIt != XORers.cend())
        {
            fIt->second.process_event(evt_apply(cav->GetUUID()));
        }
        else
        {
            auto &XORer = XORers[cav->GetUUID()];
            XORer.start(BinaryBoolOperatorDef::BinaryBooleanType::XOROp);
            XORer.process_event(evt_apply(cav->GetUUID()));
        }
    }
}

void XORTool::OnEnterCanvas(const EvCanvasEnter &e)
{
    CanvasCursorManipulator::EnterCanvas(e, bm_PathXOR);
    BoxToolT::EnterCanvas(e);
}

void XORTool::OnEntityClicked(const EvEntityClicked &e)
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.e.GetEventObject());
    if (cav)
    {
        auto fIt = XORers.find(cav->GetUUID());
        if (fIt != XORers.cend())
        {
            fIt->second.process_event(evt_entity_selected(cav->GetUUID(), e.ent));
        }
        else
        {
            auto &XORer = XORers[cav->GetUUID()];
            XORer.start(BinaryBoolOperatorDef::BinaryBooleanType::XOROp);
            XORer.process_event(evt_entity_selected(cav->GetUUID(), e.ent));
        }
    }
}