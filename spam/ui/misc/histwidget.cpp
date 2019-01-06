#include "histwidget.h"
#include <wx/artprov.h>
#include <wx/display.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <algorithm>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/2geom.h>
#pragma warning( pop )

HistogramWidget::HistogramWidget(wxWindow* parent)
{
    wxControl::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    InheritAttributes();

    Bind(wxEVT_PAINT, &HistogramWidget::OnPaint, this, wxID_ANY);
}

void HistogramWidget::OnPaint(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(this);
    PrepareDC(dc);

    wxColour backgroundColour = GetBackgroundColour();
    dc.SetBrush(wxBrush(backgroundColour));
    wxRect cRect = GetClientRect();
    dc.DrawRectangle(cRect);

    int width = cRect.GetWidth();
    int height = cRect.GetHeight();

    for (const Profile &profile : profiles_)
    {
        if (profile.seq.size()>1)
        {
            auto mmIt = std::minmax_element(profile.seq.cbegin(), profile.seq.cend());
            double minVal = *mmIt.first;
            double maxVal = *mmIt.second;
            double yInter = maxVal - minVal;

            std::vector<wxPoint> points;
            if (yInter>0.1)
            {
                for (int n=0; n<profile.seq.size(); ++n)
                {
                    int x = wxRound(n / (profile.seq.size() - 1.0) * width);
                    int y = wxRound((1-(profile.seq[n] - minVal) / yInter) * height);
                    points.push_back(wxPoint(x, y));
                }
            }
            else
            {
                for (int n = 0; n<profile.seq.size(); ++n)
                {
                    int x = wxRound(n / (profile.seq.size() - 1.0) * width);
                    int y = wxRound(height / 2.0);
                    points.push_back(wxPoint(x, y));
                }
            }

            dc.SetPen(wxPen(profile.color));
            dc.DrawLines(static_cast<int>(points.size()), points.data());
        }
    }
}