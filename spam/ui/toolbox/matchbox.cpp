#include "matchbox.h"
#include <wx/artprov.h>
#include <wx/statline.h>
#include <wx/collpane.h>
#include <wx/tglbtn.h>

MatchBox::MatchBox(wxWindow* parent)
: ToolBox(parent, kSpamID_TOOLPAGE_MATCH, wxT("Template Matching"), kSpamID_TOOLBOX_MATCH_GUARD - kSpamID_TOOLBOX_MATCH_GRAY, kSpamID_TOOLBOX_MATCH_GRAY)
{
    wxWindowID toolIds[] = {
        kSpamID_TOOLBOX_MATCH_GRAY,
        kSpamID_TOOLBOX_MATCH_SHAPE
    };

    wxString   toolTips[] = {
        wxT("Create gray-scale templates"),
        wxT("Create shape templates")
    };

    wxBitmap toolIcons[] = {
        { wxT("res/pointer.png"), wxBITMAP_TYPE_PNG },
        { wxT("res/node_edit.png"), wxBITMAP_TYPE_PNG }
    };

    ToolBox::Init(toolIds, toolTips, toolIcons);
}

MatchBox::~MatchBox()
{
}

wxPanel *MatchBox::GetOptionPanel(const int toolIndex, wxWindow *parent)
{
    return nullptr;
}