#include "procbox.h"
#include <ui/spam.h>
#include <wx/artprov.h>
#include <wx/statline.h>
#include <wx/collpane.h>
#include <wx/tglbtn.h>
#include <wx/wxhtml.h>

ProcBox::ProcBox(wxWindow* parent)
: ToolBox(parent, kSpamID_TOOLPAGE_PROC, wxT("Process"), std::vector<wxString>(), kSpamID_TOOLBOX_PROC_GUARD - kSpamID_TOOLBOX_PROC_ENHANCEMENT, kSpamID_TOOLBOX_PROC_ENHANCEMENT)
{
    wxWindowID toolIds[] = {
        kSpamID_TOOLBOX_PROC_ENHANCEMENT,
        kSpamID_TOOLBOX_PROC_THRESHOLD,
        kSpamID_TOOLBOX_PROC_FILTER,
        kSpamID_TOOLBOX_PROC_EDGE,
        kSpamID_TOOLBOX_PROC_PYRAMID
    };

    wxString   toolTips[] = {
        wxT("Image enhancement"),
        wxT("Image threshold"),
        wxT("Image filtering"),
        wxT("Image edge extraction"),
        wxT("Construct image pyramid")
    };

    const SpamIconPurpose ip = kICON_PURPOSE_TOOLBOX;
    wxBitmap toolIcons[] = {
        Spam::GetBitmap(ip, bm_Pointer),
        Spam::GetBitmap(ip, bm_NodeEdit)
    };

    ToolBox::Init(toolIds, toolTips, toolIcons, WXSIZEOF(toolTips), 0, 0);
    sig_ToolEnter.connect(std::bind(&ProcBox::OnToolEnter, this, std::placeholders::_1));
}

ProcBox::~ProcBox()
{
}

wxPanel *ProcBox::GetOptionPanel(const int toolIndex, wxWindow *parent)
{
    return nullptr;
}

ToolOptions ProcBox::GetToolOptions() const
{
    ToolOptions tos;
    return tos;
}

void ProcBox::OnToolEnter(const ToolOptions &toolOpts)
{
    const int toolId = boost::get<int>(toolOpts.at(cp_ToolId));
    switch (toolId)
    {
    case kSpamID_TOOLBOX_PROC_ENHANCEMENT:
        break;

    case kSpamID_TOOLBOX_PROC_THRESHOLD:
        break;

    default:
        break;
    }
}

wxPanel *ProcBox::CreateThresholdOption(wxWindow *parent)
{
}

void ProcBox::UpdateSelectionFilter(void)
{
}