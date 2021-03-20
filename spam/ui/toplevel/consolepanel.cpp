#include "consolepanel.h"
#include <ui/spam.h>
#include <wx/artprov.h>
#include <wx/wrapsizer.h>
#include <boost/container/flat_map.hpp>
#include <tbb/tick_count.h>
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
        if (CanCut()) menu.Append(wxID_DELETE, wxT("&Delete"));
        menu.AppendSeparator();
        menu.Append(wxID_SELECTALL, wxT("Select &All"));
        menu.Append(wxID_CLEAR, wxT("Cl&ear"));

        PopupMenu(&menu);
    }
};

ConsolePanel::ConsolePanel(wxWindow* parent)
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
    // Root sizer, vertical
    wxSizer * const sizerRoot = new wxBoxSizer(wxHORIZONTAL);
    auto consoleCtrl = new WXConsoleCtrl(this);
    auto timeCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(80, 24), wxTE_READONLY | wxTE_MULTILINE);
    sizerRoot->Add(consoleCtrl, wxSizerFlags(1).Expand())->SetId(kSpamConsoleTextCtrl);
    sizerRoot->Add(timeCtrl, wxSizerFlags().Expand())->SetId(kSpamConsoleTimeCtrl);

    consoleCtrl->Bind(wxEVT_TEXT, &ConsolePanel::OnText, this);
    consoleCtrl->Bind(wxEVT_TEXT_ENTER, &ConsolePanel::OnEnter, this);
    consoleCtrl->Bind(wxEVT_CHAR, &ConsolePanel::OnKey, this);
    consoleCtrl->Bind(wxEVT_MENU, &ConsolePanel::OnClear, this, wxID_CLEAR);
    consoleCtrl->Bind(wxEVT_SCROLLWIN_TOP, &ConsolePanel::OnConsoleScroll, this);
    consoleCtrl->Bind(wxEVT_SCROLLWIN_BOTTOM, &ConsolePanel::OnConsoleScroll, this);
    consoleCtrl->Bind(wxEVT_SCROLLWIN_LINEUP, &ConsolePanel::OnConsoleScroll, this);
    consoleCtrl->Bind(wxEVT_SCROLLWIN_LINEDOWN, &ConsolePanel::OnConsoleScroll, this);
    consoleCtrl->Bind(wxEVT_SCROLLWIN_PAGEUP, &ConsolePanel::OnConsoleScroll, this);
    consoleCtrl->Bind(wxEVT_SCROLLWIN_PAGEDOWN, &ConsolePanel::OnConsoleScroll, this);
    consoleCtrl->Bind(wxEVT_SCROLLWIN_THUMBTRACK, &ConsolePanel::OnConsoleScroll, this);
    consoleCtrl->Bind(wxEVT_SCROLLWIN_THUMBRELEASE, &ConsolePanel::OnConsoleScroll, this);
    timeCtrl->Bind(wxEVT_SCROLLWIN_TOP, &ConsolePanel::OnConsoleScroll, this);
    timeCtrl->Bind(wxEVT_SCROLLWIN_BOTTOM, &ConsolePanel::OnConsoleScroll, this);
    timeCtrl->Bind(wxEVT_SCROLLWIN_LINEUP, &ConsolePanel::OnConsoleScroll, this);
    timeCtrl->Bind(wxEVT_SCROLLWIN_LINEDOWN, &ConsolePanel::OnConsoleScroll, this);
    timeCtrl->Bind(wxEVT_SCROLLWIN_PAGEUP, &ConsolePanel::OnConsoleScroll, this);
    timeCtrl->Bind(wxEVT_SCROLLWIN_PAGEDOWN, &ConsolePanel::OnConsoleScroll, this);
    timeCtrl->Bind(wxEVT_SCROLLWIN_THUMBTRACK, &ConsolePanel::OnConsoleScroll, this);
    timeCtrl->Bind(wxEVT_SCROLLWIN_THUMBRELEASE, &ConsolePanel::OnConsoleScroll, this);

    // Set sizer for the panel
    SetSizer(sizerRoot);
    GetSizer()->SetSizeHints(this);
    Show();
    consoleCtrl->AppendText(wxT(">>> "));

    cursor_ = cmds_.cend();
}

ConsolePanel::~ConsolePanel()
{
}

void ConsolePanel::OnClear(wxCommandEvent &cmd)
{
    auto consoleCtrl = dynamic_cast<wxTextCtrl  *>(GetSizer()->GetItemById(kSpamConsoleTextCtrl)->GetWindow());
    auto timeCtrl = dynamic_cast<wxTextCtrl  *>(GetSizer()->GetItemById(kSpamConsoleTimeCtrl)->GetWindow());
    if (consoleCtrl && timeCtrl)
    {
        consoleCtrl->Clear();
        timeCtrl->Clear();
        consoleCtrl->AppendText(wxT(">>> "));
    }
}

