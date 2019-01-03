#ifndef SPAM_UI_FSM_LINE_TOOL_H
#define SPAM_UI_FSM_LINE_TOOL_H
#include "spamer.h"
#include "notool.h"
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <2geom/path-sink.h>
#pragma warning( pop )

struct LineIdle;
struct LineDraging;
struct LineTracing;

struct LineTool : boost::statechart::simple_state<LineTool, Spamer, LineIdle>
{
    LineTool() {}
    ~LineTool() {}

  void OnStartDraging(const EvLMouseDown &e);
  void OnDraging(const EvMouseMove &e);
  void OnTracing(const EvMouseMove &e);
  void OnEndDraging(const EvLMouseUp &e);
  void OnEndTracing(const EvLMouseDown &e);
  void OnReset(const EvReset &e);
  void EndDraging(const wxMouseEvent &e);
  void EndTracing(const wxMouseEvent &e);
  void OnCanvasEnter(const EvCanvasEnter &e);
  void OnCanvasLeave(const EvCanvasLeave &e);

  typedef boost::mpl::list<
      boost::statechart::transition<EvReset, LineTool>,
      boost::statechart::in_state_reaction<EvCanvasEnter, LineTool, &LineTool::OnCanvasEnter>,
      boost::statechart::in_state_reaction<EvCanvasLeave, LineTool, &LineTool::OnCanvasLeave>> reactions;

  Geom::Point   anchor;
  Geom::OptRect rect;
};

struct LineIdle : boost::statechart::simple_state<LineIdle, LineTool>
{
    LineIdle() {}
    ~LineIdle() {}
    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, LineDraging, LineTool, &LineTool::OnStartDraging>,
        boost::statechart::custom_reaction<EvToolQuit>> reactions;

    sc::result react(const EvToolQuit &e);
};

struct LineDraging : boost::statechart::simple_state<LineDraging, LineTool>
{
    LineDraging() {}
    ~LineDraging() {}

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<EvLMouseUp>,
        boost::statechart::transition<EvReset, LineIdle, LineTool, &LineTool::OnReset>,
        boost::statechart::in_state_reaction<EvMouseMove, LineTool, &LineTool::OnDraging>> reactions;

    sc::result react(const EvLMouseUp &e);
};

struct LineTracing : boost::statechart::simple_state<LineTracing, LineTool>
{
    LineTracing() {}
    ~LineTracing() {}

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, LineIdle, LineTool, &LineTool::OnEndTracing>,
        boost::statechart::transition<EvReset, LineIdle, LineTool, &LineTool::OnReset>,
        boost::statechart::in_state_reaction<EvMouseMove, LineTool, &LineTool::OnTracing>> reactions;
};

#endif //SPAM_UI_FSM_LINE_TOOL_H