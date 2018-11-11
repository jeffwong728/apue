#include "cvimagepanel.h"
#include "cairocanvas.h"
#include "wxdccanvas.h"
#include <wx/artprov.h>
#include <wx/valnum.h>
#include <opencv2/highgui.hpp>
#include <ui/misc/percentvalidator.h>
#include <ui/evts.h>

CVImagePanel::CVImagePanel(wxWindow* parent, const std::string &cvWinName, const wxString &panelName, const wxSize& size)
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER, panelName)
, canv_(nullptr)
{
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);
    canv_ = new CairoCanvas(this, cvWinName, panelName, size);
    sizerRoot->Add(canv_, 1, wxEXPAND, 0)->SetId(kSpamImageCanvas);

    Bind(wxEVT_CHAR, &CVImagePanel::OnChar, this, wxID_ANY);
    Bind(wxEVT_UPDATE_UI, &CVImagePanel::OnUpdateUI, this, kSpamID_SCALE_CHOICE, kSpamID_ZOOM_DOUBLE);
    wxTheApp->Bind(spamEVT_DROP_IMAGE, &CVImagePanel::OnDropImage, this, GetId());

    SetSizer(sizerRoot);
    GetSizer()->SetSizeHints(this);
    Show();
}

CVImagePanel::~CVImagePanel()
{
}

double CVImagePanel::LoadImageFromFile(const wxString &filePath)
{
    const std::string &ansiFilePath = filePath.ToStdString();
    auto img = cv::imread(cv::String(ansiFilePath.c_str()), cv::IMREAD_UNCHANGED);

    if (img.empty())
    {
        wxLogMessage(wxT("cv::imread error : %s"), filePath.wx_str());
    }
    else
    {
        scale_ = SetImage(img);
    }

    return scale_;
}

double CVImagePanel::SetImage(const cv::Mat &img)
{
    img_ = img;
    if (!img_.empty())
    {
        int dph = img_.depth();
        int cnl = img_.channels();

        wxString::const_pointer dphStr[CV_64F + 1] =
        {
            wxT("CV_8U"),
            wxT("CV_8S"),
            wxT("CV_16U"),
            wxT("CV_16S"),
            wxT("CV_32S"),
            wxT("CV_32F"),
            wxT("CV_64F")
        };

        wxLogMessage(wxT("channels : %d"), cnl);
        if (CV_8U == dph)
        {
            wxLogMessage(wxT("depth : %s"), dphStr[dph]);

            wxSizerItem* sizerItem = GetSizer()->GetItemById(kSpamImageCanvas);
            if (sizerItem)
            {
                auto cvWidget = dynamic_cast<CairoCanvas *>(sizerItem->GetWindow());
                if (cvWidget)
                {
                    wxSize cvImgSize{ img_.cols, img_.rows };
                    wxLogMessage(wxT("width : %d"), cvImgSize.GetX());
                    wxLogMessage(wxT("height : %d"), cvImgSize.GetY());
                    wxLogMessage(wxT("elemSize : %zd"), img_.elemSize());
                    wxLogMessage(wxT("elemSize1 : %zd"), img_.elemSize1());
                    wxLogMessage(wxT("step1 : %zd"), img_.step1());

                    cvWidget->ShowImage(img_);
                    double newScale = cvWidget->GetMatScale();
                    if (newScale > 0)
                    {
                        scale_ = newScale * 100;
                    }
                }
            }
        }
        else
        {
            wxLogMessage(wxT("Not supported depth : %s"), dphStr[dph]);
        }
    }

    return scale_;
}

void CVImagePanel::AdjustImgWndSize(const int id, const wxSize &imgSize) const
{
    wxSizerItem* sizerItem = GetSizer()->GetItemById(id);
    auto sizeDelta = sizerItem->GetSize()-sizerItem->GetWindow()->GetClientSize();
    sizerItem->SetMinSize(imgSize + sizeDelta);
    GetSizer()->Layout();
}

void CVImagePanel::SetScale(const double scale, bool getFocus)
{
    wxSizerItem* sizerItem = GetSizer()->GetItemById(kSpamImageCanvas);
    if (sizerItem)
    {
        auto cvWidget = dynamic_cast<CairoCanvas *>(sizerItem->GetWindow());
        if (cvWidget)
        {
            if (getFocus && !cvWidget->HasFocus())
            {
                cvWidget->SetFocus();
            }

            scale_ = scale;
            cvWidget->ScaleImage(scale_ / 100);
        }
    }
}

double CVImagePanel::ZoomIn(bool getFocus)
{
    wxSizerItem* sizerItem = GetSizer()->GetItemById(kSpamImageCanvas);
    if (sizerItem)
    {
        auto cvWidget = dynamic_cast<CairoCanvas *>(sizerItem->GetWindow());
        if (cvWidget)
        {
            if (getFocus && !cvWidget->HasFocus())
            {
                cvWidget->SetFocus();
            }

            cvWidget->ScaleUp(0.25);
            double newScale = cvWidget->GetMatScale();
            if (newScale > 0)
            {
                scale_ = newScale * 100;
            }
        }
    }

    return scale_;
}

