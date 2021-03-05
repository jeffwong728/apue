#include "pyeditor.h"
#include <wx/artprov.h>
#include <wx/wrapsizer.h>
#include <wx/stc/stc.h>
#include <ui/spam.h>
extern void PyAddImportPath(const std::string &strDir);

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
    stc->CmdKeyClear(wxSTC_KEY_TAB, 0);
    stc->SetLayoutCache(wxSTC_CACHE_PAGE);
    stc->UsePopUp(wxSTC_POPUP_ALL);
    stc->SetWhitespaceForeground(true, wxColour("DARK SLATE GREY"));
    stc->SetSelBackground(true, wxColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));

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

    if (SpamConfig::Get<bool>(cp_Py3EditorRememberScriptPath, true))
    {
        const wxString fullPath = SpamConfig::Get<wxString>(cp_Py3EditorScriptFullPath, wxT(""));
        const std::wstring ansiFilePath = fullPath.ToStdWstring();
        boost::system::error_code ec;
        boost::filesystem::path p(ansiFilePath);
        if (boost::filesystem::exists(p, ec) && boost::filesystem::is_regular_file(p, ec))
        {
            LoadPyFile(fullPath);
            PyAddImportPath(p.remove_filename().string());
        }
    }

    Show();
}

PyEditor::~PyEditor()
{
}

void PyEditor::LoadPyFile(const wxString &fullPath)
{
    auto stc = dynamic_cast<wxStyledTextCtrl *>(GetSizer()->GetItemById(kSpamPyEditorCtrl)->GetWindow());
    if (stc)
    {
        stc->LoadFile(fullPath);
        stc->EmptyUndoBuffer();

        if (SpamConfig::Get<bool>(cp_Py3EditorRememberScriptPath, true))
        {
            SpamConfig::Set(cp_Py3EditorScriptFullPath, fullPath);
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
