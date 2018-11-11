#include "cvtilepanel.h"
#include "cvwidget.h"
#include <opencv2/highgui.hpp>

CVTilePanel::CVTilePanel(wxWindow* parent, const std::string &cvWinName, const wxString &panelName)
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER, panelName)
{
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    sizerRoot->Add(new CVWidget(this, cvWinName), 1, wxEXPAND)->SetId(0);

    Bind(wxEVT_CHAR, &CVTilePanel::OnChar, this, wxID_ANY);

    SetSizer(sizerRoot);
    GetSizer()->SetSizeHints(this);
    Show();
}

CVTilePanel::~CVTilePanel()
{
}

void CVTilePanel::AdjustImgWndSize(const int id, const wxSize &imgSize) const
{
    wxSizerItem* sizerItem = GetSizer()->GetItemById(id);
    auto sizeDelta = sizerItem->GetSize()-sizerItem->GetWindow()->GetClientSize();
    sizerItem->SetMinSize(imgSize + sizeDelta);
    GetSizer()->Layout();
}

void CVTilePanel::OnChar(wxKeyEvent &e)
{
    wxString strC(static_cast<char>(e.GetKeyCode()), 1);
    strC.UpperCase();
    wxLogMessage(wxT("CVTilePanel::OnLeftMouseDown - ") + strC);
}