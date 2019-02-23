#ifndef SPAM_UI_FSM_PROC_TOOL_H
#define SPAM_UI_FSM_PROC_TOOL_H
#include "spamer.h"
#include "notool.h"
#include "boxtool.h"
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )

struct ThresholdTool;
struct ThresholdIdle;
struct ThresholdDraging;
using  ThresholdBoxTool = BoxTool<ThresholdTool, kSpamID_TOOLBOX_PROC_THRESHOLD>;

struct ThresholdTool : boost::statechart::simple_state<ThresholdTool, Spamer, ThresholdIdle>, ThresholdBoxTool
{
    using BoxToolT = BoxToolImpl;
    ThresholdTool() : ThresholdBoxTool(*this) {}
    ~ThresholdTool() {}

    void OnOptionChanged(const EvToolOption &e);
    void OnBoxingEnded(const EvBoxingEnded &e);
    void OnImageClicked(const EvImageClicked &e);
    void OnEntityClicked(const EvEntityClicked &e);

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, ThresholdTool>,
        boost::statechart::transition<EvToolQuit, NoTool>,
        boost::statechart::in_state_reaction<EvAppQuit, BoxToolT, &BoxToolT::QuitApp>,
        boost::statechart::in_state_reaction<EvBoxingEnded, ThresholdTool, &ThresholdTool::OnBoxingEnded>,
        boost::statechart::in_state_reaction<EvImageClicked, ThresholdTool, &ThresholdTool::OnImageClicked>,
        boost::statechart::in_state_reaction<EvEntityClicked, ThresholdTool, &ThresholdTool::OnEntityClicked>,
        boost::statechart::in_state_reaction<EvDrawableDelete, BoxToolT, &BoxToolT::DeleteDrawable>,
        boost::statechart::in_state_reaction<EvDrawableSelect, BoxToolT, &BoxToolT::SelectDrawable>> reactions;

    ToolOptions toolOptions;
};

struct ThresholdIdle : boost::statechart::simple_state<ThresholdIdle, ThresholdTool>
{
    ThresholdIdle() {}
    ~ThresholdIdle() {}

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, ThresholdDraging, ThresholdTool::BoxToolT, &ThresholdTool::BoxToolT::StartBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, ThresholdTool::BoxToolT, &ThresholdTool::BoxToolT::Safari>,
        boost::statechart::in_state_reaction<EvToolOption, ThresholdTool, &ThresholdTool::OnOptionChanged>,
        boost::statechart::in_state_reaction<EvCanvasLeave, ThresholdTool::BoxToolT, &ThresholdTool::BoxToolT::LeaveCanvas>> reactions;
};

struct ThresholdDraging : boost::statechart::simple_state<ThresholdDraging, ThresholdTool>
{
    ThresholdDraging() {}
    ~ThresholdDraging() {}

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, ThresholdIdle, ThresholdTool::BoxToolT, &ThresholdTool::BoxToolT::EndBoxing>,
        boost::statechart::transition<EvReset, ThresholdIdle, ThresholdTool::BoxToolT, &ThresholdTool::BoxToolT::ResetBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, ThresholdTool::BoxToolT, &ThresholdTool::BoxToolT::ContinueBoxing>> reactions;
};

#endif //SPAM_UI_FSM_PROC_TOOL_H