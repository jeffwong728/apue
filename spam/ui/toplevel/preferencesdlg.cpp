#include "preferencesdlg.h"
#include <wx/artprov.h>
#include <wx/wrapsizer.h>
#include <wx/treebook.h>
#include <wx/statline.h>
#include <wx/clrpicker.h>
#include <wx/fontpicker.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <ui/misc/switchbtn.h>
#include <gmodule.h>
#include <gtk/gtk.h>

class ThemePage : public wxPanel
{
public:
    ThemePage(wxWindow* parent)
        : wxPanel(parent)
    {
        wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);
        wxSizer * const sizerDark = new wxBoxSizer(wxHORIZONTAL);
        wxSwitchButton *pSWC = new wxSwitchButton(this, wxID_ANY);
        sizerDark->Add(new wxStaticText(this, wxID_ANY, wxT("Prefer Dark Theme")), wxSizerFlags().CentreVertical().DoubleBorder(wxTOP | wxBOTTOM | wxLEFT | wxRIGHT));
        sizerDark->Add(pSWC, wxSizerFlags().DoubleBorder(wxTOP | wxBOTTOM | wxLEFT | wxRIGHT));
        sizerRoot->Add(sizerDark, wxSizerFlags(0));

        pSWC->Bind(wxEVT_SWITCHBUTTON, &ThemePage::OnSwitchDarkTheme, this);

        SetSizer(sizerRoot);
        GetSizer()->SetSizeHints(this);
    }
    ~ThemePage() {}

private:
    void OnSwitchDarkTheme(wxCommandEvent &cmd)
    {
        GtkSettings *settings = gtk_settings_get_default();
        if (cmd.GetInt())
        {
            g_object_set(G_OBJECT(settings), "gtk-application-prefer-dark-theme", 1, NULL);
        }
        else
        {
            g_object_set(G_OBJECT(settings), "gtk-application-prefer-dark-theme", 0, NULL);
        }
    }
};

class P3EditorGeneralPage : public wxPanel
{
public:
    P3EditorGeneralPage(wxWindow* parent)
        : wxPanel(parent)
    {
        wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);
        sizerRoot->Add(new wxCheckBox(this, wxID_ANY, wxT("View Lines Number")), wxSizerFlags().DoubleBorder(wxTOP | wxLEFT | wxRIGHT));
        sizerRoot->Add(new wxCheckBox(this, wxID_ANY, wxT("View White Space")), wxSizerFlags().DoubleBorder(wxLEFT | wxRIGHT));
        sizerRoot->Add(new wxCheckBox(this, wxID_ANY, wxT("View Indentation Guides")), wxSizerFlags().DoubleBorder(wxBOTTOM | wxLEFT | wxRIGHT));

        SetSizer(sizerRoot);
        GetSizer()->SetSizeHints(this);
    }
    ~P3EditorGeneralPage() {}
};

