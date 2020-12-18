#ifndef _WX_GTK_SWITCH_BUTTON_H_
#define _WX_GTK_SWITCH_BUTTON_H_

#include "wx/defs.h"
#include "wx/event.h"
#include "wx/control.h"

extern WXDLLIMPEXP_DATA_CORE(const char) wxCheckBoxNameStr[];
wxDECLARE_EVENT(wxEVT_SWITCHBUTTON, wxCommandEvent);

class wxSwitchButton: public wxControl
{
public:
    // construction/destruction
    wxSwitchButton() {}
    wxSwitchButton(wxWindow *parent,
                   wxWindowID id,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxValidator& validator = wxDefaultValidator,
                   const wxString& name = wxASCII_STR(wxCheckBoxNameStr))
    {
        Create(parent, id, pos, size, style, validator, name);
    }

    // Create the control
    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize, long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxCheckBoxNameStr));

    // Get/set the value
    void SetValue(bool state);
    bool GetValue() const;

    void GTKDisableEvents();
    void GTKEnableEvents();

protected:
    virtual void DoApplyWidgetStyle(GtkRcStyle *style) wxOVERRIDE;

private:
    typedef wxControl base_type;

    wxDECLARE_DYNAMIC_CLASS(wxSwitchButton);
};

#endif // _WX_GTK_SWITCH_BUTTON_H_

