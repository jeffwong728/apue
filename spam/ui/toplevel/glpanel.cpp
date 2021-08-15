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

    auto styleSizer = new wxFlexGridSizer(2, 2, 2);
    styleSizer->AddGrowableCol(1, 1);
    styleSizer->SetFlexibleDirection(wxHORIZONTAL);
    styleSizer->Add(new wxStaticText(this, wxID_ANY, wxT("X axis:")), wxSizerFlags().CentreVertical().Border(wxLEFT));
    styleSizer->Add(new wxSlider(this, kSpamID_GL_X_AXIS_ANGLE_SLIDER, 0, 0, 360, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_VALUE_LABEL), wxSizerFlags(1).Expand());
    styleSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Y axis:")), wxSizerFlags().CentreVertical().Border(wxLEFT));
    styleSizer->Add(new wxSlider(this, kSpamID_GL_Y_AXIS_ANGLE_SLIDER, 0, 0, 360, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_VALUE_LABEL), wxSizerFlags(1).Expand());
    styleSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Z axis:")), wxSizerFlags().CentreVertical().Border(wxLEFT));
    styleSizer->Add(new wxSlider(this, kSpamID_GL_Z_AXIS_ANGLE_SLIDER, 0, 0, 360, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_VALUE_LABEL), wxSizerFlags(1).Expand());
    sizerRoot->Add(styleSizer, wxSizerFlags().Expand());

    Bind(wxEVT_SIZE, &GLPanel::OnSize, this, wxID_ANY);
    Bind(wxEVT_SLIDER, &GLPanel::OnXYZAnglesChanged, this, kSpamID_GL_X_AXIS_ANGLE_SLIDER, kSpamID_GL_Z_AXIS_ANGLE_SLIDER);

    // Set sizer for the panel
    SetSizer(sizerRoot);
    GetSizer()->SetSizeHints(this);
    Show();
}

GLPanel::~GLPanel()
{
}

void GLPanel::OnSize(wxSizeEvent &e)
{
    e.Skip();
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

