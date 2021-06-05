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
struct RegionTool;
struct HistogramTool;
struct ProbeIdle;
struct RegionIdle;
struct HistogramIdle;
struct ProbeDraging;
struct RegionDraging;
struct HistogramDraging;
struct ProfileIdle;
struct ProfileDraging;
struct ProfileTracing;
using  ProbeBoxTool = BoxTool<ProbeTool, kSpamID_TOOLBOX_PROBE_SELECT>;
using  RegionBoxTool = BoxTool<RegionTool, kSpamID_TOOLBOX_PROBE_REGION>;
using  HistogramBoxTool = BoxTool<HistogramTool, kSpamID_TOOLBOX_PROBE_HISTOGRAM>;

struct ProbeTool : boost::statechart::simple_state<ProbeTool, Spamer, ProbeIdle>, ProbeBoxTool
{
    using BoxToolT = BoxToolImpl;
    ProbeTool() : ProbeBoxTool(*this) { wxLogMessage(wxT("ProbeTool Enter.")); }
    ~ProbeTool() { wxLogMessage(wxT("ProbeTool Quit.")); }

    void OnOptionChanged(const EvToolOption &e);
    void OnImageClicked(const EvImageClicked &e);

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, ProbeTool>,
        boost::statechart::transition<EvToolQuit, NoTool>,
        boost::statechart::in_state_reaction<EvAppQuit,        BoxToolT,  &BoxToolT::QuitApp>,
        boost::statechart::in_state_reaction<EvImageClicked,   ProbeTool, &ProbeTool::OnImageClicked>,
        boost::statechart::in_state_reaction<EvDrawableDelete, BoxToolT,  &BoxToolT::DeleteDrawable>,
        boost::statechart::in_state_reaction<EvDrawableSelect, BoxToolT,  &BoxToolT::SelectDrawable>> reactions;

    ToolOptions toolOptions;
};

struct ProbeIdle : boost::statechart::simple_state<ProbeIdle, ProbeTool>
{
    ProbeIdle() { wxLogMessage(wxT("ProbeIdle Enter.")); }
    ~ProbeIdle() { wxLogMessage(wxT("ProbeIdle Quit.")); }

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
    ProbeDraging() { wxLogMessage(wxT("ProbeDraging Enter.")); }
    ~ProbeDraging() { wxLogMessage(wxT("ProbeDraging Quit.")); }

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, ProbeIdle, ProbeTool::BoxToolT, &ProbeTool::BoxToolT::EndBoxing>,
        boost::statechart::transition<EvReset, ProbeIdle, ProbeTool::BoxToolT, &ProbeTool::BoxToolT::ResetBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, ProbeTool::BoxToolT, &ProbeTool::BoxToolT::ContinueBoxing>> reactions;
};

struct HistogramTool : boost::statechart::simple_state<HistogramTool, Spamer, HistogramIdle>, HistogramBoxTool
{
    using BoxToolT = BoxToolImpl;
    HistogramTool() : HistogramBoxTool(*this) { wxLogMessage(wxT("HistogramTool Enter.")); }
    ~HistogramTool() { wxLogMessage(wxT("HistogramTool Quit.")); }

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
    HistogramIdle() { wxLogMessage(wxT("HistogramIdle Enter.")); }
    ~HistogramIdle() { wxLogMessage(wxT("HistogramIdle Quit.")); }

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, HistogramDraging, HistogramTool::BoxToolT, &HistogramTool::BoxToolT::StartBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, HistogramTool::BoxToolT, &HistogramTool::BoxToolT::Safari>,
        boost::statechart::in_state_reaction<EvToolOption, HistogramTool, &HistogramTool::OnOptionChanged>,
        boost::statechart::in_state_reaction<EvCanvasLeave, HistogramTool::BoxToolT, &HistogramTool::BoxToolT::LeaveCanvas>> reactions;
};

