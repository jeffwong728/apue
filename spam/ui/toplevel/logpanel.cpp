#include "logpanel.h"
#include <ui/spam.h>
#include <wx/artprov.h>
#include <wx/wrapsizer.h>
#include <boost/container/flat_map.hpp>
extern std::pair<std::string, bool> PyRunCommand(const std::string &strCmd);

class WXConsoleCtrl : public wxTextCtrl
{
public:
    WXConsoleCtrl(wxWindow* parent) : wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_MULTILINE | wxTE_DONTWRAP)
    {
        Bind(wxEVT_CONTEXT_MENU, &WXConsoleCtrl::OnContextMenu, this);
    }
    ~WXConsoleCtrl() {}

public:
    bool CanCut() const wxOVERRIDE
    {
        long fromPos = -1, toPos = -1;
        GetSelection(&fromPos, &toPos);

        long fromX = 0, fromY = 0;
        long toX = 0, toY = 0;
        if (PositionToXY(fromPos, &fromX, &fromY) && PositionToXY(toPos, &toX, &toY))
        {
            if (fromY == toY &&
                fromY == GetNumberOfLines() - 1 &&
                fromX >=5 &&
                GetLineText(GetNumberOfLines() - 1).StartsWith(wxT(">>> ")))
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        return wxTextCtrl::CanCut();
    }

    bool CanPaste() const wxOVERRIDE
    {
        long fromPos = -1, toPos = -1;
        GetSelection(&fromPos, &toPos);
        if (fromPos != toPos)
        {
            return CanCut();
        }

        long x = 0, y = 0;
        const auto insPos = GetInsertionPoint();
        if (PositionToXY(insPos, &x, &y))
        {
            if (y < GetNumberOfLines() - 1)
            {
                return false;
            }
            else
            {
                if (4 > x &&
                    GetLineText(GetNumberOfLines() - 1).StartsWith(wxT(">>> ")))
                {
                    return false;
                }
            }
        }

        return wxTextCtrl::CanPaste();
    }

protected:
    void OnContextMenu(wxContextMenuEvent &evt)
    {
        wxMenu menu;
        menu.Append(wxID_UNDO, wxT("&Undo"));
        menu.Append(wxID_REDO, wxT("&Redo"));
        menu.AppendSeparator();
        if (CanCut()) menu.Append(wxID_CUT, wxT("Cu&t"));
        menu.Append(wxID_COPY, wxT("&Copy"));
        if (CanPaste()) menu.Append(wxID_PASTE, wxT("&Paste"));
        if (CanCut()) menu.Append(wxID_CLEAR, wxT("&Delete"));
        menu.AppendSeparator();
        menu.Append(wxID_SELECTALL, wxT("Select &All"));

        PopupMenu(&menu);
    }
};

ConsolePanel::ConsolePanel(wxWindow* parent)
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
    // Root sizer, vertical
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);
    auto consoleCtrl = new WXConsoleCtrl(this);
    sizerRoot->Add(consoleCtrl, wxSizerFlags(1).Expand())->SetId(kSpamLogTextCtrl);

    consoleCtrl->Bind(wxEVT_TEXT, &ConsolePanel::OnText, this);
    consoleCtrl->Bind(wxEVT_TEXT_ENTER, &ConsolePanel::OnEnter, this);
    consoleCtrl->Bind(wxEVT_CHAR, &ConsolePanel::OnKey, this);

    // Set sizer for the panel
    SetSizer(sizerRoot);
    GetSizer()->SetSizeHints(this);
    Show();

    cursor_ = cmds_.cend();

    delete wxLog::SetActiveTarget(new wxLogTextCtrl(consoleCtrl));
}

ConsolePanel::~ConsolePanel()
{
}

wxToolBar *ConsolePanel::MakeToolBar()
{
    wxToolBar *tb = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
    tb->SetToolBitmapSize(wxSize(16, 16));

    tb->AddTool(kSpamID_LOG_CLEAR, wxT("Clear"),    wxArtProvider::GetBitmap(wxART_DELETE, wxART_TOOLBAR, wxSize(16, 16)));
    tb->AddTool(kSpamID_LOG_SAVE, wxT("Save"), wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR, wxSize(16, 16)));
    tb->Realize();

    tb->Bind(wxEVT_TOOL, &ConsolePanel::OnClear, this, kSpamID_LOG_CLEAR);
    tb->Bind(wxEVT_TOOL, &ConsolePanel::OnSave, this, kSpamID_LOG_SAVE);

    return tb;
}

void ConsolePanel::OnClear(wxCommandEvent &cmd)
{
    auto consoleCtrl = dynamic_cast<wxTextCtrl *>(GetSizer()->GetItemById(kSpamLogTextCtrl)->GetWindow());
    if (consoleCtrl)
    {
        consoleCtrl->Clear();
    }
}

void ConsolePanel::OnSave(wxCommandEvent &cmd)
{
    boost::container::flat_map<int, int> fm;
}

void ConsolePanel::OnText(wxCommandEvent& evt)
{
    auto consoleCtrl = dynamic_cast<wxTextCtrl *>(GetSizer()->GetItemById(kSpamLogTextCtrl)->GetWindow());
    if (consoleCtrl)
    {
        //consoleCtrl->SetInsertionPointEnd();
    }
}

