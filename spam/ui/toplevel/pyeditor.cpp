#include "pyeditor.h"
#include <wx/artprov.h>
#include <wx/wrapsizer.h>
#include <wx/stc/stc.h>
#include <ui/spam.h>
extern void PyAddImportPath(const std::string &strDir);
extern std::pair<std::string, bool> PyRunStrings(const std::string &strCmd);

PyEditor::PyEditor(wxWindow* parent)
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
    // Root sizer, vertical
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    auto stc = new wxStyledTextCtrl(this, wxID_ANY);
    sizerRoot->Add(stc, wxSizerFlags(1).Expand())->SetId(kSpamPyEditorCtrl);

    stc->StyleClearAll();
    stc->SetLexer(wxSTC_LEX_PYTHON);

    stc->SetCaretForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    stc->SetMarginType(0, wxSTC_MARGIN_NUMBER);
    stc->SetMarginWidth(0, stc->TextWidth(wxSTC_STYLE_LINENUMBER, "_9999"));
    stc->SetIndentationGuides(TRUE);
    stc->SetViewWhiteSpace(wxSTC_WS_VISIBLEALWAYS);
    stc->SetLayoutCache(wxSTC_CACHE_PAGE);
    stc->UsePopUp(wxSTC_POPUP_ALL);
    stc->SetWhitespaceForeground(true, wxColour("GREY"));
    stc->SetSelBackground(true, wxColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
    stc->SetIndent(4);
    stc->SetUseTabs(false);
    stc->SetTabWidth(4);

    for (int st = wxSTC_STYLE_DEFAULT; st <= wxSTC_STYLE_LASTPREDEFINED; ++st)
    {
        stc->StyleSetForeground(st, config_GetPy3EditorForeground(st));
        stc->StyleSetBackground(st, config_GetPy3EditorBackground(st));
        stc->StyleSetFont(st, config_GetPy3EditorFont(st));
    }

    for (int st = wxSTC_P_DEFAULT; st <= wxSTC_P_DECORATOR; ++st)
    {
        stc->StyleSetForeground(st, config_GetPy3EditorForeground(st));
        stc->StyleSetBackground(st, config_GetPy3EditorBackground(st));
        stc->StyleSetBold(st, config_GetPy3EditorBold(st));
        stc->StyleSetFont(st, config_GetPy3EditorFont(st));
    }

    const char* PythonWordlist1 =
        "and assert break class continue def del elif else except exec "
        "finally for from global if import in is lambda None not or pass "
        "print raise return try while yield";
    const char* PythonWordlist2 =
        "ACCELERATORS ALT AUTO3STATE AUTOCHECKBOX AUTORADIOBUTTON BEGIN "
        "BITMAP BLOCK BUTTON CAPTION CHARACTERISTICS CHECKBOX CLASS "
        "COMBOBOX CONTROL CTEXT CURSOR DEFPUSHBUTTON DIALOG DIALOGEX "
        "DISCARDABLE EDITTEXT END EXSTYLE FONT GROUPBOX ICON LANGUAGE "
        "LISTBOX LTEXT MENU MENUEX MENUITEM MESSAGETABLE POPUP PUSHBUTTON "
        "RADIOBUTTON RCDATA RTEXT SCROLLBAR SEPARATOR SHIFT STATE3 "
        "STRINGTABLE STYLE TEXTINCLUDE VALUE VERSION VERSIONINFO VIRTKEY";

    stc->SetKeyWords(0, PythonWordlist1);
    stc->SetKeyWords(1, PythonWordlist2);

    // Set sizer for the panel
    SetSizer(sizerRoot);
    GetSizer()->SetSizeHints(this);

    Show();
}

PyEditor::~PyEditor()
{
}

void PyEditor::LoadDefaultPyFile()
{
    if (SpamConfig::Get<bool>(cp_Py3EditorRememberScriptPath, true))
    {
        LoadPyFile(SpamConfig::Get<wxString>(cp_Py3EditorScriptFullPath, wxT("")));
    }
}

void PyEditor::LoadPyFile(const wxString &fullPath)
{
    auto stc = dynamic_cast<wxStyledTextCtrl *>(GetSizer()->GetItemById(kSpamPyEditorCtrl)->GetWindow());
    if (stc)
    {
        boost::system::error_code ec;
        const std::wstring ansiFilePath = fullPath.ToStdWstring();
        boost::filesystem::path p(ansiFilePath);
        if (boost::filesystem::exists(p, ec) && boost::filesystem::is_regular_file(p, ec))
        {
            std::string pyDir = p.remove_filename().string();
            if (pyImpDirs_.insert(pyDir).second)
            {
                PyAddImportPath(pyDir);
            }

            stc->LoadFile(fullPath);
            stc->EmptyUndoBuffer();

            if (SpamConfig::Get<bool>(cp_Py3EditorRememberScriptPath, true))
            {
                SpamConfig::Set(cp_Py3EditorScriptFullPath, fullPath);
            }
        }
    }
}

void PyEditor::SavePyFile() const
{
    auto stc = dynamic_cast<wxStyledTextCtrl *>(GetSizer()->GetItemById(kSpamPyEditorCtrl)->GetWindow());
    if (stc)
    {
        const wxString fullPath = SpamConfig::Get<wxString>(cp_Py3EditorScriptFullPath, wxT(""));
        stc->SaveFile(fullPath);
    }
}

void PyEditor::PlayPyFile() const
{
    auto stc = dynamic_cast<wxStyledTextCtrl *>(GetSizer()->GetItemById(kSpamPyEditorCtrl)->GetWindow());
    if (stc)
    {
        std::pair<std::string, bool> res{"", true};
        {
            wxBusyCursor wait;
            res = PyRunStrings(stc->GetText().ToStdString());
        }

        if (res.second)
        {
            Spam::LogPyOutput();
        }
        else
        {
            Spam::PopupPyError(res.first);
        }
    }
}

void PyEditor::ApplyStyleChange()
{
    auto stc = dynamic_cast<wxStyledTextCtrl *>(GetSizer()->GetItemById(kSpamPyEditorCtrl)->GetWindow());
    if (stc)
    {
        stc->Freeze();
        for (int st = wxSTC_STYLE_DEFAULT; st <= wxSTC_STYLE_LASTPREDEFINED; ++st)
        {
            stc->StyleSetForeground(st, config_GetPy3EditorForeground(st));
            stc->StyleSetBackground(st, config_GetPy3EditorBackground(st));
            stc->StyleSetFont(st, config_GetPy3EditorFont(st));
        }

        for (int st = wxSTC_P_DEFAULT; st <= wxSTC_P_DECORATOR; ++st)
        {
            stc->StyleSetForeground(st, config_GetPy3EditorForeground(st));
            stc->StyleSetBackground(st, config_GetPy3EditorBackground(st));
            stc->StyleSetBold(st, config_GetPy3EditorBold(st));
            stc->StyleSetFont(st, config_GetPy3EditorFont(st));
        }
        stc->Thaw();
    }
}
