#ifndef SPAM_UI_MISC_HISTOGRAM_WIDGET_H
#define SPAM_UI_MISC_HISTOGRAM_WIDGET_H
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/dcgraph.h>
#include <boost/signals2.hpp>

class HistogramWidget : public wxControl
{
public:
    struct Profile
    {
        wxString label;
        wxColor  color;
        std::vector<double> seq;
    };

    struct CursorData
    {
        double  x;
        double  y;
        wxPoint pos;
        wxColor color;
    };

public:
    typedef boost::signals2::keywords::mutex_type<boost::signals2::dummy_mutex> bs2_dummy_mutex;
    boost::signals2::signal_type<void(HistogramWidget *), bs2_dummy_mutex>::type sig_ThumbsMoved;

public:
    HistogramWidget(wxWindow* parent);

public:
    void ClearThumbs() { thumbs_.clear(); }
    void ClearProfiles();
    void AddThumb(const int t) { thumbs_.push_back(t); }
    void AddProfile(Profile &&profile) { profiles_.push_back(profile); }
    void SetRangeX(const std::pair<int, int> &r);
    int  GetPlane() const { return plane_; }
    void SetPlane(const int plane) { if (plane>=0 && plane<static_cast<int>(profiles_.size())) plane_ = plane; }
    const std::vector<int> &GetThumbs() const { return thumbs_; }

protected:
    void OnEnterWindow(wxMouseEvent &e);
    void OnLeaveWindow(wxMouseEvent &e);
    void OnLeftMouseDown(wxMouseEvent &e);
    void OnLeftMouseUp(wxMouseEvent &e);
    void OnMouseMotion(wxMouseEvent &e);
    void OnPaint(wxPaintEvent &e);

protected:
    bool InheritsBackgroundColour() const { return true; }
    wxSize DoGetBestSize() const wxOVERRIDE { return wxSize(150, 120); }

private:
    void DrawBackground(wxGCDC &dc) const;
    void DrawHistogram(wxGCDC &dc, const wxPoint *point=nullptr) const;
    void DrawCursors(wxGCDC &dc, const std::vector<CursorData> &curDatas) const;
    void DrawThumbs(wxGCDC &dc) const;
    void SmoothProfile(std::vector<wxPoint> &pts) const;

private:
    bool cursorVisible_{ false };
    int dragingThumb_{ -1 };
    int highlightThumb_{ -1 };
    int gapX_{5};
    int gapY_{5};
    int plane_{0};
    std::pair<int, int> rangeX_;
    mutable std::vector<Profile> profiles_;
    std::vector<int> thumbs_;

    wxDECLARE_NO_COPY_CLASS(HistogramWidget);
};
#endif //SPAM_UI_MISC_HISTOGRAM_WIDGET_H
