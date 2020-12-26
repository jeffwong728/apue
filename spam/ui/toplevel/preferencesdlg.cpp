#include "preferencesdlg.h"
#include <wx/artprov.h>
#include <wx/wrapsizer.h>
#include <wx/treebook.h>
#include <wx/statline.h>
#include <wx/clrpicker.h>
#include <wx/fontpicker.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/stc/stc.h>
#include <ui/spam.h>
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

        pSWC->SetValue(SpamConfig::Get<bool>(cp_ThemeDarkMode, true));
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
            SpamConfig::Set(cp_ThemeDarkMode, true);
            g_object_set(G_OBJECT(settings), "gtk-application-prefer-dark-theme", 1, NULL);
        }
        else
        {
            SpamConfig::Set(cp_ThemeDarkMode, false);
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
        rememberPy3Ctrl = new wxCheckBox(this, wxID_ANY, wxT("Remember Last Python 3 Script Path"));
        sizerRoot->Add(new wxCheckBox(this, wxID_ANY, wxT("View Lines Number")), wxSizerFlags().DoubleBorder(wxTOP | wxLEFT | wxRIGHT));
        sizerRoot->Add(new wxCheckBox(this, wxID_ANY, wxT("View White Space")), wxSizerFlags().DoubleBorder(wxLEFT | wxRIGHT));
        sizerRoot->Add(new wxCheckBox(this, wxID_ANY, wxT("View Indentation Guides")), wxSizerFlags().DoubleBorder(wxBOTTOM | wxLEFT | wxRIGHT));
        sizerRoot->Add(rememberPy3Ctrl, wxSizerFlags().DoubleBorder(wxTOP | wxLEFT | wxRIGHT));

        rememberPy3Ctrl->SetValue(SpamConfig::Get<bool>(cp_Py3EditorRememberScriptPath, true));
        rememberPy3Ctrl->Bind(wxEVT_CHECKBOX, &P3EditorGeneralPage::OnRememberPy3Ctrl, this, wxID_ANY);

        SetSizer(sizerRoot);
        GetSizer()->SetSizeHints(this);
    }
    ~P3EditorGeneralPage() {}

private:
    void OnRememberPy3Ctrl(wxCommandEvent &evt)
    {
        SpamConfig::Set<bool>(cp_Py3EditorRememberScriptPath, evt.GetInt());
    }

