#ifndef SPAM_UI_FSM_PYRAMID_TOOL_H
#define SPAM_UI_FSM_PYRAMID_TOOL_H
#include "spamer.h"
#include "notool.h"
#include "boxtool.h"
#include <set>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )

struct PyramidTool;
struct PyramidIdle;
struct PyramidDraging;
using  PyramidBoxTool = BoxTool<PyramidTool, kSpamID_TOOLBOX_PROC_PYRAMID>;

struct PyramidTool : boost::statechart::simple_state<PyramidTool, Spamer, PyramidIdle>, PyramidBoxTool
{
    using BoxToolT = BoxToolImpl;
    PyramidTool() : PyramidBoxTool(*this) {}
    ~PyramidTool() {}

    void OnOptionChanged(const EvToolOption &e);
    void OnBoxingEnded(const EvBoxingEnded &e);
    void OnImageClicked(const EvImageClicked &e);
    void OnEntityClicked(const EvEntityClicked &e);
    sc::result react(const EvToolQuit &e);

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, PyramidTool>,
        boost::statechart::in_state_reaction<EvAppQuit, BoxToolT, &BoxToolT::QuitApp>,
        boost::statechart::in_state_reaction<EvBoxingEnded, PyramidTool, &PyramidTool::OnBoxingEnded>,
        boost::statechart::in_state_reaction<EvImageClicked, PyramidTool, &PyramidTool::OnImageClicked>,
        boost::statechart::in_state_reaction<EvEntityClicked, PyramidTool, &PyramidTool::OnEntityClicked>,
        boost::statechart::in_state_reaction<EvDrawableDelete, BoxToolT, &BoxToolT::DeleteDrawable>,
        boost::statechart::in_state_reaction<EvDrawableSelect, BoxToolT, &BoxToolT::SelectDrawable>,
        boost::statechart::custom_reaction<EvToolQuit>> reactions;

    ToolOptions toolOptions;
    std::set<std::string> uuids;
};

struct PyramidIdle : boost::statechart::simple_state<PyramidIdle, PyramidTool>
{
    PyramidIdle() {}
    ~PyramidIdle() {}

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, PyramidDraging, PyramidTool::BoxToolT, &PyramidTool::BoxToolT::StartBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, PyramidTool::BoxToolT, &PyramidTool::BoxToolT::Safari>,
        boost::statechart::in_state_reaction<EvToolOption, PyramidTool, &PyramidTool::OnOptionChanged>,
        boost::statechart::in_state_reaction<EvCanvasLeave, PyramidTool::BoxToolT, &PyramidTool::BoxToolT::LeaveCanvas>> reactions;
};

struct PyramidDraging : boost::statechart::simple_state<PyramidDraging, PyramidTool>
{
    PyramidDraging() {}
    ~PyramidDraging() {}

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, PyramidIdle, PyramidTool::BoxToolT, &PyramidTool::BoxToolT::EndBoxing>,
        boost::statechart::transition<EvReset, PyramidIdle, PyramidTool::BoxToolT, &PyramidTool::BoxToolT::ResetBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, PyramidTool::BoxToolT, &PyramidTool::BoxToolT::ContinueBoxing>> reactions;
};

#endif //SPAM_UI_FSM_PYRAMID_TOOL_H