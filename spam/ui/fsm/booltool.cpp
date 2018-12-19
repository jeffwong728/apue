#include "booltool.h"
#include <wx/log.h>
#include <ui/spam.h>
#include <ui/cv/cairocanvas.h>
#include <ui/toplevel/rootframe.h>

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


void DiffOperatorDef::invalidate_operands::operator()(evt_quit_tool const& e, DiffOperatorDef&, Wait2ndOperand &s, Wait2ndOperand& t)
{
    Spam::InvalidateCanvasRect(e.uuid, s.operand1st->GetBoundingBox());
}

void DiffOperatorDef::invalidate_operands::operator()(evt_quit_tool const& e, DiffOperatorDef&, ReadyGo &s, ReadyGo& t)
{
    Spam::InvalidateCanvasRect(e.uuid, s.operand1st->GetBoundingBox());
    Spam::InvalidateCanvasRect(e.uuid, s.operand2nd->GetBoundingBox());
}

void DiffOperatorDef::wrap_operand::operator()(evt_entity_selected const& e, DiffOperatorDef &dop, ReadyGo &s, ReadyGo& t)
{
    Spam::InvalidateCanvasRect(e.uuid, s.operand1st->GetBoundingBox());
    s.operand1st->RestoreColor();
    t.operand1st = s.operand2nd;
    t.operand2nd = e.ent;
    t.operand1st->ChangeColorToSelected();
    t.operand2nd->ChangeColorToSelected();
}

void DiffOperatorDef::do_diff::operator()(const evt_apply & e, DiffOperatorDef&, DiffOperatorDef::ReadyGo& s, DiffOperatorDef::Wait1stOperand& t)
{
    auto frame = dynamic_cast<RootFrame *>(wxTheApp->GetTopWindow());
    if (frame)
    {
        CairoCanvas *cav = frame->FindCanvasByUUID(e.uuid);
        if (cav)
        {
            cav->DoDifference(s.operand1st, s.operand2nd);
        }
    }
}
bool DiffOperatorDef::valid_operand::operator()(evt_entity_selected const& evt, DiffOperatorDef&, Wait1stOperand&, Wait2ndOperand&)
{
    return true;
}

bool DiffOperatorDef::valid_operand::operator()(evt_entity_selected const& evt, DiffOperatorDef&, Wait2ndOperand &s, ReadyGo &t)
{
    return s.operand1st->GetUUIDTag() != evt.ent->GetUUIDTag();
}

bool DiffOperatorDef::valid_operand::operator()(evt_entity_selected const& evt, DiffOperatorDef&, ReadyGo &s, ReadyGo &t)
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
            differ.start();
            differ.process_event(evt_apply(cav->GetUUID()));
        }
    }
}

void DiffTool::FireClickEntity(const SPDrawableNode &ent, const wxMouseEvent &e, const Geom::Point &pt, const SelectionData &sd) const
{
    CairoCanvas *cav = dynamic_cast<CairoCanvas *>(e.GetEventObject());
    if (cav)
    {
        auto fIt = differs.find(cav->GetUUID());
        if (fIt != differs.cend())
        {
            fIt->second.process_event(evt_entity_selected(cav->GetUUID(), ent));
        }
        else
        {
            auto &differ = differs[cav->GetUUID()];
            differ.start();
            differ.process_event(evt_entity_selected(cav->GetUUID(), ent));
        }
    }
}