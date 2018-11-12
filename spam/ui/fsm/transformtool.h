#ifndef SPAM_UI_FSM_TRANSFORM_TOOL_H
#define SPAM_UI_FSM_TRANSFORM_TOOL_H
#include "spamer.h"
#include <ui/projs/modelfwd.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )

struct TransformIdle;
struct TransformBoxSelecting;
struct Transforming;

struct TransformTool : boost::statechart::simple_state<TransformTool, Spamer, TransformIdle>
{
    TransformTool();
    ~TransformTool();

    // Box selecting actions
    void OnBoxingStart(const EvLMouseDown &e);
    void OnBoxing(const EvMouseMove &e);
    void OnBoxingEnd(const EvLMouseUp &e);
    void OnBoxingReset(const EvReset &e);

    // Transforming actions
    void OnTransformingStart(const EvLMouseDown &e);
    void OnTransforming(const EvMouseMove &e);
    void OnTransformingEnd(const EvLMouseUp &e);
    void OnTransformingReset(const EvReset &e);

    void OnCanvasEnter(const EvCanvasEnter &e);
    void OnCanvasLeave(const EvCanvasLeave &e);
    void OnSafari(const EvMouseMove &e);

    void ClearSelection();

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, TransformTool>,
        boost::statechart::in_state_reaction<EvCanvasEnter, TransformTool, &TransformTool::OnCanvasEnter>,
        boost::statechart::in_state_reaction<EvCanvasLeave, TransformTool, &TransformTool::OnCanvasLeave>> reactions;

    Geom::Point anchor;
    Geom::Point last;
    Geom::OptRect rect;
    SPDrawableNode highlight;

    SPDrawableNodeVector selEnts;
    std::vector<SelectionState> selStates;
};

struct TransformIdle : boost::statechart::simple_state<TransformIdle, TransformTool>
{
    TransformIdle();
    ~TransformIdle();
    typedef boost::mpl::list<
        boost::statechart::custom_reaction<EvLMouseDown>,
        boost::statechart::custom_reaction<EvToolQuit>,
        boost::statechart::in_state_reaction<EvMouseMove, TransformTool, &TransformTool::OnSafari>> reactions;

    sc::result react(const EvToolQuit &e);
    sc::result react(const EvLMouseDown &e);
};

struct TransformBoxSelecting : boost::statechart::simple_state<TransformBoxSelecting, TransformTool>
{
    TransformBoxSelecting();
    ~TransformBoxSelecting();

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, TransformIdle, TransformTool, &TransformTool::OnBoxingEnd>,
        boost::statechart::transition<EvReset, TransformIdle, TransformTool, &TransformTool::OnBoxingReset>,
        boost::statechart::in_state_reaction<EvMouseMove, TransformTool, &TransformTool::OnBoxing>> reactions;
};

struct Transforming : boost::statechart::simple_state<Transforming, TransformTool>
{
    Transforming();
    ~Transforming();

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, TransformIdle, TransformTool, &TransformTool::OnTransformingEnd>,
        boost::statechart::transition<EvReset, TransformIdle, TransformTool, &TransformTool::OnTransformingReset>,
        boost::statechart::in_state_reaction<EvMouseMove, TransformTool, &TransformTool::OnTransforming>> reactions;
};

#endif //SPAM_UI_FSM_TRANSFORM_TOOL_H