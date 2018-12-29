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
struct ProbeIdle;
struct ProbeDraging;
using  ProbeBoxTool = BoxTool<ProbeTool, kSpamID_TOOLBOX_PROBE_SELECT>;

struct ProbeTool : boost::statechart::simple_state<ProbeTool, Spamer, ProbeIdle>, ProbeBoxTool
{
    using BoxToolT = BoxToolImpl;
    ProbeTool() : ProbeBoxTool(*this) {}
    ~ProbeTool() {}

    void FireClickEntity(const SPDrawableNode &ent, const wxMouseEvent &e, const Geom::Point &pt, const SelectionData &sd) const override;

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, ProbeTool>,
        boost::statechart::transition<EvToolQuit, NoTool>,
        boost::statechart::in_state_reaction<EvAppQuit, BoxToolT, &BoxToolT::QuitApp>,
        boost::statechart::in_state_reaction<EvDrawableDelete, BoxToolT, &BoxToolT::DeleteDrawable>,
        boost::statechart::in_state_reaction<EvDrawableSelect, BoxToolT, &BoxToolT::SelectDrawable>> reactions;
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

#endif //SPAM_UI_FSM_PROBE_TOOL_H