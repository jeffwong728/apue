#include "cvwidget.h"
#include <wx/wxhtml.h>
#include <wx/graphics.h>
#include <opencv2/highgui.hpp>
#include <algorithm>

CVWidget::CVWidget(wxWindow* parent, const std::string &cvWinName)
: wxScrolledWindow(parent, wxID_ANY, wxPoint(0, 0), wxDefaultSize)
, cvWndName_(cvWinName.c_str())
{
    CreateCVWindow();
    Connect(wxEVT_SIZE, wxSizeEventHandler(CVWidget::OnSize));
    Bind(wxEVT_CHAR, &CVWidget::OnChar, this, wxID_ANY);
}

CVWidget::~CVWidget()
{
    CloseCVWindow();
}

void CVWidget::ShowImage(const cv::Mat &img)
{
    cv::imshow(cvWndName_, img);
}

wxSize CVWidget::GetImageWndSize()
{
    cv::Rect imgWndSize = cv::getWindowImageRect(cvWndName_);
    return wxSize(imgWndSize.width, imgWndSize.height);
}

void CVWidget::SetImageWndSize(const wxSize &s)
{
    cvWxWnd_.SetSize(s);
    cv::resizeWindow(cvWndName_, s.GetWidth(), s.GetHeight());
    SetVirtualSize(s);
    SetScrollRate(6, 6);
}

void CVWidget::CreateCVWindow(void)
{
    cv::namedWindow(cvWndName_, cv::WINDOW_NORMAL);
    cv::setWindowProperty(cvWndName_, cv::WND_PROP_VISIBLE, -1.0);

    cvNativeChildWnd_ = static_cast<WXWidget>(cvGetWindowHandle(cvWndName_.c_str()));
    cvNativeParentWnd_ = ::GetParent(static_cast<wxNativeWindowHandle>(cvNativeChildWnd_));

    wxNativeWindow cvChildWnd;
    wxNativeWindow cvParentWnd;

    cvChildWnd.AssociateHandle(cvNativeChildWnd_);
    cvChildWnd.Reparent(this);
    cvChildWnd.DissociateHandle();

    cvParentWnd.AssociateHandle(cvNativeParentWnd_);
    cvParentWnd.Hide();
    cvParentWnd.DissociateHandle();

    cvWxWnd_.Create(this, wxID_ANY, cvNativeChildWnd_);
    cvWxWnd_.Bind(wxEVT_LEFT_DOWN, &CVWidget::OnLeftMouseDown, this, wxID_ANY);
}

void CVWidget::CloseCVWindow(void)
{
    wxNativeWindow cvChildWnd;
    wxNativeWindow cvParentWnd;

    cvWxWnd_.DissociateHandle();
    cvParentWnd.AssociateHandle(cvNativeParentWnd_);
    cvChildWnd.AssociateHandle(cvNativeChildWnd_);

    cvChildWnd.Reparent(&cvParentWnd);

    cvParentWnd.RemoveChild(&cvChildWnd);
    cvChildWnd.DissociateHandle();
    cvParentWnd.DissociateHandle();

    cv::destroyWindow(cvWndName_);
}

void CVWidget::OnSize(wxSizeEvent& event)
{
    auto cSize = GetClientSize();
    //cv::resizeWindow(cvWndName_, cSize.GetWidth(), cSize.GetHeight());
}

void CVWidget::OnLeftMouseDown(wxMouseEvent &e)
{
    if (!HasFocus())
    {
        SetFocus();
    }
 
    auto pt = e.GetPosition();
    wxLogMessage(wxT("CVWidget::OnLeftMouseDown (x=%d, y=%d)"), pt.x, pt.y);

    wxGraphicsRenderer *renders[] =
    {
        wxGraphicsRenderer::GetDefaultRenderer(),
        wxGraphicsRenderer::GetCairoRenderer(),
        wxGraphicsRenderer::GetGDIPlusRenderer(),
        wxGraphicsRenderer::GetDirect2DRenderer()
    };

    wxString::const_pointer strs[] =
    {
        wxT("DefaultRenderer"),
        wxT("CairoRenderer"),
        wxT("GDIPlusRenderer"),
        wxT("Direct2DRenderer")
    };

    std::array<int, 4> indices = { 0,1,2,3 };

    for (auto i : indices)
    {
        if (renders[i])
        {
            wxLogMessage(wxT("%s : %s"), strs[i], renders[i]->GetName().wx_str());
        }
        else
        {
            wxLogMessage(wxT("%s : not existed"), strs[i]);
        }
    }
}

void CVWidget::OnChar(wxKeyEvent &e)
{
    wxString strC(static_cast<char>(e.GetKeyCode()), 1);
    strC.UpperCase();
    wxLogMessage(wxT("CVWidget::OnChar - ")+strC);
}