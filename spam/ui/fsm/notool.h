#ifndef SPAM_UI_FSM_NO_TOOL_H
#define SPAM_UI_FSM_NO_TOOL_H
#include "spamer.h"
#include "boxtool.h"
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#include <2geom/pathvector.h>
#pragma warning( pop )

struct NoToolIdle;
struct NoToolDraging;

struct NoTool : boost::statechart::simple_state<NoTool, Spamer, NoToolIdle>, BoxTool<NoTool, -1>
{
    using BoxToolT = BoxTool<NoTool, -1>;
    NoTool();
    ~NoTool();

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, NoTool>,
        boost::statechart::in_state_reaction<EvAppQuit, BoxToolT, &BoxToolT::QuitApp>,
        boost::statechart::in_state_reaction<EvDrawableDelete, BoxToolT, &BoxToolT::DeleteDrawable>,
        boost::statechart::in_state_reaction<EvDrawableSelect, BoxToolT, &BoxToolT::SelectDrawable>,
        boost::statechart::in_state_reaction<EvCanvasLeave, BoxToolT, &BoxToolT::LeaveCanvas>> reactions;
};

struct NoToolIdle : boost::statechart::simple_state<NoToolIdle, NoTool>
{
    NoToolIdle();
    ~NoToolIdle();

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, NoToolDraging, NoTool::BoxToolT, &NoTool::BoxToolT::StartBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, NoTool::BoxToolT, &NoTool::BoxToolT::Safari>,
        boost::statechart::custom_reaction<EvToolEnter>> reactions;

    sc::result react(const EvToolEnter &e);
};

struct NoToolDraging : boost::statechart::simple_state<NoToolDraging, NoTool>
{
    NoToolDraging();
    ~NoToolDraging();

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, NoToolIdle, NoTool::BoxToolT, &NoTool::BoxToolT::EndBoxing>,
        boost::statechart::transition<EvReset, NoToolIdle, NoTool::BoxToolT, &NoTool::BoxToolT::ResetBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, NoTool::BoxToolT, &NoTool::BoxToolT::ContinueBoxing>> reactions;
};

#endif //SPAM_UI_FSM_NO_TOOL_H