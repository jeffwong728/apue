#include "instructiontip.h"
#include <wx/artprov.h>
#include <wx/display.h>
#include <wx/graphics.h>
#include <wx/generic/stattextg.h>

InstructionTip::InstructionTip(wxWindow* parent,
                       const wxString& message,
                       const wxBitmap& icon)
{
    Create(parent, wxFRAME_SHAPED);

    wxBoxSizer* const sizer = new wxBoxSizer(wxHORIZONTAL);
    if ( icon.IsOk() )
    {
        sizer->Add(new wxStaticBitmap(this, wxID_ANY, icon), wxSizerFlags().Centre().Border(wxRIGHT));
    }

    auto pMsg = new wxGenericStaticText(this, wxID_ANY, message);
    pMsg->SetLabelMarkup(message);
    sizer->Add(pMsg, wxSizerFlags().Centre().Border(wxALL));

    SetSizer(sizer);
    SetTipShapeAndSize(GetBestSize());
    Layout();
}

InstructionTip::InstructionTip(wxWindow* parent, const std::vector<wxString> &messages, const wxBitmap& icon)
{
    Create(parent, wxFRAME_SHAPED);
    wxBoxSizer* const sizer = new wxBoxSizer(wxHORIZONTAL);

    if (icon.IsOk())
    {
        sizer->Add(new wxStaticBitmap(this, wxID_ANY, icon), wxSizerFlags().Centre().Border(wxRIGHT));
    }

    wxBoxSizer* const msgSizer = new wxBoxSizer(wxVERTICAL);
    for (const auto &msg : messages)
    {
        auto pMsg = new wxGenericStaticText(this, wxID_ANY, msg);
        pMsg->SetLabelMarkup(msg);
        msgSizer->Add(pMsg, wxSizerFlags().Left().Border(wxLEFT | wxRIGHT));
    }
    sizer->Add(msgSizer, wxSizerFlags().Centre().Border(wxALL));

    SetSizer(sizer);
    SetTipShapeAndSize(GetBestSize());
    Layout();
}

void InstructionTip::SetBackgroundColours(wxColour colStart, wxColour colEnd)
{
    if (!colStart.IsOk())
    {
        colStart = wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK);
    }

    if ( colEnd.IsOk() )
    {
        // Use gradient-filled background bitmap.
        const wxSize size = GetClientSize();
        wxBitmap bmp(size);
        {
            wxMemoryDC dc(bmp);
            dc.Clear();
            dc.GradientFillLinear(size, colStart, colEnd, wxDOWN);
        }

        SetBackgroundBitmap(bmp);
    }
    else // Use solid colour.
    {
        SetBackgroundColour(colStart);
    }
}

void InstructionTip::SetTipShapeAndSize(const wxSize& contentSize)
{
    wxSize size = contentSize;
    constexpr double RADIUS = 5;
    wxGraphicsPath path = wxGraphicsRenderer::GetDefaultRenderer()->CreatePath();
    path.AddRoundedRectangle(0, 0, size.x, size.y, RADIUS);

    SetSize(size);
    SetShape(path);
}

InformationTip::InformationTip(wxWindow* parent, const std::vector<wxString> &messages, const wxBitmap& icon)
{
    Create(parent, wxFRAME_SHAPED);
    wxBoxSizer* const sizer = new wxBoxSizer(wxHORIZONTAL);

    if (icon.IsOk())
    {
        sizer->Add(new wxStaticBitmap(this, wxID_ANY, icon), wxSizerFlags().Centre().Border(wxRIGHT));
    }

    wxBoxSizer* const msgSizer = new wxBoxSizer(wxVERTICAL);
    for (const auto &msg : messages)
    {
        auto pMsg = new wxGenericStaticText(this, wxID_ANY, msg);
        pMsg->SetLabelMarkup(msg);
        msgSizer->Add(pMsg, wxSizerFlags().Left().Border(wxLEFT | wxRIGHT));
    }
    sizer->Add(msgSizer, wxSizerFlags().Centre().Border(wxALL));

    SetSizer(sizer);

    wxSize size = GetBestSize();
    constexpr double RADIUS = 5;
    wxGraphicsPath path = wxGraphicsRenderer::GetDefaultRenderer()->CreatePath();
    path.AddRoundedRectangle(0, 0, size.x, size.y, RADIUS);
    SetSize(size);
    SetShape(path);
    Layout();
}