struct HistogramDraging : boost::statechart::simple_state<HistogramDraging, HistogramTool>
{
    HistogramDraging() { wxLogMessage(wxT("HistogramDraging Enter.")); }
    ~HistogramDraging() { wxLogMessage(wxT("HistogramDraging Quit.")); }

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, HistogramIdle, HistogramTool::BoxToolT, &HistogramTool::BoxToolT::EndBoxing>,
        boost::statechart::transition<EvReset, HistogramIdle, HistogramTool::BoxToolT, &HistogramTool::BoxToolT::ResetBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, HistogramTool::BoxToolT, &HistogramTool::BoxToolT::ContinueBoxing>> reactions;
};

struct RegionTool : boost::statechart::simple_state<RegionTool, Spamer, RegionIdle>, RegionBoxTool
{
    using BoxToolT = BoxToolImpl;
    RegionTool() : RegionBoxTool(*this) { wxLogMessage(wxT("RegionTool Enter.")); }
    ~RegionTool() { wxLogMessage(wxT("RegionTool Quit.")); }

    void OnOptionChanged(const EvToolOption &e);
    void OnRegionClicked(const EvEntityClicked &e);
    void OnRegionBoxed(const EvEntityBoxed &e);
    void OnRegionHighlight(const EvEntityHighlight &e);
    void OnRegionLoseHighlight(const EvEntityLoseHighlight &e);

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, RegionTool>,
        boost::statechart::transition<EvToolQuit, NoTool>,
        boost::statechart::in_state_reaction<EvAppQuit, BoxToolT, &BoxToolT::QuitApp>,
        boost::statechart::in_state_reaction<EvEntityHighlight, RegionTool, &RegionTool::OnRegionHighlight>,
        boost::statechart::in_state_reaction<EvEntityLoseHighlight, RegionTool, &RegionTool::OnRegionLoseHighlight>,
        boost::statechart::in_state_reaction<EvEntityClicked, RegionTool, &RegionTool::OnRegionClicked>,
        boost::statechart::in_state_reaction<EvEntityBoxed, RegionTool, &RegionTool::OnRegionBoxed>,
        boost::statechart::in_state_reaction<EvDrawableDelete, BoxToolT, &BoxToolT::DeleteDrawable>,
        boost::statechart::in_state_reaction<EvDrawableSelect, BoxToolT, &BoxToolT::SelectDrawable>> reactions;

    ToolOptions toolOptions;
};

struct RegionIdle : boost::statechart::simple_state<RegionIdle, RegionTool>
{
    RegionIdle() { wxLogMessage(wxT("RegionIdle Enter.")); }
    ~RegionIdle() { wxLogMessage(wxT("RegionIdle Quit.")); }

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, RegionDraging, RegionTool::BoxToolT, &RegionTool::BoxToolT::StartBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, RegionTool::BoxToolT, &RegionTool::BoxToolT::Safari>,
        boost::statechart::in_state_reaction<EvToolOption, RegionTool, &RegionTool::OnOptionChanged>,
        boost::statechart::in_state_reaction<EvCanvasLeave, RegionTool::BoxToolT, &RegionTool::BoxToolT::LeaveCanvas>> reactions;
};

struct RegionDraging : boost::statechart::simple_state<RegionDraging, RegionTool>
{
    RegionDraging() { wxLogMessage(wxT("RegionDraging Enter.")); }
    ~RegionDraging() { wxLogMessage(wxT("RegionDraging Quit.")); }

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, RegionIdle, RegionTool::BoxToolT, &RegionTool::BoxToolT::EndBoxing>,
        boost::statechart::transition<EvReset, RegionIdle, RegionTool::BoxToolT, &RegionTool::BoxToolT::ResetBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, RegionTool::BoxToolT, &RegionTool::BoxToolT::ContinueBoxing>> reactions;
};

#endif //SPAM_UI_FSM_PROBE_TOOL_H