#include "preferencesdlg.h"
#include <wx/artprov.h>
#include <wx/wrapsizer.h>
#include <wx/treebook.h>

class P3EditorGeneralPage : public wxPanel
{
public:
    P3EditorGeneralPage(wxWindow* parent)
        : wxPanel(parent)
    {

    }
    ~P3EditorGeneralPage() {}
};

class P3EditorStylePage : public wxPanel
{
public:
    P3EditorStylePage(wxWindow* parent)
        : wxPanel(parent)
    {

    }
    ~P3EditorStylePage() {}
};

PreferencesDlg::PreferencesDlg(wxWindow* parent)
: wxDialog(parent, wxID_ANY, wxT("Preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    wxTreebook *treeBook = new wxTreebook(this, wxID_ANY, wxDefaultPosition, wxSize(600, 400));
    treeBook->AddPage(nullptr, wxT("Python 3"), false, -1);
    treeBook->AddSubPage(new P3EditorGeneralPage(treeBook), wxT("General"), true, -1);
    treeBook->AddSubPage(new P3EditorStylePage(treeBook), wxT("Editor Style"), false, -1);

    sizerRoot->Add(treeBook, wxSizerFlags(1).Expand())->SetId(kSpamPreferencesTreeBookCtrl);
    sizerRoot->AddSpacer(3);
    sizerRoot->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), wxSizerFlags().Expand().Border());

    treeBook->Bind(wxEVT_TREEBOOK_PAGE_CHANGED, &PreferencesDlg::OnTreeBook, this, wxID_ANY);

    SetSizer(sizerRoot);
    GetSizer()->SetSizeHints(this);
}

PreferencesDlg::~PreferencesDlg()
{
}

void PreferencesDlg::OnTreeBook(wxBookCtrlEvent& evt)
{
    wxTreebook *treeBook = dynamic_cast<wxTreebook *>(evt.GetEventObject());
    if (treeBook) 
    {
        treeBook->GetSelection();
    }
}