private:
    wxCheckBox *rememberPy3Ctrl = nullptr;
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
        catListBox = new wxListBox(this, wxID_ANY);
        catListBox->Append(wxString(wxT("Default Style")), (void*)wxSTC_STYLE_DEFAULT);
        catListBox->Append(wxString(wxT("Keyword")), (void*)wxSTC_P_WORD);
        catListBox->Append(wxString(wxT("Number")), (void*)wxSTC_P_NUMBER);
        catListBox->Append(wxString(wxT("String")), (void*)wxSTC_P_STRING);
        catListBox->Append(wxString(wxT("Class Name")), (void*)wxSTC_P_CLASSNAME);
        catListBox->Append(wxString(wxT("Def Name")), (void*)wxSTC_P_DEFNAME);
        catListBox->Append(wxString(wxT("Operator")), (void*)wxSTC_P_OPERATOR);
        catListBox->Append(wxString(wxT("Identifier")), (void*)wxSTC_P_IDENTIFIER);
        catListBox->Append(wxString(wxT("Line Number")), (void*)wxSTC_STYLE_LINENUMBER);
        catListBox->Append(wxString(wxT("Character")), (void*)wxSTC_P_CHARACTER);
        catListBox->Append(wxString(wxT("Comment Line")), (void*)wxSTC_P_COMMENTLINE);
        catListBox->Append(wxString(wxT("Comment Block")), (void*)wxSTC_P_COMMENTBLOCK);
        catListBox->Select(0);
        sizerRoot->Add(catListBox, wxSizerFlags(0).DoubleBorder().Expand());

        wxBoxSizer * const sizerSetting = new wxBoxSizer(wxVERTICAL);
        wxFlexGridSizer * const sizerTopSetting = new wxFlexGridSizer(2, 2, 2);
        foreColorCtrl = new wxColourPickerCtrl(this, wxID_ANY);
        backColorCtrl = new wxColourPickerCtrl(this, wxID_ANY);
        fontCtrl = new wxFontPickerCtrl(this, wxID_ANY, *wxSWISS_FONT, DP, DS, wxFNTP_FONTDESC_AS_LABEL);
        sizerTopSetting->AddGrowableCol(1, 1);
        sizerTopSetting->SetFlexibleDirection(wxHORIZONTAL);
        sizerTopSetting->Add(new wxStaticText(this, wxID_ANY, wxT("Foreground Color")), wxSizerFlags().Right().DoubleBorder());
        sizerTopSetting->Add(foreColorCtrl, wxSizerFlags(1).Expand().HorzBorder());
        sizerTopSetting->Add(new wxStaticText(this, wxID_ANY, wxT("Background Color")), wxSizerFlags().Right().DoubleBorder());
        sizerTopSetting->Add(backColorCtrl, wxSizerFlags(1).Expand().HorzBorder());
        sizerTopSetting->Add(new wxStaticText(this, wxID_ANY, wxT("Font")), wxSizerFlags().Right().DoubleBorder());
        sizerTopSetting->Add(fontCtrl, wxSizerFlags(1).Expand().HorzBorder())->SetId(kPy3EditorStyleFontCtrl);
        sizerSetting->Add(sizerTopSetting, wxSizerFlags(0).DoubleBorder().Expand());

        wxBoxSizer * const sizerBotSetting = new wxBoxSizer(wxVERTICAL);
        boldCtrl = new wxCheckBox(this, wxID_ANY, wxT("Bold"));
        italicCtrl = new wxCheckBox(this, wxID_ANY, wxT("Italic"));
        underlineCtrl = new wxCheckBox(this, wxID_ANY, wxT("Underline"));
        sizerBotSetting->Add(boldCtrl, wxSizerFlags().DoubleBorder(wxLEFT | wxRIGHT));
        sizerBotSetting->Add(italicCtrl, wxSizerFlags().DoubleBorder(wxLEFT | wxRIGHT));
        sizerBotSetting->Add(underlineCtrl, wxSizerFlags().DoubleBorder(wxLEFT | wxRIGHT));
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

        catListBox->Bind(wxEVT_LISTBOX, &P3EditorStylePage::OnListBox, this, wxID_ANY);
        foreColorCtrl->Bind(wxEVT_COLOURPICKER_CHANGED, &P3EditorStylePage::OnForegroundColor, this, wxID_ANY);
        backColorCtrl->Bind(wxEVT_COLOURPICKER_CHANGED, &P3EditorStylePage::OnBackgroundColor, this, wxID_ANY);
        boldCtrl->Bind(wxEVT_CHECKBOX, &P3EditorStylePage::OnBold, this, wxID_ANY);
        italicCtrl->Bind(wxEVT_CHECKBOX, &P3EditorStylePage::OnItalic, this, wxID_ANY);
        underlineCtrl->Bind(wxEVT_CHECKBOX, &P3EditorStylePage::OnUnderline, this, wxID_ANY);
        fontCtrl->Bind(wxEVT_FONTPICKER_CHANGED, &P3EditorStylePage::OnFontChanged, this, wxID_ANY);

        foreColorCtrl->SetColour(config_GetPy3EditorForeground(wxSTC_STYLE_DEFAULT));
        backColorCtrl->SetColour(config_GetPy3EditorBackground(wxSTC_STYLE_DEFAULT));
        boldCtrl->SetValue(config_GetPy3EditorBold(wxSTC_STYLE_DEFAULT));
        fontCtrl->SetSelectedFont(config_GetPy3EditorFont(wxSTC_STYLE_DEFAULT));

        SetSizer(sizerRoot);
        GetSizer()->SetSizeHints(this);
    }
    ~P3EditorStylePage() {}

