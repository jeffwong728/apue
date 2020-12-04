#ifndef SPAM_UI_TOP_LEVEL_PYTHON_EDITOR_PANEL_H
#define SPAM_UI_TOP_LEVEL_PYTHON_EDITOR_PANEL_H
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <ui/cmndef.h>

class PyEditor : public wxPanel
{
public:
    PyEditor(wxWindow* parent);
    ~PyEditor();

public:
    void LoadPyFile(const wxString &fullPath);
};
#endif //SPAM_UI_TOP_LEVEL_PYTHON_EDITOR_PANEL_H
