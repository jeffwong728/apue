#ifndef SPAM_UI_TOP_LEVEL_PREFERENCES_DIALOG_H
#define SPAM_UI_TOP_LEVEL_PREFERENCES_DIALOG_H
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <ui/cmndef.h>
class wxBookCtrlEvent;

class PreferencesDlg : public wxDialog
{
public:
    PreferencesDlg(wxWindow* parent);
    ~PreferencesDlg();

private:
    void OnTreeBook(wxBookCtrlEvent& event);
};
#endif //SPAM_UI_TOP_LEVEL_PREFERENCES_DIALOG_H
