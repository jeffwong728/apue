#ifndef SPAM_UI_MISC_HISTOGRAM_WIDGET_H
#define SPAM_UI_MISC_HISTOGRAM_WIDGET_H
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class HistogramWidget : public wxControl
{
public:
    struct Profile
    {
        wxString label;
        wxColor  color;
        std::vector<double> seq;
    };

public:
    HistogramWidget(wxWindow* parent);

public:
    void ClearProfiles() { profiles_.clear(); }
    void AddProfile(Profile &&profile) { profiles_.push_back(profile); }

protected:
    wxSize DoGetBestSize() const wxOVERRIDE { return wxSize(128, 100); }
    void OnPaint(wxPaintEvent&);
    bool InheritsBackgroundColour()	const { return true; }

private:
    std::vector<Profile> profiles_;

    wxDECLARE_NO_COPY_CLASS(HistogramWidget);
};
#endif //SPAM_UI_MISC_HISTOGRAM_WIDGET_H