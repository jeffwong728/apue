#ifndef SPAM_UI_FSM_PROC_TOOL_H
#define SPAM_UI_FSM_PROC_TOOL_H
#include "spamer.h"
#include "notool.h"
#include "boxtool.h"
#include <set>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )

struct FilterTool;
struct FilterIdle;
struct FilterDraging;
using  FilterBoxTool = BoxTool<FilterTool, kSpamID_TOOLBOX_PROC_FILTER>;

struct FilterTool : boost::statechart::simple_state<FilterTool, Spamer, FilterIdle>, FilterBoxTool
{
    using BoxToolT = BoxToolImpl;
    FilterTool() : FilterBoxTool(*this) {}
    ~FilterTool() {}

    void OnOptionChanged(const EvToolOption &e);
    void OnBoxingEnded(const EvBoxingEnded &e);
    void OnImageClicked(const EvImageClicked &e);
    void OnEntityClicked(const EvEntityClicked &e);
    sc::result react(const EvToolQuit &e);

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, FilterTool>,
        boost::statechart::in_state_reaction<EvAppQuit, BoxToolT, &BoxToolT::QuitApp>,
        boost::statechart::in_state_reaction<EvBoxingEnded, FilterTool, &FilterTool::OnBoxingEnded>,
        boost::statechart::in_state_reaction<EvImageClicked, FilterTool, &FilterTool::OnImageClicked>,
        boost::statechart::in_state_reaction<EvEntityClicked, FilterTool, &FilterTool::OnEntityClicked>,
        boost::statechart::in_state_reaction<EvDrawableDelete, BoxToolT, &BoxToolT::DeleteDrawable>,
        boost::statechart::in_state_reaction<EvDrawableSelect, BoxToolT, &BoxToolT::SelectDrawable>,
        boost::statechart::custom_reaction<EvToolQuit>> reactions;

    ToolOptions toolOptions;
    std::set<std::string> uuids;
};

struct FilterIdle : boost::statechart::simple_state<FilterIdle, FilterTool>
{
    FilterIdle() {}
    ~FilterIdle() {}

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, FilterDraging, FilterTool::BoxToolT, &FilterTool::BoxToolT::StartBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, FilterTool::BoxToolT, &FilterTool::BoxToolT::Safari>,
        boost::statechart::in_state_reaction<EvToolOption, FilterTool, &FilterTool::OnOptionChanged>,
        boost::statechart::in_state_reaction<EvCanvasLeave, FilterTool::BoxToolT, &FilterTool::BoxToolT::LeaveCanvas>> reactions;
};

struct FilterDraging : boost::statechart::simple_state<FilterDraging, FilterTool>
{
    FilterDraging() {}
    ~FilterDraging() {}

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, FilterIdle, FilterTool::BoxToolT, &FilterTool::BoxToolT::EndBoxing>,
        boost::statechart::transition<EvReset, FilterIdle, FilterTool::BoxToolT, &FilterTool::BoxToolT::ResetBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, FilterTool::BoxToolT, &FilterTool::BoxToolT::ContinueBoxing>> reactions;
};

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
    sc::result react(const EvToolQuit &e);

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, ThresholdTool>,
        boost::statechart::in_state_reaction<EvAppQuit, BoxToolT, &BoxToolT::QuitApp>,
        boost::statechart::in_state_reaction<EvBoxingEnded, ThresholdTool, &ThresholdTool::OnBoxingEnded>,
        boost::statechart::in_state_reaction<EvImageClicked, ThresholdTool, &ThresholdTool::OnImageClicked>,
        boost::statechart::in_state_reaction<EvEntityClicked, ThresholdTool, &ThresholdTool::OnEntityClicked>,
        boost::statechart::in_state_reaction<EvDrawableDelete, BoxToolT, &BoxToolT::DeleteDrawable>,
        boost::statechart::in_state_reaction<EvDrawableSelect, BoxToolT, &BoxToolT::SelectDrawable>,
        boost::statechart::custom_reaction<EvToolQuit>> reactions;

    ToolOptions toolOptions;
    std::set<std::string> uuids;
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