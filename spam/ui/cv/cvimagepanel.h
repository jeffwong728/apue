#ifndef SPAM_UI_CV_IMAGE_PANEL_H
#define SPAM_UI_CV_IMAGE_PANEL_H
#include <wx/wxprec.h>
#include <wx/nativewin.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <ui/cmndef.h>
#include <string>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
class DropImageEvent;
class CairoCanvas;

class CVImagePanel : public wxPanel
{
public:
    CVImagePanel(wxWindow* parent, const std::string &cvWinName, const wxString &panelName, const wxSize& size);
    ~CVImagePanel();

public:
    double LoadImageFromFile(const wxString &filePath);
    double SetImage(const cv::Mat &img);
    void BufferImage(const std::string &name);
    void AdjustImgWndSize(const int id, const wxSize &imgSize) const;
    double GetScale() const { return scale_; }
    void SetScale(const double scale, bool getFocus);
    double ZoomIn(bool getFocus);
    double ZoomOut(bool getFocus);
    double ZoomExtent(bool getFocus);
    double ZoomOriginal(bool getFocus);
    double ZoomHalf(bool getFocus);
    double ZoomDouble(bool getFocus);
    bool HasImage() const;
    const cv::Mat GetImage() const;
    cv::Mat GetImage();
    CairoCanvas *GetCanvas() const { return canv_; }

private:
    void OnChar(wxKeyEvent &e);
    void OnEnterScale(wxCommandEvent& e);
    void OnDropImage(DropImageEvent& e);
    void OnUpdateUI(wxUpdateUIEvent& e);

private:
    double scale_{100.0};
    CairoCanvas *canv_;
};
#endif //SPAM_UI_CV_IMAGE_PANEL_H