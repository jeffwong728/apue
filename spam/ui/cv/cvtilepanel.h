#ifndef SPAM_UI_CV_CVTILEPANEL_H
#define SPAM_UI_CV_CVTILEPANEL_H
#include <wx/wxprec.h>
#include <wx/nativewin.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <string>
#include <opencv2/core/cvstd.hpp>

class CVTilePanel : public wxPanel
{
public:
    CVTilePanel(wxWindow* parent, const std::string &cvWinName, const wxString &panelName);
    ~CVTilePanel();

public:
    void AdjustImgWndSize(const int id, const wxSize &imgSize) const;

private:
    void OnChar(wxKeyEvent &e);
};
#endif //SPAM_UI_CV_CVTILEPANEL_H