void ConsolePanel::OnSave(wxCommandEvent &cmd)
{
    boost::container::flat_map<int, int> fm;
}

void ConsolePanel::OnText(wxCommandEvent& evt)
{
    auto consoleCtrl = dynamic_cast<wxTextCtrl  *>(GetSizer()->GetItemById(kSpamConsoleTextCtrl)->GetWindow());
    if (consoleCtrl)
    {
        //consoleCtrl->SetInsertionPointEnd();
    }
}

void ConsolePanel::OnAction(wxCommandEvent& evt)
{
    auto consoleCtrl = dynamic_cast<wxTextCtrl  *>(GetSizer()->GetItemById(kSpamConsoleTextCtrl)->GetWindow());
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
    auto consoleCtrl = dynamic_cast<wxTextCtrl  *>(GetSizer()->GetItemById(kSpamConsoleTextCtrl)->GetWindow());
    auto timeCtrl = dynamic_cast<wxTextCtrl  *>(GetSizer()->GetItemById(kSpamConsoleTimeCtrl)->GetWindow());
    if (consoleCtrl && timeCtrl)
    {
        double timeMS = 0;
        wxString strLine = consoleCtrl->GetLineText(consoleCtrl->GetNumberOfLines() - 1);
        if (strLine.StartsWith(wxT(">>> ")))
        {
            consoleCtrl->AppendText(wxT("\n"));
            wxString strCmd = strLine.SubString(4, strLine.Length()-1).Strip(wxString::both);
            if (!strCmd.IsEmpty())
            {
                if (wxString("clear") == strCmd)
                {
                    consoleCtrl->Clear();
                    timeCtrl->Clear();
                }
                else
                {
                    wxBusyCursor wait;
                    tbb::tick_count t1 = tbb::tick_count::now();
                    std::pair<std::string, bool> res = PyRunCommand(strCmd.ToStdString());
                    tbb::tick_count t2 = tbb::tick_count::now();
                    timeMS = (t2 - t1).seconds() * 1000;
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
            }
            consoleCtrl->AppendText(wxT(">>> "));
        }
        else
        {
            consoleCtrl->AppendText(wxT("\n>>> "));
        }

        int numLeftLines = consoleCtrl->GetNumberOfLines();
        int numRightLines = timeCtrl->GetNumberOfLines();
        if (numRightLines < numLeftLines && timeMS > 1e-6)
        {
            if (timeMS < 100)
            {
                timeCtrl->AppendText(wxString::Format(wxT("%.2f ms\n"), timeMS));
            }
            else
            {
                timeCtrl->AppendText(wxString::Format(wxT("%.2f s\n"), timeMS/1000));
            }

            numRightLines += 1;
        }

        while (numRightLines < numLeftLines)
        {
            timeCtrl->AppendText(wxT(" ~ \n"));
            numRightLines += 1;
        }
    }
}

void ConsolePanel::OnKey(wxKeyEvent &evt)
{
    auto consoleCtrl = dynamic_cast<wxTextCtrl  *>(GetSizer()->GetItemById(kSpamConsoleTextCtrl)->GetWindow());
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

void ConsolePanel::OnConsoleScroll(wxScrollWinEvent & evt)
{
    if (wxVERTICAL == evt.GetOrientation())
    {
        auto consoleCtrl = dynamic_cast<wxTextCtrl  *>(GetSizer()->GetItemById(kSpamConsoleTextCtrl)->GetWindow());
        auto timeCtrl = dynamic_cast<wxTextCtrl  *>(GetSizer()->GetItemById(kSpamConsoleTimeCtrl)->GetWindow());
        if (consoleCtrl && timeCtrl)
        {
            timeCtrl->SetScrollPos(wxVERTICAL, evt.GetPosition());
        }
    }
}

void ConsolePanel::OnTimingScroll(wxScrollWinEvent & evt)
{
    if (wxVERTICAL == evt.GetOrientation())
    {
        auto consoleCtrl = dynamic_cast<wxTextCtrl  *>(GetSizer()->GetItemById(kSpamConsoleTextCtrl)->GetWindow());
        auto timeCtrl = dynamic_cast<wxTextCtrl  *>(GetSizer()->GetItemById(kSpamConsoleTimeCtrl)->GetWindow());
        if (consoleCtrl && timeCtrl)
        {
            consoleCtrl->SetScrollPos(wxVERTICAL, evt.GetPosition());
        }
    }
}