class P3EditorStylePage : public wxPanel
{
public:
    P3EditorStylePage(wxWindow* parent)
        : wxPanel(parent)
    {
        const wxPoint DP = wxDefaultPosition;
        const wxSize DS = wxDefaultSize;
        wxSizer * const sizerRoot = new wxBoxSizer(wxHORIZONTAL);
        wxListBox * catListBox = new wxListBox(this, wxID_ANY);
        catListBox->Append(wxString(wxT("Default Style")));
        catListBox->Append(wxString(wxT("Number")));
        catListBox->Append(wxString(wxT("String")));
        catListBox->Append(wxString(wxT("Class Name")));
        catListBox->Append(wxString(wxT("Def Name")));
        catListBox->Append(wxString(wxT("Operator")));
        catListBox->Append(wxString(wxT("Identifier")));
        catListBox->Append(wxString(wxT("Line Number")));
        catListBox->Append(wxString(wxT("Character")));
        catListBox->Append(wxString(wxT("Comment Line")));
        catListBox->Append(wxString(wxT("Comment Block")));
        sizerRoot->Add(catListBox, wxSizerFlags(0).DoubleBorder().Expand());

        wxBoxSizer * const sizerSetting = new wxBoxSizer(wxVERTICAL);
        wxFlexGridSizer * const sizerTopSetting = new wxFlexGridSizer(2, 2, 2);
        sizerTopSetting->AddGrowableCol(1, 1);
        sizerTopSetting->SetFlexibleDirection(wxHORIZONTAL);
        sizerTopSetting->Add(new wxStaticText(this, wxID_ANY, wxT("Foreground Color")), wxSizerFlags().Right().DoubleBorder());
        sizerTopSetting->Add(new wxColourPickerCtrl(this, wxID_ANY), wxSizerFlags(1).Expand().HorzBorder());
        sizerTopSetting->Add(new wxStaticText(this, wxID_ANY, wxT("Background Color")), wxSizerFlags().Right().DoubleBorder());
        sizerTopSetting->Add(new wxColourPickerCtrl(this, wxID_ANY), wxSizerFlags(1).Expand().HorzBorder());
        sizerTopSetting->Add(new wxStaticText(this, wxID_ANY, wxT("Font")), wxSizerFlags().Right().DoubleBorder());
        sizerTopSetting->Add(new wxFontPickerCtrl(this, wxID_ANY, *wxSWISS_FONT, DP, DS, wxFNTP_FONTDESC_AS_LABEL), wxSizerFlags(1).Expand().HorzBorder())->SetId(kPy3EditorStyleFontCtrl);
        sizerSetting->Add(sizerTopSetting, wxSizerFlags(0).DoubleBorder().Expand());

        wxBoxSizer * const sizerBotSetting = new wxBoxSizer(wxVERTICAL);
        sizerBotSetting->Add(new wxCheckBox(this, wxID_ANY, wxT("Bold")), wxSizerFlags().DoubleBorder(wxLEFT | wxRIGHT));
        sizerBotSetting->Add(new wxCheckBox(this, wxID_ANY, wxT("Italic")), wxSizerFlags().DoubleBorder(wxLEFT | wxRIGHT));
        sizerBotSetting->Add(new wxCheckBox(this, wxID_ANY, wxT("Underline")), wxSizerFlags().DoubleBorder(wxLEFT | wxRIGHT));
        sizerBotSetting->Add(new wxStaticText(this, wxID_ANY, wxT("Sample")), wxSizerFlags().DoubleBorder(wxTOP | wxLEFT | wxRIGHT));
        sizerBotSetting->AddSpacer(5);
        sizerBotSetting->Add(new wxTextCtrl(this, wxID_ANY, wxT(""), DP, DS, wxTE_MULTILINE | wxTE_READONLY), wxSizerFlags(1).DoubleBorder(wxLEFT | wxRIGHT).Expand())->SetId(kPy3EditorStyleSampleCtrl);
        sizerSetting->Add(sizerBotSetting, wxSizerFlags(1).DoubleBorder().Expand());

        sizerRoot->Add(sizerSetting, wxSizerFlags(1).DoubleBorder().Expand());

        auto sampleTextCtrl = dynamic_cast<wxTextCtrl *>(sizerBotSetting->GetItemById(kPy3EditorStyleSampleCtrl)->GetWindow());
        sampleTextCtrl->AppendText(wxT("0123456789\n"));
        sampleTextCtrl->AppendText(wxT("abcdefghijklmnopqrstuvwxyz\n"));
        sampleTextCtrl->AppendText(wxT("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"));
        sampleTextCtrl->AppendText(wxT("!@#$%^&*()_+{}|[]\\:\";'<>?,./`~"));

        SetSizer(sizerRoot);
        GetSizer()->SetSizeHints(this);
    }
    ~P3EditorStylePage() {}
};

PreferencesDlg::PreferencesDlg(wxWindow* parent)
: wxDialog(parent, wxID_ANY, wxT("Preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    wxTreebook *treeBook = new wxTreebook(this, wxID_ANY, wxDefaultPosition, wxSize(640, 480));
    treeBook->AddPage(new ThemePage(treeBook), wxT("Theme"), false, -1);
    treeBook->AddPage(nullptr, wxT("Python 3"), false, -1);
    treeBook->AddSubPage(new P3EditorGeneralPage(treeBook), wxT("General"), true, -1);
    treeBook->AddSubPage(new P3EditorStylePage(treeBook), wxT("Fonts and Colors"), false, -1);

    sizerRoot->Add(treeBook, wxSizerFlags(1).Expand())->SetId(kSpamPreferencesTreeBookCtrl);
    sizerRoot->Add(new wxStaticLine(this), wxSizerFlags().Expand());
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
