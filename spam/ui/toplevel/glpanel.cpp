#include "glpanel.h"
#include <ui/spam.h>
#include <wx/artprov.h>
#include <wx/wrapsizer.h>
#include <ui/misc/gtkglareawidget.h>

GLPanel::GLPanel(wxWindow* parent)
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
, glCtrl_(nullptr)
{
    // Root sizer, vertical
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);
    glCtrl_ = new wxGLAreaWidget(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    sizerRoot->Add(glCtrl_, wxSizerFlags(1).Expand());

    // Set sizer for the panel
    SetSizer(sizerRoot);
    GetSizer()->SetSizeHints(this);
    Show();
}

GLPanel::~GLPanel()
{
}

void GLPanel::OnClear(wxCommandEvent &cmd)
{
    auto consoleCtrl = dynamic_cast<wxTextCtrl *>(GetSizer()->GetItemById(kSpamLogTextCtrl)->GetWindow());
    if (consoleCtrl)
    {
        consoleCtrl->Clear();
    }
}

void GLPanel::OnSave(wxCommandEvent &cmd)
{
}

void GLPanel::OnContextMenu(wxContextMenuEvent &evt)
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

void GLPanel::OnXYZAnglesChanged(wxCommandEvent& evt)
{
    glCtrl_->SetAxisAngleValue(evt.GetId() - kSpamID_GL_X_AXIS_ANGLE_SLIDER, evt.GetInt());
}

