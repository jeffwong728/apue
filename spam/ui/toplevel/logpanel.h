#ifndef SPAM_UI_TOP_LEVEL_LOG_PANEL_H
#define SPAM_UI_TOP_LEVEL_LOG_PANEL_H
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <ui/cmndef.h>
#include <stack>
#include <list>

class LogPanel : public wxPanel
{
public:
    LogPanel(wxWindow* parent);
    ~LogPanel();

private:
    void OnClear(wxCommandEvent &cmd);
    void OnSave(wxCommandEvent &cmd);
    void OnContextMenu(wxContextMenuEvent &evt);
};
#endif //SPAM_UI_TOP_LEVEL_LOG_PANEL_H