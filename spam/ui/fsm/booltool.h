#ifndef SPAM_UI_FSM_BOOL_TOOL_H
#define SPAM_UI_FSM_BOOL_TOOL_H
#include "spamer.h"
#include "notool.h"
#include "boxtool.h"
#include <ui/projs/geomnode.h>
#include <ui/projs/drawablenode.h>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#include <2geom/pathvector.h>
#pragma warning( pop )

struct UnionTool;
struct UnionToolIdle;
struct UnionToolDraging;
using UnionBoxTool = BoxTool<UnionTool, kSpamID_TOOLBOX_GEOM_UNION>;

struct UnionTool : boost::statechart::simple_state<UnionTool, Spamer, UnionToolIdle>, UnionBoxTool
{
    using BoxToolT = BoxToolImpl;
    UnionTool() : UnionBoxTool(*this) {}
    ~UnionTool() {}
    void OnMMouseDown(const EvMMouseDown &e);

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, UnionTool>,
        boost::statechart::transition<EvToolQuit, NoTool>,
        boost::statechart::in_state_reaction<EvAppQuit, BoxToolT, &BoxToolT::QuitApp>,
        boost::statechart::in_state_reaction<EvDrawableDelete, BoxToolT, &BoxToolT::DeleteDrawable>,
        boost::statechart::in_state_reaction<EvDrawableSelect, BoxToolT, &BoxToolT::SelectDrawable>,
        boost::statechart::in_state_reaction<EvCanvasLeave, BoxToolT, &BoxToolT::LeaveCanvas>> reactions;
};

struct UnionToolIdle : boost::statechart::simple_state<UnionToolIdle, UnionTool>
{
    UnionToolIdle() {}
    ~UnionToolIdle() {}

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, UnionToolDraging, UnionTool::BoxToolT, &UnionTool::BoxToolT::StartBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, UnionTool::BoxToolT, &UnionTool::BoxToolT::Safari>,
        boost::statechart::in_state_reaction<EvMMouseDown, UnionTool, &UnionTool::OnMMouseDown>> reactions;
};

struct UnionToolDraging : boost::statechart::simple_state<UnionToolDraging, UnionTool>
{
    UnionToolDraging() {}
    ~UnionToolDraging() {}

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, UnionToolIdle, UnionTool::BoxToolT, &UnionTool::BoxToolT::EndBoxing>,
        boost::statechart::transition<EvReset, UnionToolIdle, UnionTool::BoxToolT, &UnionTool::BoxToolT::ResetBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, UnionTool::BoxToolT, &UnionTool::BoxToolT::ContinueBoxing>> reactions;
};

struct DiffOperatorDef : public boost::msm::front::state_machine_def<DiffOperatorDef>
{
    struct Wait1stOperand : public boost::msm::front::state<> 
    {
        template <class Event> void on_exit(Event const&, DiffOperatorDef& fsm) {}
    };

    struct Wait2ndOperand : public boost::msm::front::state<>
    {
        template <class Event> void on_exit(Event const&, DiffOperatorDef& fsm) { operand1st->RestoreColor(); }
        SPDrawableNode operand1st;
    };

    struct ReadyGo : public boost::msm::front::state<>
    {
        template <class Event> void on_exit(Event const&, DiffOperatorDef& fsm) { operand1st->RestoreColor(); operand2nd->RestoreColor(); }
        SPDrawableNode operand1st;
        SPDrawableNode operand2nd;
    };

    struct save_1st_operand
    {
        void operator()(evt_entity_selected const& e, DiffOperatorDef&, Wait1stOperand&, Wait2ndOperand& t) 
        { 
            t.operand1st = e.ent;
            t.operand1st->ChangeColorToSelected();
        }
    };

    struct save_2nd_operand
    {
        void operator()(evt_entity_selected const& e, DiffOperatorDef&, Wait2ndOperand &s, ReadyGo& t)
        { 
            t.operand1st = s.operand1st;
            t.operand2nd = e.ent;
            t.operand1st->ChangeColorToSelected();
            t.operand2nd->ChangeColorToSelected();
        }
    };

    struct invalidate_operands
    {
        void operator()(evt_quit_tool const& e, DiffOperatorDef&, Wait2ndOperand &s, Wait2ndOperand& t);
        void operator()(evt_quit_tool const& e, DiffOperatorDef&, ReadyGo &s, ReadyGo& t);
    };

