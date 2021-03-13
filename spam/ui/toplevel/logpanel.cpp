#include "logpanel.h"
#include <ui/spam.h>
#include <wx/artprov.h>
#include <wx/wrapsizer.h>

LogPanel::LogPanel(wxWindow* parent)
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
    // Root sizer, vertical
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);
    auto textCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE);
    sizerRoot->Add(textCtrl, wxSizerFlags(1).Expand())->SetId(kSpamLogTextCtrl);

    textCtrl->Bind(wxEVT_MENU, &LogPanel::OnClear, this, wxID_CLEAR);
    textCtrl->Bind(wxEVT_MENU, &LogPanel::OnSave, this, wxID_SAVE);
    textCtrl->Bind(wxEVT_CONTEXT_MENU, &LogPanel::OnContextMenu, this);

    // Set sizer for the panel
    SetSizer(sizerRoot);
    GetSizer()->SetSizeHints(this);
    Show();

    delete wxLog::SetActiveTarget(new wxLogTextCtrl(textCtrl));
}

LogPanel::~LogPanel()
{
}

void LogPanel::OnClear(wxCommandEvent &cmd)
{
    auto consoleCtrl = dynamic_cast<wxTextCtrl *>(GetSizer()->GetItemById(kSpamLogTextCtrl)->GetWindow());
    if (consoleCtrl)
    {
        consoleCtrl->Clear();
    }
}

void LogPanel::OnSave(wxCommandEvent &cmd)
{
}

void LogPanel::OnContextMenu(wxContextMenuEvent &evt)
{
    auto consoleCtrl = dynamic_cast<wxTextCtrl *>(GetSizer()->GetItemById(kSpamLogTextCtrl)->GetWindow());
    if (consoleCtrl)
    {
        wxMenu menu;
        menu.Append(wxID_SAVE, wxT("&Save"));
        menu.Append(wxID_CLEAR, wxT("Cl&ear"));
        consoleCtrl->PopupMenu(&menu);
    }
}
