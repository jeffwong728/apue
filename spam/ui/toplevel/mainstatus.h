#ifndef SPAM_UI_TOP_LEVEL_MAIN_STATUS_H
#define SPAM_UI_TOP_LEVEL_MAIN_STATUS_H
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <ui/cmndef.h>

class MainStatus : public wxStatusBar
{
    enum 
    {
        Field_Text,
        Field_Checkbox,
        Field_Bitmap,
        Field_NumLockIndicator,
        Field_Clock,
        Field_CapsLockIndicator,
        Field_Max
    };
public:
    MainStatus(wxWindow *parent, long style = wxSTB_DEFAULT_STYLE);
    virtual ~MainStatus();

private:
    void UpdateClock();
    void OnTimer(wxTimerEvent& WXUNUSED(event)) { UpdateClock(); }
    void OnSize(wxSizeEvent& event);
    void OnToggleClock(wxCommandEvent& event);
    void OnIdle(wxIdleEvent& event);
    void DoToggle();

    wxTimer m_timer;
    wxCheckBox *m_checkbox;
    wxStaticBitmap *m_statbmp;
};
#endif //SPAM_UI_TOP_LEVEL_MAIN_STATUS_H