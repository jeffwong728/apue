#ifndef SPAM_UI_FSM_RECT_TOOL_H
#define SPAM_UI_FSM_RECT_TOOL_H
#include "spamer.h"
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#include <2geom/pathvector.h>
#pragma warning( pop )

struct RectIdle;
struct RectDraging;
struct RectTracing;

struct RectTool : boost::statechart::simple_state<RectTool, Spamer, RectIdle>
{
    RectTool();
    ~RectTool();

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
      boost::statechart::transition<EvReset, RectTool>,
      boost::statechart::in_state_reaction<EvCanvasEnter, RectTool, &RectTool::OnCanvasEnter>,
      boost::statechart::in_state_reaction<EvCanvasLeave, RectTool, &RectTool::OnCanvasLeave>> reactions;

  Geom::Point   anchor;
  Geom::OptRect rect;
};

struct RectIdle : boost::statechart::simple_state<RectIdle, RectTool>
{
    RectIdle();
    ~RectIdle();
    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, RectDraging, RectTool, &RectTool::OnStartDraging>,
        boost::statechart::custom_reaction<EvToolQuit>> reactions;

    sc::result react(const EvToolQuit &e);
};

struct RectDraging : boost::statechart::simple_state<RectDraging, RectTool>
{
    RectDraging();
    ~RectDraging();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<EvLMouseUp>,
        boost::statechart::transition<EvReset, RectIdle, RectTool, &RectTool::OnReset>,
        boost::statechart::in_state_reaction<EvMouseMove, RectTool, &RectTool::OnDraging>> reactions;

    sc::result react(const EvLMouseUp &e);
};

struct RectTracing : boost::statechart::simple_state<RectTracing, RectTool>
{
    RectTracing();
    ~RectTracing();

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, RectIdle, RectTool, &RectTool::OnEndTracing>,
        boost::statechart::transition<EvReset, RectIdle, RectTool, &RectTool::OnReset>,
        boost::statechart::in_state_reaction<EvMouseMove, RectTool, &RectTool::OnTracing>> reactions;
};

#endif //SPAM_UI_FSM_RECT_TOOL_H