#ifndef SPAM_UI_FSM_PROBE_TOOL_H
#define SPAM_UI_FSM_PROBE_TOOL_H
#include "spamer.h"
#include "notool.h"
#include "boxtool.h"
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )

struct ProbeTool;
struct HistogramTool;
struct ProbeIdle;
struct HistogramIdle;
struct ProbeDraging;
struct HistogramDraging;
using  ProbeBoxTool = BoxTool<ProbeTool, kSpamID_TOOLBOX_PROBE_SELECT>;
using  HistogramBoxTool = BoxTool<HistogramTool, kSpamID_TOOLBOX_PROBE_HISTOGRAM>;

struct ProbeTool : boost::statechart::simple_state<ProbeTool, Spamer, ProbeIdle>, ProbeBoxTool
{
    using BoxToolT = BoxToolImpl;
    ProbeTool() : ProbeBoxTool(*this) {}
    ~ProbeTool() {}

    void OnOptionChanged(const EvToolOption &e);
    void OnImageClicked(const EvImageClicked &e);
    void OnRegionClicked(const EvRegionClicked &e);
    void OnContourClicked(const EvContourClicked &e);

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, ProbeTool>,
        boost::statechart::transition<EvToolQuit, NoTool>,
        boost::statechart::in_state_reaction<EvAppQuit,        BoxToolT,  &BoxToolT::QuitApp>,
        boost::statechart::in_state_reaction<EvImageClicked,   ProbeTool, &ProbeTool::OnImageClicked>,
        boost::statechart::in_state_reaction<EvRegionClicked,  ProbeTool, &ProbeTool::OnRegionClicked>,
        boost::statechart::in_state_reaction<EvContourClicked, ProbeTool, &ProbeTool::OnContourClicked>,
        boost::statechart::in_state_reaction<EvDrawableDelete, BoxToolT,  &BoxToolT::DeleteDrawable>,
        boost::statechart::in_state_reaction<EvDrawableSelect, BoxToolT,  &BoxToolT::SelectDrawable>> reactions;

    ToolOptions toolOptions;
};

struct ProbeIdle : boost::statechart::simple_state<ProbeIdle, ProbeTool>
{
    ProbeIdle() {}
    ~ProbeIdle() {}

    void OnSafari(const EvMouseMove &e);
    void OnLeaveCanvas(const EvCanvasLeave &e);

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, ProbeDraging, ProbeTool::BoxToolT, &ProbeTool::BoxToolT::StartBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, ProbeIdle, &ProbeIdle::OnSafari>,
        boost::statechart::in_state_reaction<EvToolOption, ProbeTool, &ProbeTool::OnOptionChanged>,
        boost::statechart::in_state_reaction<EvCanvasLeave, ProbeIdle, &ProbeIdle::OnLeaveCanvas>> reactions;
};

struct ProbeDraging : boost::statechart::simple_state<ProbeDraging, ProbeTool>
{
    ProbeDraging() {}
    ~ProbeDraging() {}

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, ProbeIdle, ProbeTool::BoxToolT, &ProbeTool::BoxToolT::EndBoxing>,
        boost::statechart::transition<EvReset, ProbeIdle, ProbeTool::BoxToolT, &ProbeTool::BoxToolT::ResetBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, ProbeTool::BoxToolT, &ProbeTool::BoxToolT::ContinueBoxing>> reactions;
};

struct HistogramTool : boost::statechart::simple_state<HistogramTool, Spamer, HistogramIdle>, HistogramBoxTool
{
    using BoxToolT = BoxToolImpl;
    HistogramTool() : HistogramBoxTool(*this) {}
    ~HistogramTool() {}

    void OnOptionChanged(const EvToolOption &e);
    void OnBoxingEnded(const EvBoxingEnded &e);
    void OnImageClicked(const EvImageClicked &e);
    void OnEntityClicked(const EvEntityClicked &e);

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, HistogramTool>,
        boost::statechart::transition<EvToolQuit, NoTool>,
        boost::statechart::in_state_reaction<EvAppQuit, BoxToolT, &BoxToolT::QuitApp>,
        boost::statechart::in_state_reaction<EvBoxingEnded, HistogramTool, &HistogramTool::OnBoxingEnded>,
        boost::statechart::in_state_reaction<EvImageClicked, HistogramTool, &HistogramTool::OnImageClicked>,
        boost::statechart::in_state_reaction<EvEntityClicked, HistogramTool, &HistogramTool::OnEntityClicked>,
        boost::statechart::in_state_reaction<EvDrawableDelete, BoxToolT, &BoxToolT::DeleteDrawable>,
        boost::statechart::in_state_reaction<EvDrawableSelect, BoxToolT, &BoxToolT::SelectDrawable>> reactions;

    ToolOptions toolOptions;
};

struct HistogramIdle : boost::statechart::simple_state<HistogramIdle, HistogramTool>
{
    HistogramIdle() {}
    ~HistogramIdle() {}

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, HistogramDraging, HistogramTool::BoxToolT, &HistogramTool::BoxToolT::StartBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, HistogramTool::BoxToolT, &HistogramTool::BoxToolT::Safari>,
        boost::statechart::in_state_reaction<EvToolOption, HistogramTool, &HistogramTool::OnOptionChanged>,
        boost::statechart::in_state_reaction<EvCanvasLeave, HistogramTool::BoxToolT, &HistogramTool::BoxToolT::LeaveCanvas>> reactions;
};

struct HistogramDraging : boost::statechart::simple_state<HistogramDraging, HistogramTool>
{
    HistogramDraging() {}
    ~HistogramDraging() {}

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, HistogramIdle, HistogramTool::BoxToolT, &HistogramTool::BoxToolT::EndBoxing>,
        boost::statechart::transition<EvReset, HistogramIdle, HistogramTool::BoxToolT, &HistogramTool::BoxToolT::ResetBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, HistogramTool::BoxToolT, &HistogramTool::BoxToolT::ContinueBoxing>> reactions;
};

#endif //SPAM_UI_FSM_PROBE_TOOL_H