void ConsolePanel::OnAction(wxCommandEvent& evt)
{
    auto consoleCtrl = dynamic_cast<wxTextCtrl *>(GetSizer()->GetItemById(kSpamLogTextCtrl)->GetWindow());
    if (consoleCtrl)
    {
        const auto itemID = evt.GetEventType();
        if (wxEVT_TEXT_CUT == itemID || wxEVT_TEXT_PASTE == itemID)
        {
            const auto insPos = consoleCtrl->GetInsertionPoint();
            long x = 0, y = 0;
            if (consoleCtrl->PositionToXY(insPos, &x, &y))
            {
                if (y < consoleCtrl->GetNumberOfLines() - 1)
                {
                    consoleCtrl->SetInsertionPointEnd();
                    evt.Skip();
                }
                else
                {
                    if (5 > x &&
                        consoleCtrl->GetLineText(consoleCtrl->GetNumberOfLines() - 1).StartsWith(wxT(">>> ")))
                    {
                        evt.Skip();
                    }
                }
            }
        }
    }
}

void ConsolePanel::OnUpdateUI(wxUpdateUIEvent& evt)
{

}

void ConsolePanel::OnEnter(wxCommandEvent& evt)
{
    auto consoleCtrl = dynamic_cast<wxTextCtrl *>(GetSizer()->GetItemById(kSpamLogTextCtrl)->GetWindow());
    if (consoleCtrl)
    {
        wxString strLine = consoleCtrl->GetLineText(consoleCtrl->GetNumberOfLines() - 1);
        if (strLine.StartsWith(wxT(">>> ")))
        {
            consoleCtrl->AppendText(wxT("\n"));
            wxString strCmd = strLine.SubString(4, strLine.Length()-1);
            if (!strCmd.IsEmpty())
            {
                std::pair<std::string, bool> res = PyRunCommand(strCmd.ToStdString());
                if (res.second)
                {
                    cursor_ = cmds_.insert(cursor_, strCmd);
                    if (!res.first.empty())
                    {
                        consoleCtrl->AppendText(res.first);
                    }
                }
                else
                {
                    Spam::PopupPyError(res.first);
                }
            }
            consoleCtrl->AppendText(wxT(">>> "));
        }
        else
        {
            consoleCtrl->AppendText(wxT("\n>>> "));
        }
    }
}

void ConsolePanel::OnKey(wxKeyEvent &evt)
{
    auto consoleCtrl = dynamic_cast<wxTextCtrl *>(GetSizer()->GetItemById(kSpamLogTextCtrl)->GetWindow());
    if (consoleCtrl)
    {
        const auto insPos = consoleCtrl->GetInsertionPoint();
        long x = 0;
        long y = 0;
        if (consoleCtrl->PositionToXY(insPos, &x, &y))
        {
            if (y < consoleCtrl->GetNumberOfLines() - 1)
            {
                consoleCtrl->SetInsertionPointEnd();
                return;
            }
            else
            {
                if (WXK_BACK == evt.GetKeyCode() &&
                    5 > x &&
                    consoleCtrl->GetLineText(consoleCtrl->GetNumberOfLines() - 1).StartsWith(wxT(">>> ")))
                {
                    return;
                }

                if (!cmds_.empty())
                {
                    if (WXK_UP == evt.GetKeyCode() && consoleCtrl->GetLineText(consoleCtrl->GetNumberOfLines() - 1).StartsWith(wxT(">>> ")))
                    {
                        if (cursor_ != cmds_.cend())
                        {
                            consoleCtrl->Replace(consoleCtrl->XYToPosition(4, consoleCtrl->GetNumberOfLines() - 1), consoleCtrl->GetLastPosition(), *cursor_);
                            cursor_ = std::next(cursor_);
                        }

                        return;
                    }

                    if (WXK_DOWN == evt.GetKeyCode() && consoleCtrl->GetLineText(consoleCtrl->GetNumberOfLines() - 1).StartsWith(wxT(">>> ")))
                    {
                        if (cursor_ == cmds_.cend())
                        {
                            if (cursor_ != cmds_.cbegin()) cursor_ = std::prev(cursor_);
                            if (cursor_ != cmds_.cbegin()) cursor_ = std::prev(cursor_);
                            consoleCtrl->Replace(consoleCtrl->XYToPosition(4, consoleCtrl->GetNumberOfLines() - 1), consoleCtrl->GetLastPosition(), *cursor_);
                        }
                        else if (cursor_ == cmds_.cbegin())
                        {
                            consoleCtrl->Replace(consoleCtrl->XYToPosition(4, consoleCtrl->GetNumberOfLines() - 1), consoleCtrl->GetLastPosition(), *cursor_);
                        }
                        else
                        {
                            if (cursor_ != cmds_.cbegin()) cursor_ = std::prev(cursor_);
                            consoleCtrl->Replace(consoleCtrl->XYToPosition(4, consoleCtrl->GetNumberOfLines() - 1), consoleCtrl->GetLastPosition(), *cursor_);
                        }

                        return;
                    }
                }
            }
        }
        consoleCtrl->OnChar(evt);
    }
}
