#ifndef SPAM_UI_TOOLBOX_PROC_BOX_H
#define SPAM_UI_TOOLBOX_PROC_BOX_H
#include "toolbox.h"
#include <ui/cmndef.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

class ProcBox : public ToolBox
{
public:
    ProcBox(wxWindow* parent);
    ~ProcBox();

protected:
    wxPanel * GetOptionPanel(const int toolIndex, wxWindow *parent) override;
    ToolOptions GetToolOptions() const override;

private:
    void OnToolEnter(const ToolOptions &toolOpts);

private:
    wxPanel *CreateThresholdOption(wxWindow *parent);
    void UpdateSelectionFilter(void);
};
#endif //SPAM_UI_TOOLBOX_PROC_BOX_H