private:
    void OnListBox(wxCommandEvent& evt)
    {
        int styleCatId = static_cast<int>((long long)evt.GetClientData());
        foreColorCtrl->SetColour(config_GetPy3EditorForeground(styleCatId));
        backColorCtrl->SetColour(config_GetPy3EditorBackground(styleCatId));
        boldCtrl->SetValue(config_GetPy3EditorBold(styleCatId));
        fontCtrl->SetSelectedFont(config_GetPy3EditorFont(styleCatId));
    }

    void OnForegroundColor(wxColourPickerEvent& evt)
    {
        int styleCatId = static_cast<int>((long long)catListBox->GetClientData(catListBox->GetSelection()));
        SpamConfig::Set(config_GetPy3EditorStyleFullName(Py3EditorStyle::kPES_FOREGROUND, styleCatId), evt.GetColour());
    }

    void OnBackgroundColor(wxColourPickerEvent& evt)
    {
        int styleCatId = static_cast<int>((long long)catListBox->GetClientData(catListBox->GetSelection()));
        SpamConfig::Set(config_GetPy3EditorStyleFullName(Py3EditorStyle::kPES_BACKGROUND, styleCatId), evt.GetColour());
    }

    void OnBold(wxCommandEvent& evt)
    {
        int styleCatId = static_cast<int>((long long)catListBox->GetClientData(catListBox->GetSelection()));
        SpamConfig::Set<bool>(config_GetPy3EditorStyleFullName(Py3EditorStyle::kPES_BOLD, styleCatId), evt.GetInt());
    }

    void OnItalic(wxCommandEvent& evt)
    {
        int styleCatId = static_cast<int>((long long)catListBox->GetClientData(catListBox->GetSelection()));
        SpamConfig::Set<bool>(config_GetPy3EditorStyleFullName(Py3EditorStyle::kPES_ITALIC, styleCatId), evt.GetInt());
    }

    void OnUnderline(wxCommandEvent& evt)
    {
        int styleCatId = static_cast<int>((long long)catListBox->GetClientData(catListBox->GetSelection()));
        SpamConfig::Set<bool>(config_GetPy3EditorStyleFullName(Py3EditorStyle::kPES_UNDERLINE, styleCatId), evt.GetInt());
    }

    void OnFontChanged(wxFontPickerEvent& evt)
    {
        int styleCatId = static_cast<int>((long long)catListBox->GetClientData(catListBox->GetSelection()));
        SpamConfig::Set(config_GetPy3EditorStyleFullName(Py3EditorStyle::kPES_FONT, styleCatId), evt.GetFont());
    }

private:
    wxListBox *catListBox               = nullptr;
    wxColourPickerCtrl *foreColorCtrl   = nullptr;
    wxColourPickerCtrl *backColorCtrl   = nullptr;
    wxFontPickerCtrl *fontCtrl          = nullptr;
    wxCheckBox *boldCtrl                = nullptr;
    wxCheckBox *italicCtrl              = nullptr;
    wxCheckBox *underlineCtrl           = nullptr;
};

PreferencesDlg::PreferencesDlg(wxWindow* parent)
: wxDialog(parent, wxID_ANY, wxT("Preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    wxTreebook *treeBook = new wxTreebook(this, wxID_ANY, wxDefaultPosition, wxSize(640, 480));
    treeBook->AddPage(new ThemePage(treeBook), wxT("Theme"), false, -1);
    treeBook->AddPage(nullptr, wxT("Python 3"), false, -1);
    treeBook->AddSubPage(new P3EditorGeneralPage(treeBook), wxT("General"), false, -1);
    treeBook->AddSubPage(new P3EditorStylePage(treeBook), wxT("Fonts and Colors"), true, -1);

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
