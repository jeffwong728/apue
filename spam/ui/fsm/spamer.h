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

struct Spamer : boost::statechart::state_machine<Spamer, NoTool>
{
    Spamer();
    ~Spamer();

    void OnAppQuit();
    void OnToolEnter(const ToolOptions &tos);
    void OnToolQuit(int toolId);
    void OnToolOptionsChanged(const ToolOptions &tos);
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
    bs2::signal_type<void(const SPDrawableNode &), bs2_dummy_mutex>::type sig_EntityGlow;
    bs2::signal_type<void(const SPDrawableNode &), bs2_dummy_mutex>::type sig_EntityDim;
    bs2::signal_type<void(const SPDrawableNodeVector &), bs2_dummy_mutex>::type sig_EntitySel;
    bs2::signal_type<void(const SPDrawableNodeVector &), bs2_dummy_mutex>::type sig_EntityDesel;
};

#endif //SPAM_UI_FSM_SPAMER_H