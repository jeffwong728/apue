#ifndef SPAM_UI_MISC_HISTOGRAM_WIDGET_H
#define SPAM_UI_MISC_HISTOGRAM_WIDGET_H
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "wx/custombgwin.h"

class HistogramWidget : public wxCustomBackgroundWindow<wxControl>
{
public:
    HistogramWidget(wxWindow* parent);

protected:
    wxSize DoGetBestSize() const wxOVERRIDE { return wxSize(200, 100); }
    void OnPaint(wxPaintEvent&);

private:
    void SetBackgroundColours(wxColour colStart, wxColour colEnd);

private:
    std::vector<double> seqs_;

    wxDECLARE_NO_COPY_CLASS(HistogramWidget);
};
#endif //SPAM_UI_MISC_HISTOGRAM_WIDGET_H