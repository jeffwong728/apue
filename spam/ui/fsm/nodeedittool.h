#ifndef SPAM_UI_FSM_NODE_EDIT_TOOL_H
#define SPAM_UI_FSM_NODE_EDIT_TOOL_H
#include "spamer.h"
#include "notool.h"
#include "edittool.h"
#include <ui/projs/modelfwd.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )
#include <unordered_map>

struct NodeEditIdle;
struct NodeEditBoxSelecting;
struct NodeEditing;
struct NodeEditTool;

using NodeEditToolImpl = EditTool<NodeEditTool, kSpamID_TOOLBOX_GEOM_EDIT>;
using NodeEditIdleImpl = EditIdle<NodeEditIdle, NodeEditTool, NodeEditing, NodeEditBoxSelecting, kSpamID_TOOLBOX_GEOM_EDIT>;

struct NodeEditTool : sc::simple_state<NodeEditTool, Spamer, NodeEditIdle>, NodeEditToolImpl
{
    using BoxToolT  = BoxToolImpl;
    using EditToolT = EditToolImpl;

    NodeEditTool();
    ~NodeEditTool();

    typedef boost::mpl::list<
        sc::transition<EvReset, NodeEditTool>,
        sc::transition<EvToolQuit, NoTool>,
        sc::in_state_reaction<EvAppQuit, BoxToolT, &BoxToolT::QuitApp>,
        sc::in_state_reaction<EvDrawableDelete, BoxToolT, &BoxToolT::DeleteDrawable>,
        sc::in_state_reaction<EvDrawableSelect, BoxToolT, &BoxToolT::SelectDrawable>,
        sc::in_state_reaction<EvCanvasEnter, BoxToolT, &BoxToolT::EnterCanvas>,
        sc::in_state_reaction<EvCanvasLeave, BoxToolT, &BoxToolT::LeaveCanvas>> reactions;
};

struct NodeEditIdle : sc::simple_state<NodeEditIdle, NodeEditTool>, NodeEditIdleImpl
{
    using EditIdleT = NodeEditIdleImpl;

    NodeEditIdle();
    ~NodeEditIdle();
    typedef boost::mpl::list<
        sc::custom_reaction<EvLMouseDown>,
        sc::in_state_reaction<EvMouseMove, NodeEditTool::BoxToolT, &NodeEditTool::BoxToolT::Safari>> reactions;

    sc::result react(const EvLMouseDown &e);
};

struct NodeEditBoxSelecting : sc::simple_state<NodeEditBoxSelecting, NodeEditTool>
{
    NodeEditBoxSelecting();
    ~NodeEditBoxSelecting();

    typedef boost::mpl::list<
        sc::transition<EvLMouseUp, NodeEditIdle, NodeEditTool::BoxToolT, &NodeEditTool::BoxToolT::EndBoxing>,
        sc::transition<EvReset, NodeEditIdle, NodeEditTool::BoxToolT, &NodeEditTool::BoxToolT::ResetBoxing>,
        sc::in_state_reaction<EvMouseMove, NodeEditTool::BoxToolT, &NodeEditTool::BoxToolT::ContinueBoxing>> reactions;
};

struct NodeEditing : sc::simple_state<NodeEditing, NodeEditTool>
{
    NodeEditing();
    ~NodeEditing();

    typedef boost::mpl::list<
        sc::transition<EvLMouseUp, NodeEditIdle, NodeEditTool::EditToolT, &NodeEditTool::EditToolT::EndEditing>,
        sc::transition<EvReset, NodeEditIdle, NodeEditTool::EditToolT, &NodeEditTool::EditToolT::ResetEditing>,
        sc::in_state_reaction<EvMouseMove, NodeEditTool::EditToolT, &NodeEditTool::EditToolT::ContinueEditing>> reactions;
};

#endif //SPAM_UI_FSM_NODE_EDIT_TOOL_H