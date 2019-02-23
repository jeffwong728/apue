#include "logpanel.h"
#include <wx/artprov.h>
#include <wx/wrapsizer.h>
#include <boost/container/flat_map.hpp>

LogPanel::LogPanel(wxWindow* parent)
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
    // Root sizer, vertical
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);
    sizerRoot->Add(MakeToolBar(), wxSizerFlags().Expand())->SetId(kSpamLogToolBar);

    auto logWindow = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE | wxTE_DONTWRAP);
    sizerRoot->Add(logWindow, wxSizerFlags(1).Expand())->SetId(kSpamLogTextCtrl);

    // Set sizer for the panel
    SetSizer(sizerRoot);
    GetSizer()->SetSizeHints(this);
    Show();

    delete wxLog::SetActiveTarget(new wxLogTextCtrl(logWindow));
}

LogPanel::~LogPanel()
{
}

wxToolBar *LogPanel::MakeToolBar()
{
    wxToolBar *tb = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
    tb->SetToolBitmapSize(wxSize(16, 16));

    tb->AddTool(kSpamID_LOG_CLEAR, wxT("Clear"),    wxArtProvider::GetBitmap(wxART_DELETE, wxART_TOOLBAR, wxSize(16, 16)));
    tb->AddTool(kSpamID_LOG_SAVE, wxT("Save"), wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR, wxSize(16, 16)));
    tb->Realize();

    tb->Bind(wxEVT_TOOL, &LogPanel::OnClear, this, kSpamID_LOG_CLEAR);
    tb->Bind(wxEVT_TOOL, &LogPanel::OnSave, this, kSpamID_LOG_SAVE);

    return tb;
}

void LogPanel::OnClear(wxCommandEvent &cmd)
{
    auto logTextCtrl = dynamic_cast<wxTextCtrl *>(GetSizer()->GetItemById(kSpamLogTextCtrl)->GetWindow());
    if (logTextCtrl)
    {
        logTextCtrl->Clear();
    }
}

void LogPanel::OnSave(wxCommandEvent &cmd)
{
    boost::container::flat_map<int, int> fm;
}