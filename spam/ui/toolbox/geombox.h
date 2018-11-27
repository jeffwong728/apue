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

protected:
    wxPanel * GetOptionPanel(const int toolIndex, wxWindow *parent);

private:
    wxPanel *CreateNodeEditOption(wxWindow *parent);
    wxPanel *CreateRectOption(wxWindow *parent);
};
#endif //SPAM_UI_TOOLBOX_GEOM_BOX_H