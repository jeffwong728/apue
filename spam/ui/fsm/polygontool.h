#ifndef SPAM_UI_FSM_POLYGON_TOOL_H
#define SPAM_UI_FSM_POLYGON_TOOL_H
#include "spamer.h"
#include "notool.h"
#include <ui/projs/modelfwd.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )

struct PolygonIdle;
struct PolygonTracing;

struct PolygonTool : boost::statechart::simple_state<PolygonTool, Spamer, PolygonIdle>
{
    PolygonTool();
    ~PolygonTool();

  void OnStartTracing(const EvLMouseDown &e);
  void OnTracing(const EvMouseMove &e);
  void OnReset(const EvReset &e);
  void EndTracing(const wxMouseEvent &e);
  void OnCanvasEnter(const EvCanvasEnter &e);
  void OnCanvasLeave(const EvCanvasLeave &e);

  void OnAddCorner(const EvLMouseDown &e);
  void OnMMouseDown(const EvMMouseDown &e);
  void OnLMouseDClick(const EvLMouseDClick &e);

  typedef boost::mpl::list<
      boost::statechart::transition<EvReset, PolygonTool>,
      boost::statechart::in_state_reaction<EvCanvasEnter, PolygonTool, &PolygonTool::OnCanvasEnter>,
      boost::statechart::in_state_reaction<EvCanvasLeave, PolygonTool, &PolygonTool::OnCanvasLeave>> reactions;

  SPPolygonNode polygon;
};

struct PolygonIdle : boost::statechart::simple_state<PolygonIdle, PolygonTool>
{
    PolygonIdle();
    ~PolygonIdle();
    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, PolygonTracing, PolygonTool, &PolygonTool::OnStartTracing>,
        boost::statechart::custom_reaction<EvToolQuit>> reactions;

    sc::result react(const EvToolQuit &e);
};

struct PolygonTracing : boost::statechart::simple_state<PolygonTracing, PolygonTool>
{
    PolygonTracing();
    ~PolygonTracing();

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, PolygonIdle, PolygonTool, &PolygonTool::OnReset>,
        boost::statechart::in_state_reaction<EvMouseMove, PolygonTool, &PolygonTool::OnTracing>,
        boost::statechart::in_state_reaction<EvLMouseDown, PolygonTool, &PolygonTool::OnAddCorner>,
        boost::statechart::transition<EvMMouseDown, PolygonIdle, PolygonTool, &PolygonTool::OnMMouseDown>,
        boost::statechart::transition<EvLMouseDClick, PolygonIdle, PolygonTool, &PolygonTool::OnLMouseDClick>> reactions;
};

#endif //SPAM_UI_FSM_POLYGON_TOOL_H