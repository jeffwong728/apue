#ifndef SPAM_UI_FSM_PROFILE_TOOL_H
#define SPAM_UI_FSM_PROFILE_TOOL_H
#include "spamer.h"
#include "notool.h"
#include "boxtool.h"
#include <set>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )

struct ProfileIdle;
struct ProfileDraging;
struct ProfileTracing;

struct ProfileTool : boost::statechart::simple_state<ProfileTool, Spamer, ProfileIdle>
{
    ProfileTool() {}
    ~ProfileTool() {}

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
        boost::statechart::transition<EvReset, ProfileTool>,
        boost::statechart::in_state_reaction<EvCanvasEnter, ProfileTool, &ProfileTool::OnCanvasEnter>,
        boost::statechart::in_state_reaction<EvCanvasLeave, ProfileTool, &ProfileTool::OnCanvasLeave>> reactions;

    Geom::Point anchor;
    std::set<std::string> uuids;
};

struct ProfileIdle : boost::statechart::simple_state<ProfileIdle, ProfileTool>
{
    ProfileIdle() {}
    ~ProfileIdle() {}
    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, ProfileDraging, ProfileTool, &ProfileTool::OnStartDraging>,
        boost::statechart::custom_reaction<EvToolQuit>> reactions;

    sc::result react(const EvToolQuit &e);
};

struct ProfileDraging : boost::statechart::simple_state<ProfileDraging, ProfileTool>
{
    ProfileDraging() {}
    ~ProfileDraging() {}

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<EvLMouseUp>,
        boost::statechart::transition<EvReset, ProfileIdle, ProfileTool, &ProfileTool::OnReset>,
        boost::statechart::in_state_reaction<EvMouseMove, ProfileTool, &ProfileTool::OnDraging>> reactions;

    sc::result react(const EvLMouseUp &e);
};

struct ProfileTracing : boost::statechart::simple_state<ProfileTracing, ProfileTool>
{
    ProfileTracing() {}
    ~ProfileTracing() {}

    typedef boost::mpl::list<
        boost::statechart::transition<EvLMouseDown, ProfileIdle, ProfileTool, &ProfileTool::OnEndTracing>,
        boost::statechart::transition<EvReset, ProfileIdle, ProfileTool, &ProfileTool::OnReset>,
        boost::statechart::in_state_reaction<EvMouseMove, ProfileTool, &ProfileTool::OnTracing>> reactions;
};

#endif //SPAM_UI_FSM_PROFILE_TOOL_H
