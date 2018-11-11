#include "probebox.h"
#include <wx/artprov.h>
#include <wx/statline.h>
#include <wx/collpane.h>
#include <wx/tglbtn.h>

ProbeBox::ProbeBox(wxWindow* parent)
: ToolBox(parent, kSpamID_TOOLPAGE_PROBE, wxT("Infomation"), kSpamID_TOOLBOX_PROBE_GUARD - kSpamID_TOOLBOX_PROBE_SELECT, kSpamID_TOOLBOX_PROBE_SELECT)
{
    wxWindowID toolIds[] = {
        kSpamID_TOOLBOX_PROBE_SELECT,
        kSpamID_TOOLBOX_PROBE_HISTOGRAM
    };

    wxString   toolTips[] = {
        wxT("Select entities to show infomation"),
        wxT("Select entities to show histogram")
    };

    wxBitmap toolIcons[] = {
        { wxT("res/pointer.png"), wxBITMAP_TYPE_PNG },
        { wxT("res/node_edit.png"), wxBITMAP_TYPE_PNG }
    };

    ToolBox::Init(toolIds, toolTips, toolIcons);
}

ProbeBox::~ProbeBox()
{
}

wxPanel *ProbeBox::GetOptionPanel(const int toolIndex, wxWindow *parent)
{
    return nullptr;
}