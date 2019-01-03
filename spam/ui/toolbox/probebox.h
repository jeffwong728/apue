#ifndef SPAM_UI_TOOLBOX_PROBE_BOX_H
#define SPAM_UI_TOOLBOX_PROBE_BOX_H
#include "toolbox.h"
#include <ui/cmndef.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

class ProbeBox : public ToolBox
{
public:
    ProbeBox(wxWindow* parent);
    ~ProbeBox();

protected:
    wxPanel * GetOptionPanel(const int toolIndex, wxWindow *parent) override;
    ToolOptions GetToolOptions() const override;

private:
    void OnProbeMode(wxCommandEvent &cmd);
    void OnToolEnter(const ToolOptions &toolOpts);

private:
    wxPanel *CreateSelectOption(wxWindow *parent);
    wxPanel *CreateHistOption(wxWindow *parent);
    void UpdateSelectionFilter(void);

private:
    int probeMode_{ kSpamID_TOOLBOX_PROBE_PIXEL };
};
#endif //SPAM_UI_TOOLBOX_PROBE_BOX_H