#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "wx/custombgwin.h"
#include <wx/popupwin.h>

class InstructionTip : public wxCustomBackgroundWindow<wxPopupWindow>
{
public:
    InstructionTip(wxWindow* parent, const wxString& message, const wxBitmap& icon);
    InstructionTip(wxWindow* parent, const std::vector<wxString> &messages, const wxBitmap& icon);

private:
    void SetBackgroundColours(wxColour colStart, wxColour colEnd);
    void SetTipShapeAndSize(const wxSize& contentSize);

    wxDECLARE_NO_COPY_CLASS(InstructionTip);
};

class InformationTip : public wxCustomBackgroundWindow<wxPopupTransientWindow>
{
public:
    InformationTip(wxWindow* parent, const std::vector<wxString> &messages, const wxBitmap& icon);

protected:
    void OnDismiss() wxOVERRIDE { delete this; }

    wxDECLARE_NO_COPY_CLASS(InformationTip);
};