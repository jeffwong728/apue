#ifndef SPAM_UI_FSM_ELLIPSE_TOOL_H
#define SPAM_UI_FSM_ELLIPSE_TOOL_H
#include "spamer.h"
#include "notool.h"
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#include <2geom/pathvector.h>
#pragma warning( pop )

struct EllipseIdle;
struct EllipseDraging;
struct EllipseTracing;

struct EllipseTool : boost::statechart::simple_state<EllipseTool, Spamer, EllipseIdle>
{
    EllipseTool();
    ~EllipseTool();

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
      boost::statechart::transition<EvReset, EllipseTool>,
      boost::statechart::in_state_reaction<EvCanvasEnter, EllipseTool, &EllipseTool::OnCanvasEnter>,
      boost::statechart::in_state_reaction<EvCanvasLeave, EllipseTool, &EllipseTool::OnCanvasLeave>> reactions;

  Geom::Point   anchor;
  Geom::OptRect rect;
};

struct EllipseIdle : boost::statechart::simple_state<EllipseIdle, EllipseTool>
{
    EllipseIdle();
    ~EllipseIdle();
    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, EllipseDraging, EllipseTool, &EllipseTool::OnStartDraging>,
        boost::statechart::custom_reaction<EvToolQuit>> reactions;

    sc::result react(const EvToolQuit &e);
};

struct EllipseDraging : boost::statechart::simple_state<EllipseDraging, EllipseTool>
{
    EllipseDraging();
    ~EllipseDraging();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<EvLMouseUp>,
        boost::statechart::transition<EvReset, EllipseIdle, EllipseTool, &EllipseTool::OnReset>,
        boost::statechart::in_state_reaction<EvMouseMove, EllipseTool, &EllipseTool::OnDraging>> reactions;

    sc::result react(const EvLMouseUp &e);
};

struct EllipseTracing : boost::statechart::simple_state<EllipseTracing, EllipseTool>
{
    EllipseTracing();
    ~EllipseTracing();

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, EllipseIdle, EllipseTool, &EllipseTool::OnEndTracing>,
        boost::statechart::transition<EvReset, EllipseIdle, EllipseTool, &EllipseTool::OnReset>,
        boost::statechart::in_state_reaction<EvMouseMove, EllipseTool, &EllipseTool::OnTracing>> reactions;
};

#endif //SPAM_UI_FSM_ELLIPSE_TOOL_H