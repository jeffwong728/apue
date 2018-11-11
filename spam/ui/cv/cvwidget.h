#ifndef SPAM_UI_CV_CVWIDGET_H
#define SPAM_UI_CV_CVWIDGET_H
#include <wx/wxprec.h>
#include <wx/nativewin.h>
#include <wx/vscroll.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <string>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>

class CVWidget : public wxScrolledWindow
{
public:
    CVWidget(wxWindow* parent, const std::string &cvWinName);
    ~CVWidget();

public:
    void ShowImage(const cv::Mat &img);
    wxSize GetImageWndSize();
    void SetImageWndSize(const wxSize &s);

private:
    void CreateCVWindow(void);
    void CloseCVWindow(void);
    void OnSize(wxSizeEvent& event);
    void OnLeftMouseDown(wxMouseEvent &e);
    void OnChar(wxKeyEvent &e);

private:
    wxNativeWindow cvWxWnd_;
    WXWidget cvNativeParentWnd_;
    WXWidget cvNativeChildWnd_;
    cv::String cvWndName_;
};
#endif //SPAM_UI_CV_CVWIDGET_H