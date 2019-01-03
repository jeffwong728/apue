#include "histwidget.h"
#include <wx/artprov.h>
#include <wx/display.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>

HistogramWidget::HistogramWidget(wxWindow* parent)
{
    wxControl::Create(parent, wxID_ANY);
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    Bind(wxEVT_PAINT, &HistogramWidget::OnPaint, this, wxID_ANY);
}

void HistogramWidget::OnPaint(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(this);
    PrepareDC(dc);

    wxRect cRect = GetClientRect();
    dc.DrawRectangle(cRect);
}

void HistogramWidget::SetBackgroundColours(wxColour colStart, wxColour colEnd)
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