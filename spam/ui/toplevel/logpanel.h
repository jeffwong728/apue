#ifndef SPAM_UI_TOP_LEVEL_LOG_PANEL_H
#define SPAM_UI_TOP_LEVEL_LOG_PANEL_H
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <ui/cmndef.h>
#include <stack>
#include <list>

class ConsolePanel : public wxPanel
{
public:
    ConsolePanel(wxWindow* parent);
    ~ConsolePanel();

private:
    wxToolBar *MakeToolBar();

private:
    void OnClear(wxCommandEvent &cmd);
    void OnSave(wxCommandEvent &cmd);
    void OnText(wxCommandEvent& evt);
    void OnAction(wxCommandEvent& evt);
    void OnUpdateUI(wxUpdateUIEvent& evt);
    void OnEnter(wxCommandEvent& evt);
    void OnKey(wxKeyEvent &evt);

private:
    std::list<wxString> cmds_;
    std::list<wxString>::const_iterator cursor_;
};
#endif //SPAM_UI_TOP_LEVEL_LOG_PANEL_H