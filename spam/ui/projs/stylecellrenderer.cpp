#include "stylecellrenderer.h"
#include <wx/graphics.h>
#include <memory>

wxString StyleCellRenderer::GetDefaultType()
{
    wxVariant var;
    var << DrawStyle();
    return var.GetType();
}

bool StyleCellRenderer::Render(wxRect rect, wxDC *dc, int WXUNUSED(state))
{
    wxBrush brush(drawStyle_.fillColor_);
    wxPen   pen(drawStyle_.strokeColor_, static_cast<int>(drawStyle_.strokeWidth_));

    wxGraphicsContext *gc = wxGraphicsContext::CreateFromUnknownDC(*dc);
    if (gc)
    {
        std::unique_ptr<wxGraphicsContext> ugc(gc);
        ugc->SetPen(pen);
        ugc->SetBrush(brush);

        rect.Deflate(2);
        wxDouble w = rect.GetWidth();
        wxDouble h = rect.GetHeight();
        wxDouble cx = rect.GetX() + w/2;
        wxDouble cy = rect.GetY() + h/2;
        if (w<h)
        {
            h = w;
        }
        else
        {
            w = h;
        }
   
        ugc->DrawRectangle(cx-w/2, cy-h/2, w, h);
    }

    return true;
}

bool StyleCellRenderer::ActivateCell(const wxRect& cell,
    wxDataViewModel *model,
    const wxDataViewItem &item,
    unsigned int col,
    const wxMouseEvent *mouseEvent)
{
    return true;
}

wxSize StyleCellRenderer::GetSize() const
{
    return wxSize(24, -1);
}

bool StyleCellRenderer::SetValue(const wxVariant &value)
{
    if (!value.IsNull())
    {
        drawStyle_ << value;
    }
 
    return true;
}

bool StyleCellRenderer::GetValue(wxVariant &value) const
{
    value << drawStyle_;
    return true;
}