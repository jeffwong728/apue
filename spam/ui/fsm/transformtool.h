#ifndef SPAM_UI_FSM_TRANSFORM_TOOL_H
#define SPAM_UI_FSM_TRANSFORM_TOOL_H
#include "spamer.h"
#include "notool.h"
#include "edittool.h"

struct TransformIdle;
struct TransformBoxSelecting;
struct Transforming;
struct TransformTool;

using  TransformEditTool = EditTool<TransformTool, kSpamID_TOOLBOX_GEOM_TRANSFORM>;
using  TransformEditIdle = EditIdle<TransformIdle, TransformTool, Transforming, TransformBoxSelecting, kSpamID_TOOLBOX_GEOM_TRANSFORM>;

struct TransformTool : sc::simple_state<TransformTool, Spamer, TransformIdle>, TransformEditTool
{
    using BoxToolT = BoxToolImpl;
    using EditToolT = EditToolImpl;

    TransformTool();
    ~TransformTool();

    typedef boost::mpl::list<
        sc::transition<EvReset, TransformTool>,
        sc::transition<EvToolQuit, NoTool>,
        sc::in_state_reaction<EvAppQuit, BoxToolT, &BoxToolT::QuitApp>,
        sc::in_state_reaction<EvDrawableDelete, BoxToolT, &BoxToolT::DeleteDrawable>,
        sc::in_state_reaction<EvDrawableSelect, BoxToolT, &BoxToolT::SelectDrawable>,
        sc::in_state_reaction<EvCanvasEnter, BoxToolT, &BoxToolT::EnterCanvas>,
        sc::in_state_reaction<EvCanvasLeave, BoxToolT, &BoxToolT::LeaveCanvas>> reactions;
};

struct TransformIdle : sc::simple_state<TransformIdle, TransformTool>, TransformEditIdle
{
    using IdleT = EditIdle<TransformIdle, TransformTool, Transforming, TransformBoxSelecting, kSpamID_TOOLBOX_GEOM_TRANSFORM>;

    TransformIdle();
    ~TransformIdle();

    typedef boost::mpl::list<
        sc::custom_reaction<EvLMouseDown>,
        sc::in_state_reaction<EvMouseMove, TransformTool::BoxToolT, &TransformTool::BoxToolT::Safari>> reactions;

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