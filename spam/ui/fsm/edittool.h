#ifndef SPAM_UI_FSM_EDIT_TOOL_H
#define SPAM_UI_FSM_EDIT_TOOL_H
#include "spamer.h"
#include "boxtool.h"

struct EditToolImpl : public BoxToolImpl
{
    EditToolImpl(const int tid) : BoxToolImpl(tid){}
    ~EditToolImpl() {}

    void StartEditing(const EvLMouseDown &e);
    void ContinueEditing(const EvMouseMove &e);
    void EndEditing(const EvLMouseUp &e);
    void ResetEditing(const EvReset &e);

    Geom::Point last;
};

struct EditIdleImpl
{
    enum NextAction { editing, boxing, discard };
    EditIdleImpl(const int tid) : toolId(tid) {}
    ~EditIdleImpl() {}

    NextAction GetNextAction(const EvLMouseDown &e);

    virtual void FireDeselectEntity(const SPDrawableNodeVector &ents) const = 0;
    virtual void FireSelectEntity(const SPDrawableNodeVector &ents) const = 0;
    virtual EntitySelection &GetSelections(const std::string &uuid) = 0;
    virtual void ClearSelections(const std::string &uuid) = 0;

    const int toolId;
};

template<typename ToolT, int ToolId>
struct EditTool : public EditToolImpl
{
    EditTool(ToolT &t) : EditToolImpl(ToolId), tool(t) {}
    ~EditTool() { BoxToolImpl::QuitTool(EvToolQuit(ToolId)); }

    void FireDeselectEntity(const SPDrawableNodeVector &ents) const override { tool.template context<Spamer>().sig_EntityDesel(ents); }
    void FireSelectEntity(const SPDrawableNodeVector &ents) const override { tool.template context<Spamer>().sig_EntitySel(ents); }
    void FireDimEntity(const SPDrawableNode &ent) const override { tool.template context<Spamer>().sig_EntityDim(ent); }
    void FireGlowEntity(const SPDrawableNode &ent) const override { tool.template context<Spamer>().sig_EntityGlow(ent); }

    ToolT &tool;
};

template<typename IdleT, typename ToolT, typename EditingT, typename BoxingT, int ToolId>
struct EditIdle : EditIdleImpl
{
    EditIdle(IdleT &idl) : EditIdleImpl(ToolId), idle(idl) {}

    void FireDeselectEntity(const SPDrawableNodeVector &ents) const override { idle.template context<Spamer>().sig_EntityDesel(ents); }
    void FireSelectEntity(const SPDrawableNodeVector &ents) const override { idle.template context<Spamer>().sig_EntitySel(ents); }
    EntitySelection &GetSelections(const std::string &uuid) override { return idle.template context<ToolT>().selData[uuid]; }
    void ClearSelections(const std::string &uuid) override { return idle.template context<ToolT>().ClearSelection(uuid); }

    sc::result reactLMouseDown(const EvLMouseDown &e)
    {
        NextAction na = EditIdleImpl::GetNextAction(e);
        switch (na)
        {
        case editing: return idle.template transit<EditingT>(&ToolT::EditToolT::StartEditing, e);
        case boxing: return idle.template transit<BoxingT>(&ToolT::BoxToolT::StartBoxing, e);
        default: return idle.discard_event();
        }
    }

    IdleT &idle;
};

#endif //SPAM_UI_FSM_EDIT_TOOL_H