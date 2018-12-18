#include "probebox.h"
#include <ui/spam.h>
#include <wx/artprov.h>
#include <wx/statline.h>
#include <wx/collpane.h>
#include <wx/tglbtn.h>

ProbeBox::ProbeBox(wxWindow* parent)
: ToolBox(parent, kSpamID_TOOLPAGE_PROBE, wxT("Infomation"), std::vector<wxString>(), kSpamID_TOOLBOX_PROBE_GUARD - kSpamID_TOOLBOX_PROBE_SELECT, kSpamID_TOOLBOX_PROBE_SELECT)
{
    wxWindowID toolIds[] = {
        kSpamID_TOOLBOX_PROBE_SELECT,
        kSpamID_TOOLBOX_PROBE_HISTOGRAM
    };

    wxString   toolTips[] = {
        wxT("Select entities to show infomation"),
        wxT("Select entities to show histogram")
    };

    const SpamIconPurpose ip = kICON_PURPOSE_TOOLBOX;
    wxBitmap toolIcons[] = {
        Spam::GetBitmap(ip, bm_Pointer),
        Spam::GetBitmap(ip, bm_NodeEdit)
    };

    ToolBox::Init(toolIds, toolTips, toolIcons, WXSIZEOF(toolTips), 0, 0);
}

ProbeBox::~ProbeBox()
{
}

wxPanel *ProbeBox::GetOptionPanel(const int toolIndex, wxWindow *parent)
{
    return nullptr;
}

ToolOptions ProbeBox::GetToolOptions() const
{
    ToolOptions to;
    return to;
}