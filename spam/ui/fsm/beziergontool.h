#ifndef SPAM_UI_FSM_BEZIERGON_TOOL_H
#define SPAM_UI_FSM_BEZIERGON_TOOL_H
#include "spamer.h"
#include "notool.h"
#include <ui/projs/modelfwd.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )

struct BeziergonIdle;
struct BeziergonTracing;
struct BeziergonDraging;

struct BeziergonTool : boost::statechart::simple_state<BeziergonTool, Spamer, BeziergonIdle>
{
    BeziergonTool();
    ~BeziergonTool();

  void OnTracing(const EvMouseMove &e);
  void OnReset(const EvReset &e);
  void CompleteCreate(const wxMouseEvent &e);
  void OnCanvasEnter(const EvCanvasEnter &e);
  void OnCanvasLeave(const EvCanvasLeave &e);

  void OnInitDraging(const EvLMouseDown &e);
  void OnStartDraging(const EvLMouseDown &e);
  void OnDraging(const EvMouseMove &e);
  void OnEndDraging(const EvLMouseUp &e);

  void OnAddCorner(const EvLMouseDown &e);
  void OnMMouseDown(const EvMMouseDown &e);
  void OnLMouseDClick(const EvLMouseDClick &e);

  typedef boost::mpl::list<
      boost::statechart::transition<EvReset, BeziergonTool>,
      boost::statechart::in_state_reaction<EvCanvasEnter, BeziergonTool, &BeziergonTool::OnCanvasEnter>,
      boost::statechart::in_state_reaction<EvCanvasLeave, BeziergonTool, &BeziergonTool::OnCanvasLeave>> reactions;

  SPBeziergonNode beziergon;
};

struct BeziergonIdle : boost::statechart::simple_state<BeziergonIdle, BeziergonTool>
{
    BeziergonIdle();
    ~BeziergonIdle();
    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, BeziergonDraging, BeziergonTool, &BeziergonTool::OnInitDraging>,
        boost::statechart::custom_reaction<EvToolQuit>> reactions;

    sc::result react(const EvToolQuit &e);
};

struct BeziergonTracing : boost::statechart::simple_state<BeziergonTracing, BeziergonTool>
{
    BeziergonTracing();
    ~BeziergonTracing();

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, BeziergonIdle, BeziergonTool, &BeziergonTool::OnReset>,
        boost::statechart::in_state_reaction<EvMouseMove, BeziergonTool, &BeziergonTool::OnTracing>,
        boost::statechart::transition<EvLMouseDown, BeziergonDraging, BeziergonTool, &BeziergonTool::OnStartDraging>,
        boost::statechart::transition<EvMMouseDown, BeziergonIdle, BeziergonTool, &BeziergonTool::OnMMouseDown>,
        boost::statechart::transition<EvLMouseDClick, BeziergonIdle, BeziergonTool, &BeziergonTool::OnLMouseDClick>> reactions;
};

struct BeziergonDraging : boost::statechart::simple_state<BeziergonDraging, BeziergonTool>
{
    BeziergonDraging();
    ~BeziergonDraging();

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, BeziergonIdle, BeziergonTool, &BeziergonTool::OnReset>,
        boost::statechart::in_state_reaction<EvMouseMove, BeziergonTool, &BeziergonTool::OnDraging>,
        boost::statechart::transition<EvLMouseUp, BeziergonTracing, BeziergonTool, &BeziergonTool::OnEndDraging>> reactions;
};

#endif //SPAM_UI_FSM_BEZIERGON_TOOL_H