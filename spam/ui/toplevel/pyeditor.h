#ifndef SPAM_UI_TOP_LEVEL_PYTHON_EDITOR_PANEL_H
#define SPAM_UI_TOP_LEVEL_PYTHON_EDITOR_PANEL_H
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <ui/cmndef.h>
#include <set>

class PyEditor : public wxPanel
{
public:
    PyEditor(wxWindow* parent);
    ~PyEditor();

public:
    void LoadDefaultPyFile();
    void LoadPyFile(const wxString &fullPath);
    void SavePyFile()const;
    void PlayPyFile()const;
    void ApplyStyleChange();

private:
    std::set<std::string> pyImpDirs_;
};
#endif //SPAM_UI_TOP_LEVEL_PYTHON_EDITOR_PANEL_H
