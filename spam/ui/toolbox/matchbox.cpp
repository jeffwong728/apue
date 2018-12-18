#include "matchbox.h"
#include <ui/spam.h>
#include <wx/artprov.h>
#include <wx/statline.h>
#include <wx/collpane.h>
#include <wx/tglbtn.h>

MatchBox::MatchBox(wxWindow* parent)
: ToolBox(parent, kSpamID_TOOLPAGE_MATCH, wxT("Template Matching"), std::vector<wxString>(), kSpamID_TOOLBOX_MATCH_GUARD - kSpamID_TOOLBOX_MATCH_GRAY, kSpamID_TOOLBOX_MATCH_GRAY)
{
    wxWindowID toolIds[] = {
        kSpamID_TOOLBOX_MATCH_GRAY,
        kSpamID_TOOLBOX_MATCH_SHAPE
    };

    wxString   toolTips[] = {
        wxT("Create gray-scale templates"),
        wxT("Create shape templates")
    };

    const SpamIconPurpose ip = kICON_PURPOSE_TOOLBOX;
    wxBitmap toolIcons[] = {
        Spam::GetBitmap(ip, bm_Pointer),
        Spam::GetBitmap(ip, bm_NodeEdit)
    };

    ToolBox::Init(toolIds, toolTips, toolIcons, WXSIZEOF(toolTips), 0, 0);
}

MatchBox::~MatchBox()
{
}

wxPanel *MatchBox::GetOptionPanel(const int toolIndex, wxWindow *parent)
{
    return nullptr;
}

ToolOptions MatchBox::GetToolOptions() const
{
    ToolOptions to;
    return to;
}