#ifndef SPAM_UI_TOOLBOX_MATCH_BOX_H
#define SPAM_UI_TOOLBOX_MATCH_BOX_H
#include "toolbox.h"
#include <ui/cmndef.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

class MatchBox : public ToolBox
{
public:
    MatchBox(wxWindow* parent);
    ~MatchBox();

protected:
    wxPanel * GetOptionPanel(const int toolIndex, wxWindow *parent) override;
    ToolOptions GetToolOptions() const override;
};
#endif //SPAM_UI_TOOLBOX_MATCH_BOX_H