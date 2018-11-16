#ifndef SPAM_UI_FSM_NODE_EDIT_TOOL_H
#define SPAM_UI_FSM_NODE_EDIT_TOOL_H
#include "spamer.h"
#include <ui/projs/modelfwd.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )
#include <unordered_map>

struct NodeEditIdle;
struct NodeEditBoxSelecting;
struct NodeEditing;

struct EntitySelection
{
    SPDrawableNodeVector ents;
    SpamMany  mementos;
    std::vector<SelectionState> states;
};

struct NodeEditTool : boost::statechart::simple_state<NodeEditTool, Spamer, NodeEditIdle>
{
    NodeEditTool();
    ~NodeEditTool();

    // Box selecting actions
    void OnBoxingStart(const EvLMouseDown &e);
    void OnBoxing(const EvMouseMove &e);
    void OnBoxingEnd(const EvLMouseUp &e);
    void OnBoxingReset(const EvReset &e);

    // NodeEditing actions
    void OnNodeEditingStart(const EvLMouseDown &e);
    void OnNodeEditing(const EvMouseMove &e);
    void OnNodeEditingEnd(const EvLMouseUp &e);
    void OnNodeEditingReset(const EvReset &e);

    void OnCanvasEnter(const EvCanvasEnter &e);
    void OnCanvasLeave(const EvCanvasLeave &e);
    void OnSafari(const EvMouseMove &e);
    void OnAppQuit(const EvAppQuit &e);
    void OnDrawableDelete(const EvDrawableDelete &e);
    void OnDrawableSelect(const EvDrawableSelect &e);

    void ClearSelection(const std::string &uuid);
    void ClearHighlightData() 
    {
        hlData.hls = HighlightState::kHlNone;
        hlData.id = -1;
        hlData.subid = -1;
    }

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, NodeEditTool>,
        boost::statechart::in_state_reaction<EvAppQuit, NodeEditTool, &NodeEditTool::OnAppQuit>,
        boost::statechart::in_state_reaction<EvDrawableDelete, NodeEditTool, &NodeEditTool::OnDrawableDelete>,
        boost::statechart::in_state_reaction<EvDrawableSelect, NodeEditTool, &NodeEditTool::OnDrawableSelect>,
        boost::statechart::in_state_reaction<EvCanvasEnter, NodeEditTool, &NodeEditTool::OnCanvasEnter>,
        boost::statechart::in_state_reaction<EvCanvasLeave, NodeEditTool, &NodeEditTool::OnCanvasLeave>> reactions;

    Geom::Point anchor;
    Geom::Point last;
    Geom::OptRect rect;
    SPDrawableNode highlight;
    HighlightData  hlData;

    std::unordered_map<std::string, EntitySelection> selData;
};

struct NodeEditIdle : boost::statechart::simple_state<NodeEditIdle, NodeEditTool>
{
    NodeEditIdle();
    ~NodeEditIdle();
    typedef boost::mpl::list<
        boost::statechart::custom_reaction<EvLMouseDown>,
        boost::statechart::custom_reaction<EvToolQuit>,
        boost::statechart::in_state_reaction<EvMouseMove, NodeEditTool, &NodeEditTool::OnSafari>> reactions;

    sc::result react(const EvToolQuit &e);
    sc::result react(const EvLMouseDown &e);
};

struct NodeEditBoxSelecting : boost::statechart::simple_state<NodeEditBoxSelecting, NodeEditTool>
{
    NodeEditBoxSelecting();
    ~NodeEditBoxSelecting();

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, NodeEditIdle, NodeEditTool, &NodeEditTool::OnBoxingEnd>,
        boost::statechart::transition<EvReset, NodeEditIdle, NodeEditTool, &NodeEditTool::OnBoxingReset>,
        boost::statechart::in_state_reaction<EvMouseMove, NodeEditTool, &NodeEditTool::OnBoxing>> reactions;
};

struct NodeEditing : boost::statechart::simple_state<NodeEditing, NodeEditTool>
{
    NodeEditing();
    ~NodeEditing();

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, NodeEditIdle, NodeEditTool, &NodeEditTool::OnNodeEditingEnd>,
        boost::statechart::transition<EvReset, NodeEditIdle, NodeEditTool, &NodeEditTool::OnNodeEditingReset>,
        boost::statechart::in_state_reaction<EvMouseMove, NodeEditTool, &NodeEditTool::OnNodeEditing>> reactions;
};

#endif //SPAM_UI_FSM_NODE_EDIT_TOOL_H