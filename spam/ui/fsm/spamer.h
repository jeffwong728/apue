#ifndef SPAM_UI_FSM_SPAMER_H
#define SPAM_UI_FSM_SPAMER_H
#include "events.h"
#include <ui/cmndef.h>
#include <ui/projs/modelfwd.h>
#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/in_state_reaction.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )
#include <boost/signals2.hpp>
namespace bs2 = boost::signals2;
namespace sc = boost::statechart;

class RootFrame;
struct NoTool;
struct NoToolIdle;
struct NoToolDraging;

struct Spamer : boost::statechart::state_machine<Spamer, NoTool>
{
    Spamer();
    ~Spamer();

    void OnAppQuit();
    void OnToolEnter(int toolId);
    void OnToolQuit(int toolId);
    void OnCanvasEnter(wxMouseEvent &e);
    void OnCanvasLeave(wxMouseEvent &e);
    void OnCanvasLeftMouseDown(wxMouseEvent &e);
    void OnCanvasLeftMouseUp(wxMouseEvent &e);
    void OnCanvasMouseMotion(wxMouseEvent &e);
    void OnCanvasLeftDClick(wxMouseEvent &e);
    void OnCanvasMiddleDown(wxMouseEvent &e);
    void OnCanvasKeyDown(wxKeyEvent &e);
    void OnCanvasKeyUp(wxKeyEvent &e);
    void OnCanvasChar(wxKeyEvent &e);
    void OnGeomDelete(const SPModelNodeVector &geoms);
    void OnDrawableSelect(const SPDrawableNodeVector &dras);

    typedef bs2::keywords::mutex_type<bs2::dummy_mutex> bs2_dummy_mutex;
    bs2::signal_type<void(const SPModelNode &), bs2_dummy_mutex>::type sig_EntityGlow;
    bs2::signal_type<void(const SPModelNode &), bs2_dummy_mutex>::type sig_EntityDim;
    bs2::signal_type<void(const SPDrawableNodeVector &), bs2_dummy_mutex>::type sig_EntitySel;
    bs2::signal_type<void(const SPDrawableNodeVector &), bs2_dummy_mutex>::type sig_EntityDesel;
};

struct NoTool : boost::statechart::simple_state<NoTool, Spamer, NoToolIdle>
{
    NoTool();
    ~NoTool();

    void OnStartDraging(const EvLMouseDown &e);
    void OnDraging(const EvMouseMove &e);
    void OnEndDraging(const EvLMouseUp &e);
    void OnReset(const EvReset &e);
    void OnSafari(const EvMouseMove &e);
    void OnCanvasLeave(const EvCanvasLeave &e);
    void OnAppQuit(const EvAppQuit &e);
    void OnDrawableDelete(const EvDrawableDelete &e);
    void OnDrawableSelect(const EvDrawableSelect &e);

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, NoTool>,
        boost::statechart::in_state_reaction<EvAppQuit, NoTool, &NoTool::OnAppQuit>,
        boost::statechart::in_state_reaction<EvDrawableDelete, NoTool, &NoTool::OnDrawableDelete>,
        boost::statechart::in_state_reaction<EvDrawableSelect, NoTool, &NoTool::OnDrawableSelect>,
        boost::statechart::in_state_reaction<EvCanvasLeave, NoTool, &NoTool::OnCanvasLeave>> reactions;

    Geom::Point anchor;
    Geom::OptRect rect;
    SPDrawableNode highlight_;
    std::unordered_map<std::string, SPDrawableNodeVector> selData;
};

struct NoToolIdle : boost::statechart::simple_state<NoToolIdle, NoTool>
{
    NoToolIdle();
    ~NoToolIdle();

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, NoToolDraging, NoTool, &NoTool::OnStartDraging>,
        boost::statechart::in_state_reaction<EvMouseMove, NoTool, &NoTool::OnSafari>,
        boost::statechart::custom_reaction<EvToolEnter>> reactions;

    sc::result react(const EvToolEnter &e);
};

struct NoToolDraging : boost::statechart::simple_state<NoToolDraging, NoTool>
{
    NoToolDraging();
    ~NoToolDraging();

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, NoToolIdle, NoTool, &NoTool::OnEndDraging>,
        boost::statechart::transition<EvReset, NoToolIdle, NoTool, &NoTool::OnReset>,
        boost::statechart::in_state_reaction<EvMouseMove, NoTool, &NoTool::OnDraging>> reactions;
};

#endif //SPAM_UI_FSM_SPAMER_H