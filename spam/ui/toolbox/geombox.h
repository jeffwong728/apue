#ifndef SPAM_UI_TOOLBOX_GEOM_BOX_H
#define SPAM_UI_TOOLBOX_GEOM_BOX_H
#include "toolbox.h"
#include <ui/cmndef.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <tuple>
#include <boost/signals2.hpp>
class wxCollapsiblePaneEvent;
class wxToggleButton;
class wxGenericCollapsiblePane;
namespace bs2 = boost::signals2;

class GeomBox : public ToolBox
{
public:
    GeomBox(wxWindow* parent);
    ~GeomBox();

private:
    void OnNodeEditMode(wxCommandEvent &cmd);
    void OnToolEnter(const ToolOptions &toolOpts);

protected:
    wxPanel * GetOptionPanel(const int toolIndex, wxWindow *parent) override;
    ToolOptions GetToolOptions() const override;
    void UpdateSelectionFilter(void);

private:
    wxPanel *CreateNodeEditOption(wxWindow *parent);
    wxPanel *CreateRectOption(wxWindow *parent);
    int toolEditMode_{ kSpamID_TOOLBOX_NODE_MOVE };
};
#endif //SPAM_UI_TOOLBOX_GEOM_BOX_H
