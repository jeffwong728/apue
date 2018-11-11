#ifndef SPAM_UI_CV_WXDC_CANVAS_H
#define SPAM_UI_CV_WXDC_CANVAS_H
#include <wx/wxprec.h>
#include <wx/vscroll.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <string>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>

class WXDCCanvas : public wxScrolledCanvas
{
public:
    WXDCCanvas(wxWindow* parent, const std::string &cvWinName);
    ~WXDCCanvas();

public:
    void ShowImage(const cv::Mat &img);
    void SetImageWndSize(const wxSize &s);

private:
    void OnSize(wxSizeEvent& event);
    void OnLeftMouseDown(wxMouseEvent &e);
    void OnChar(wxKeyEvent &e);
    void OnPaint(wxPaintEvent& e);
    void Draw(wxDC &dc);

private:
    cv::String cvWndName_;
    wxBitmap disMat_;
    wxImage disImg_;
};
#endif //SPAM_UI_CV_WXDC_CANVAS_H