    struct wrap_operand
    {
        void operator()(evt_entity_selected const& e, DiffOperatorDef &dop, ReadyGo &s, ReadyGo& t);
    };

    struct do_diff
    {
        void operator()(const evt_apply &e, DiffOperatorDef&, ReadyGo& s, Wait1stOperand& t);
    };

    struct valid_operand
    {
        bool operator()(evt_entity_selected const& evt, DiffOperatorDef&, Wait1stOperand&, Wait2ndOperand&);
        bool operator()(evt_entity_selected const& evt, DiffOperatorDef&, Wait2ndOperand &s, ReadyGo &t);
        bool operator()(evt_entity_selected const& evt, DiffOperatorDef&, ReadyGo &s, ReadyGo &t);
    };

    struct transition_table : boost::mpl::vector<
        boost::msm::front::Row<Wait1stOperand, evt_entity_selected, Wait2ndOperand, save_1st_operand, valid_operand>,
        boost::msm::front::Row<Wait2ndOperand, evt_entity_selected, ReadyGo, save_2nd_operand, valid_operand>,
        boost::msm::front::Row<Wait2ndOperand, evt_quit_tool, Wait2ndOperand, invalidate_operands, boost::msm::front::none>,
        boost::msm::front::Row<ReadyGo, evt_entity_selected, ReadyGo, wrap_operand, valid_operand>,
        boost::msm::front::Row<ReadyGo, evt_apply, Wait1stOperand, do_diff, boost::msm::front::none>,
        boost::msm::front::Row<ReadyGo, evt_quit_tool, ReadyGo, invalidate_operands, boost::msm::front::none>> {};

    template <class FSM, class Event> void no_transition(Event const& e, FSM&, int state){}
    typedef Wait1stOperand initial_state;
};

typedef boost::msm::back::state_machine<DiffOperatorDef> DiffOperator;

struct DiffTool;
struct DiffToolIdle;
struct DiffToolDraging;
using DiffBoxTool = BoxTool<DiffTool, kSpamID_TOOLBOX_GEOM_DIFF>;

struct DiffTool : boost::statechart::simple_state<DiffTool, Spamer, DiffToolIdle>, DiffBoxTool
{
    using BoxToolT = BoxToolImpl;
    DiffTool();
    ~DiffTool();

    void OnMMouseDown(const EvMMouseDown &e);
    void FireClickEntity(const SPDrawableNode &ent, const wxMouseEvent &e, const Geom::Point &pt, const SelectionData &sd) const override;

    typedef boost::mpl::list<
        boost::statechart::transition<EvReset, DiffTool>,
        boost::statechart::transition<EvToolQuit, NoTool>,
        boost::statechart::in_state_reaction<EvAppQuit, BoxToolT, &BoxToolT::QuitApp>,
        boost::statechart::in_state_reaction<EvDrawableDelete, BoxToolT, &BoxToolT::DeleteDrawable>,
        boost::statechart::in_state_reaction<EvDrawableSelect, BoxToolT, &BoxToolT::SelectDrawable>,
        boost::statechart::in_state_reaction<EvCanvasLeave, BoxToolT, &BoxToolT::LeaveCanvas>> reactions;

    mutable std::map<std::string, DiffOperator> differs;
};

struct DiffToolIdle : boost::statechart::simple_state<DiffToolIdle, DiffTool>
{
    DiffToolIdle() {}
    ~DiffToolIdle() {}

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, DiffToolDraging, DiffTool::BoxToolT, &DiffTool::BoxToolT::StartBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, DiffTool::BoxToolT, &DiffTool::BoxToolT::Safari>,
        boost::statechart::in_state_reaction<EvMMouseDown, DiffTool, &DiffTool::OnMMouseDown>> reactions;
};

struct DiffToolDraging : boost::statechart::simple_state<DiffToolDraging, DiffTool>
{
    DiffToolDraging() {}
    ~DiffToolDraging() {}

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseUp, DiffToolIdle, DiffTool::BoxToolT, &DiffTool::BoxToolT::EndBoxing>,
        boost::statechart::transition<EvReset, DiffToolIdle, DiffTool::BoxToolT, &DiffTool::BoxToolT::ResetBoxing>,
        boost::statechart::in_state_reaction<EvMouseMove, DiffTool::BoxToolT, &DiffTool::BoxToolT::ContinueBoxing>> reactions;
};

#endif //SPAM_UI_FSM_BOOL_TOOL_H