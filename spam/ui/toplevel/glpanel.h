#ifndef SPAM_UI_TOP_LEVEL_GL_PANEL_H
#define SPAM_UI_TOP_LEVEL_GL_PANEL_H
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <ui/cmndef.h>
#include <stack>
#include <list>
class wxGLAreaWidget;

class GLPanel : public wxPanel
{
public:
    GLPanel(wxWindow* parent);
    ~GLPanel();

public:
    wxGLAreaWidget *GetGLWidget() const { return glCtrl_; }

private:
    void OnClear(wxCommandEvent &cmd);
    void OnSave(wxCommandEvent &cmd);
    void OnContextMenu(wxContextMenuEvent &evt);
    void OnXYZAnglesChanged(wxCommandEvent& evt);

private:
    wxGLAreaWidget *glCtrl_;
};
#endif //SPAM_UI_TOP_LEVEL_GL_PANEL_H