#ifndef SPAM_UI_FSM_TRANSFORM_TOOL_H
#define SPAM_UI_FSM_TRANSFORM_TOOL_H
#include "spamer.h"
#include "edittool.h"
#include <ui/projs/modelfwd.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )
#include <unordered_map>

struct TransformIdle;
struct TransformBoxSelecting;
struct Transforming;

struct TransformTool : sc::simple_state<TransformTool, Spamer, TransformIdle>, EditTool<TransformTool, kSpamID_TOOLBOX_GEOM_TRANSFORM>
{
    using BoxToolT = BoxTool<TransformTool, kSpamID_TOOLBOX_GEOM_TRANSFORM>;
    using EditToolT = EditTool<TransformTool, kSpamID_TOOLBOX_GEOM_TRANSFORM>;

    TransformTool();
    ~TransformTool();

    typedef boost::mpl::list<
        sc::transition<EvReset, TransformTool>,
        sc::in_state_reaction<EvAppQuit, BoxToolT, &BoxToolT::QuitApp>,
        sc::in_state_reaction<EvDrawableDelete, BoxToolT, &BoxToolT::DeleteDrawable>,
        sc::in_state_reaction<EvDrawableSelect, BoxToolT, &BoxToolT::SelectDrawable>,
        sc::in_state_reaction<EvCanvasEnter, BoxToolT, &BoxToolT::EnterCanvas>,
        sc::in_state_reaction<EvCanvasLeave, BoxToolT, &BoxToolT::LeaveCanvas>> reactions;
};

struct TransformIdle : sc::simple_state<TransformIdle, TransformTool>, EditIdle<TransformIdle, TransformTool, Transforming, TransformBoxSelecting, kSpamID_TOOLBOX_GEOM_TRANSFORM>
{
    using IdleT = EditIdle<TransformIdle, TransformTool, Transforming, TransformBoxSelecting, kSpamID_TOOLBOX_GEOM_TRANSFORM>;

    TransformIdle();
    ~TransformIdle();

    typedef boost::mpl::list<
        sc::custom_reaction<EvLMouseDown>,
        sc::custom_reaction<EvToolQuit>,
        sc::in_state_reaction<EvMouseMove, TransformTool::BoxToolT, &TransformTool::BoxToolT::Safari>> reactions;

    sc::result react(const EvToolQuit &e);
    sc::result react(const EvLMouseDown &e);
};

struct TransformBoxSelecting : sc::simple_state<TransformBoxSelecting, TransformTool>
{
    TransformBoxSelecting();
    ~TransformBoxSelecting();

    typedef boost::mpl::list<
        sc::transition<EvLMouseUp, TransformIdle, TransformTool::BoxToolT, &TransformTool::BoxToolT::EndBoxing>,
        sc::transition<EvReset, TransformIdle, TransformTool::BoxToolT, &TransformTool::BoxToolT::ResetBoxing>,
        sc::in_state_reaction<EvMouseMove, TransformTool::BoxToolT, &TransformTool::BoxToolT::ContinueBoxing>> reactions;
};

struct Transforming : sc::simple_state<Transforming, TransformTool>
{
    Transforming();
    ~Transforming();

    typedef boost::mpl::list<
        sc::transition<EvLMouseUp, TransformIdle, TransformTool::EditToolT, &TransformTool::EditToolT::EndEditing>,
        sc::transition<EvReset, TransformIdle, TransformTool::EditToolT, &TransformTool::EditToolT::ResetEditing>,
        sc::in_state_reaction<EvMouseMove, TransformTool::EditToolT, &TransformTool::EditToolT::ContinueEditing>> reactions;
};

#endif //SPAM_UI_FSM_TRANSFORM_TOOL_H