#ifndef SPAM_UI_TOP_LEVEL_GL_PANEL_H
#define SPAM_UI_TOP_LEVEL_GL_PANEL_H
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <ui/cmndef.h>
#include <stack>
#include <list>

class GLPanel : public wxPanel
{
public:
    GLPanel(wxWindow* parent);
    ~GLPanel();

private:
    void OnSize(wxSizeEvent &e);
    void GLPanel::OnPaint(wxPaintEvent& e);
    void OnClear(wxCommandEvent &cmd);
    void OnSave(wxCommandEvent &cmd);
    void OnContextMenu(wxContextMenuEvent &evt);

private:
    GtkWidget *box_;
};
#endif //SPAM_UI_TOP_LEVEL_GL_PANEL_H