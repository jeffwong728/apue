#ifndef SPAM_UI_TOOLBOX_PROBE_MACHINE_H
#define SPAM_UI_TOOLBOX_PROBE_MACHINE_H
#include "spamer.h"
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )

struct ProbeIdle;
struct ProbeDraging;

struct ProbeTool : boost::statechart::simple_state<ProbeTool, Spamer, ProbeIdle>
{
    ProbeTool();
    ~ProbeTool();

  void OnStartDraging(const EvLMouseDown &e);
  void OnDraging(const EvMouseMove &e);
  void OnEndDraging(const EvLMouseUp &e);
  void OnReset(const EvReset &e);

  typedef boost::statechart::transition<EvReset, ProbeTool> reactions;

  Geom::Point anchor;
};

struct ProbeIdle : boost::statechart::simple_state<ProbeIdle, ProbeTool>
{
    ProbeIdle();
    ~ProbeIdle();
    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, ProbeDraging, ProbeTool, &ProbeTool::OnStartDraging>,
        boost::statechart::custom_reaction<EvToolQuit>> reactions;

    sc::result react(const EvToolQuit &e);
};

struct ProbeDraging : boost::statechart::simple_state<ProbeDraging, ProbeTool>
{
    ProbeDraging();
    ~ProbeDraging();

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, ProbeIdle, ProbeTool, &ProbeTool::OnEndDraging>,
        boost::statechart::transition<EvReset, ProbeIdle, ProbeTool, &ProbeTool::OnReset>,
        boost::statechart::in_state_reaction<EvMouseMove, ProbeTool, &ProbeTool::OnDraging>> reactions;
};

#endif //SPAM_UI_TOOLBOX_PROBE_MACHINE_H