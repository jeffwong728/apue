#ifndef SPAM_UI_MISC_SCOPED_TIMER_H
#define SPAM_UI_MISC_SCOPED_TIMER_H
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
//#include <tbb/tbb.h>

class ScopedTimer
{
public:
    ScopedTimer(const wxString &label);
    ~ScopedTimer();

private:
    //tbb::tick_count startTime_;
    wxString label_;
};
#endif //SPAM_UI_MISC_SCOPED_TIMER_H