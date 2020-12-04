#include "pyeditor.h"
#include <wx/artprov.h>
#include <wx/wrapsizer.h>
#include <wx/stc/stc.h>

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
        stc->StyleSetForeground(st, wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
        stc->StyleSetBackground(st, wxSystemSettings::GetColour(wxSYS_COLOUR_DESKTOP));
    }

    for (int st = wxSTC_P_DEFAULT; st <= wxSTC_P_DECORATOR; ++st)
    {
        stc->StyleSetForeground(st, wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
        stc->StyleSetBackground(st, wxSystemSettings::GetColour(wxSYS_COLOUR_DESKTOP));
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

    stc->StyleSetBold(wxSTC_P_WORD, true);
    stc->StyleSetBold(wxSTC_P_WORD2, true);
    stc->StyleSetBold(wxSTC_P_DEFNAME, true);

    stc->StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour("GREY"));
    stc->StyleSetForeground(wxSTC_P_COMMENTLINE, wxColour("MEDIUM GREY"));
    stc->StyleSetForeground(wxSTC_P_NUMBER, wxColour("RED"));
    stc->StyleSetForeground(wxSTC_P_STRING, wxColour(214, 157, 133));
    stc->StyleSetForeground(wxSTC_P_CHARACTER, wxColour(214, 157, 133));
    stc->StyleSetForeground(wxSTC_P_WORD, wxColour("YELLOW"));
    stc->StyleSetForeground(wxSTC_P_TRIPLE, wxColour(214, 157, 133));
    stc->StyleSetForeground(wxSTC_P_TRIPLEDOUBLE, wxColour(214, 157, 133));
    stc->StyleSetForeground(wxSTC_P_CLASSNAME, wxColour("TURQUOISE"));
    stc->StyleSetForeground(wxSTC_P_DEFNAME, wxColour(78, 201, 176));
    stc->StyleSetForeground(wxSTC_P_OPERATOR, wxColour(128, 255, 255));
    stc->StyleSetForeground(wxSTC_P_IDENTIFIER, wxColour("LIGHT GREY"));
    stc->StyleSetForeground(wxSTC_P_COMMENTBLOCK, wxColour("MEDIUM GREY"));
    stc->StyleSetForeground(wxSTC_P_WORD2, wxColour("YELLOW"));

    // Set sizer for the panel
    SetSizer(sizerRoot);
    GetSizer()->SetSizeHints(this);
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
    }
}
