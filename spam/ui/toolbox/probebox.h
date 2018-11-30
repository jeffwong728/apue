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
};
#endif //SPAM_UI_TOOLBOX_PROBE_BOX_H