double CVImagePanel::ZoomOut(bool getFocus)
{
    wxSizerItem* sizerItem = GetSizer()->GetItemById(kSpamImageCanvas);
    if (sizerItem)
    {
        auto cvWidget = dynamic_cast<CairoCanvas *>(sizerItem->GetWindow());
        if (cvWidget)
        {
            if (getFocus && !cvWidget->HasFocus())
            {
                cvWidget->SetFocus();
            }

            cvWidget->ScaleDown(0.25);
            double newScale = cvWidget->GetMatScale();
            if (newScale > 0)
            {
                scale_ = newScale * 100;
            }
        }
    }

    return scale_;
}

double CVImagePanel::ZoomExtent(bool getFocus)
{
    wxSizerItem* sizerItem = GetSizer()->GetItemById(kSpamImageCanvas);
    if (sizerItem)
    {
        auto cvWidget = dynamic_cast<CairoCanvas *>(sizerItem->GetWindow());
        if (cvWidget)
        {
            if (getFocus && !cvWidget->HasFocus())
            {
                cvWidget->SetFocus();
            }

            cvWidget->ExtentImage();
            double newScale = cvWidget->GetMatScale();
            if (newScale > 0)
            {
                scale_ = newScale * 100;
            }
        }
    }

    return scale_;
}

double CVImagePanel::ZoomOriginal(bool getFocus)
{
    wxSizerItem* sizerItem = GetSizer()->GetItemById(kSpamImageCanvas);
    if (sizerItem)
    {
        auto cvWidget = dynamic_cast<CairoCanvas *>(sizerItem->GetWindow());
        if (cvWidget)
        {
            if (getFocus && !cvWidget->HasFocus())
            {
                cvWidget->SetFocus();
            }

            scale_ = 100;
            cvWidget->ScaleImage(scale_ / 100);
        }
    }

    return scale_;
}

double CVImagePanel::ZoomHalf(bool getFocus)
{
    wxSizerItem* sizerItem = GetSizer()->GetItemById(kSpamImageCanvas);
    if (sizerItem)
    {
        auto cvWidget = dynamic_cast<CairoCanvas *>(sizerItem->GetWindow());
        if (cvWidget)
        {
            if (getFocus && !cvWidget->HasFocus())
            {
                cvWidget->SetFocus();
            }

            scale_ = 50;
            cvWidget->ScaleImage(scale_ / 100);
        }
    }

    return scale_;
}

double CVImagePanel::ZoomDouble(bool getFocus)
{
    wxSizerItem* sizerItem = GetSizer()->GetItemById(kSpamImageCanvas);
    if (sizerItem)
    {
        auto cvWidget = dynamic_cast<CairoCanvas *>(sizerItem->GetWindow());
        if (cvWidget)
        {
            if (getFocus && !cvWidget->HasFocus())
            {
                cvWidget->SetFocus();
            }

            scale_ = 200;
            cvWidget->ScaleImage(scale_ / 100);
        }
    }

    return scale_;
}

bool CVImagePanel::HasImage() const
{
    wxSizerItem* sizerItem = GetSizer()->GetItemById(kSpamImageCanvas);
    if (sizerItem)
    {
        auto cvWidget = dynamic_cast<CairoCanvas *>(sizerItem->GetWindow());
        if (cvWidget)
        {
            return cvWidget->HasImage();
        }
    }

    return false;
}

void CVImagePanel::OnChar(wxKeyEvent &e)
{
    wxString strC(static_cast<char>(e.GetKeyCode()), 1);
    strC.UpperCase();
    wxLogMessage(wxT("CVImagePanel::OnLeftMouseDown - ") + strC);
}

void CVImagePanel::OnEnterScale(wxCommandEvent& e)
{
    wxSizerItem* sizerItem = GetSizer()->GetItemById(kSpamImageToolBar);
    if (sizerItem)
    {
        auto tb = dynamic_cast<wxToolBar *>(sizerItem->GetWindow());
        if (tb)
        {
            if (!tb->TransferDataFromWindow())
            {
                tb->TransferDataToWindow();
            }
        }
    }

    sizerItem = GetSizer()->GetItemById(kSpamImageCanvas);
    if (sizerItem)
    {
        auto cvWidget = dynamic_cast<CairoCanvas *>(sizerItem->GetWindow());
        if (cvWidget)
        {
            if (!cvWidget->HasFocus())
            {
                cvWidget->SetFocus();
            }

            cvWidget->ScaleImage(scale_/100);
        }
    }
}

void CVImagePanel::OnDropImage(DropImageEvent& e)
{
    e.Skip();
}

void CVImagePanel::OnUpdateUI(wxUpdateUIEvent& e)
{
    if (e.GetId() <= kSpamID_ZOOM_DOUBLE && e.GetId() >= kSpamID_SCALE_CHOICE)
    {
        e.Enable(!img_.empty());
    }
}