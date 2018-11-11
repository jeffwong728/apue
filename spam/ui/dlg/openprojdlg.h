#ifndef SPAM_UI_DLG_OPEN_PROJ_DLG_H
#define SPAM_UI_DLG_OPEN_PROJ_DLG_H
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/filepicker.h>
#include <ui/cmndef.h>

class OpenProjDlg : public wxDialog
{
public:
    OpenProjDlg(wxWindow* parent, const wxString &dbPath, const wxString &selProj);

public:
    bool TransferDataToWindow() wxOVERRIDE;
    bool TransferDataFromWindow() wxOVERRIDE;

private:
    void OnDBChange(wxFileDirPickerEvent &e);

public:
    wxString dbPath_;
    wxString selProj_;
};
#endif //SPAM_UI_DLG_OPEN_PROJ_DLG_H