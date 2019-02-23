#include "procbox.h"
#include <ui/spam.h>
#include <ui/misc/spamutility.h>
#include <ui/proc/basic.h>
#include <ui/cv/cairocanvas.h>
#include <wx/artprov.h>
#include <wx/statline.h>
#include <wx/collpane.h>
#include <wx/tglbtn.h>
#include <wx/wxhtml.h>

ProcBox::ProcBox(wxWindow* parent)
: ToolBox(parent, kSpamID_TOOLPAGE_PROC, wxT("Process"), std::vector<wxString>(), kSpamID_TOOLBOX_PROC_GUARD - kSpamID_TOOLBOX_PROC_ENHANCEMENT, kSpamID_TOOLBOX_PROC_ENHANCEMENT)
, nameText_(nullptr)
, channelChoice_(nullptr)
, hist_(nullptr)
{
    wxWindowID toolIds[] = {
        kSpamID_TOOLBOX_PROC_ENHANCEMENT,
        kSpamID_TOOLBOX_PROC_THRESHOLD,
        kSpamID_TOOLBOX_PROC_FILTER,
        kSpamID_TOOLBOX_PROC_EDGE,
        kSpamID_TOOLBOX_PROC_PYRAMID
    };

    wxString   toolTips[] = {
        wxT("Image enhancement"),
        wxT("Image threshold"),
        wxT("Image filtering"),
        wxT("Image edge extraction"),
        wxT("Construct image pyramid")
    };

    const SpamIconPurpose ip = kICON_PURPOSE_TOOLBOX;
    wxBitmap toolIcons[] = {
        Spam::GetBitmap(ip, bm_Pointer),
        Spam::GetBitmap(ip, bm_Pointer),
        Spam::GetBitmap(ip, bm_Pointer),
        Spam::GetBitmap(ip, bm_Pointer),
        Spam::GetBitmap(ip, bm_Pointer)
    };

    ToolBox::Init(toolIds, toolTips, toolIcons, WXSIZEOF(toolTips), 0, 0);
    sig_ToolEnter.connect(std::bind(&ProcBox::OnToolEnter, this, std::placeholders::_1));
}

ProcBox::~ProcBox()
{
}

void ProcBox::UpdateHistogram(const std::string &uuidTag, const cv::Mat &srcImg, const boost::any &roi)
{
    const Geom::OptRect *rect = boost::any_cast<Geom::OptRect>(&roi);
    if (rect)
    {
        if (!rect->empty())
        {
            Geom::PathVector pv{ Geom::Path(**rect) };
            mask_ = SpamUtility::GetMaskFromPath(pv, cv::Size(srcImg.cols, srcImg.rows));
        }
    }
    else
    {
        const Geom::PathVector *pv = boost::any_cast<Geom::PathVector>(&roi);
        if (pv)
        {
            mask_ = SpamUtility::GetMaskFromPath(*pv, cv::Size(srcImg.cols, srcImg.rows));
        }
    }

    img_ = srcImg;
    uuidStation_ = uuidTag;
    roi_.clear();
    roi_.AddRun(mask_);
    cv::split(srcImg, imgs_);

    RePopulateHistogramProfiles(imgs_, mask_);
    RePopulateChannelChoice(static_cast<int>(imgs_.size()));
    ReThreshold();
}

wxPanel *ProcBox::GetOptionPanel(const int toolIndex, wxWindow *parent)
{
    constexpr int numTools = kSpamID_TOOLBOX_PROC_GUARD - kSpamID_TOOLBOX_PROC_ENHANCEMENT;
    wxPanel *(ProcBox::*createOption[numTools])(wxWindow *parent) = { nullptr, &ProcBox::CreateThresholdOption };

    if (createOption[toolIndex])
    {
        return (this->*createOption[toolIndex])(parent);
    }

    return nullptr;
}

ToolOptions ProcBox::GetToolOptions() const
{
    ToolOptions tos;
    return tos;
}

void ProcBox::OnToolEnter(const ToolOptions &toolOpts)
{
    const int toolId = boost::get<int>(toolOpts.at(cp_ToolId));
    switch (toolId)
    {
    case kSpamID_TOOLBOX_PROC_ENHANCEMENT:
    case kSpamID_TOOLBOX_PROC_THRESHOLD:
    case kSpamID_TOOLBOX_PROC_FILTER:
    case kSpamID_TOOLBOX_PROC_EDGE:
        Spam::GetSelectionFilter()->ReplacePassType(SpamEntityType::kET_GEOM);
        Spam::GetSelectionFilter()->SetEntitySelectionMode(SpamEntitySelectionMode::kESM_BOX_SINGLE);
        Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_GENERAL);
        break;

    case kSpamID_TOOLBOX_PROC_PYRAMID:
        break;

    default:
        break;
    }
}

void ProcBox::OnChannelChanged(wxCommandEvent& e)
{
    int sel = channelChoice_->GetSelection();
    if (sel != hist_->GetPlane())
    {
        hist_->SetPlane(sel);
        hist_->Refresh(true);
    }
}

void ProcBox::OnThreshold(HistogramWidget *hist)
{
    ReThreshold();
}

wxPanel *ProcBox::CreateThresholdOption(wxWindow *parent)
{
    auto panel = new wxScrolledWindow(parent, wxID_ANY);
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    hist_ = new HistogramWidget(panel);
    sizerRoot->Add(hist_, wxSizerFlags(0).Expand().Border());
    hist_->SetRangeX(std::make_pair(0, 255));
    hist_->AddThumb(60);
    hist_->AddThumb(200);
    hist_->sig_ThumbsMoved.connect(std::bind(&ProcBox::OnThreshold, this, std::placeholders::_1));

    auto optSizer = new wxFlexGridSizer(2, 2, 2);
    nameText_ = new wxTextCtrl(panel, wxID_ANY, wxT("binary"));
    channelChoice_ = new wxChoice(panel, wxID_ANY);
    optSizer->AddGrowableCol(1, 1);
    optSizer->SetFlexibleDirection(wxHORIZONTAL);
    optSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Channel:")), wxSizerFlags().Right().Border(wxLEFT));
    optSizer->Add(channelChoice_, wxSizerFlags(1).Expand().HorzBorder());
    optSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Name:")), wxSizerFlags().Right().Border(wxLEFT));
    optSizer->Add(nameText_, wxSizerFlags(1).Expand().HorzBorder());
    sizerRoot->Add(optSizer, wxSizerFlags().Expand());
    channelChoice_->Bind(wxEVT_CHOICE, &ProcBox::OnChannelChanged, this);

    auto helpPane = new wxCollapsiblePane(panel, wxID_ANY, wxT("Instructions"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
    helpPane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &ProcBox::OnHelpCollapse, this, wxID_ANY);
    sizerRoot->Add(helpPane, wxSizerFlags(1).Expand());

    wxWindow *win = helpPane->GetPane();
    auto helpSizer = new wxBoxSizer(wxVERTICAL);
    auto html = new wxHtmlWindow(win, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_NEVER);
    html->SetBorders(0);
    html->LoadPage(wxT("res/help/rect.htm"));
    html->SetInitialSize(wxSize(html->GetInternalRepresentation()->GetWidth(), html->GetInternalRepresentation()->GetHeight()));
    helpSizer->Add(html, wxSizerFlags(1).Expand().DoubleBorder());
    win->SetSizerAndFit(helpSizer);

    panel->SetScrollRate(6, 6);
    panel->SetVirtualSize(panel->GetBestSize());
    panel->SetSizerAndFit(sizerRoot);
    return panel;
}

void ProcBox::UpdateSelectionFilter(void)
{
}

void ProcBox::RePopulateChannelChoice(const int numChannels)
{
    wxString  channelNames[4] = { wxT("Blue"), wxT("Green"), wxT("Red"), wxT("Alpha") };
    if (numChannels < 2)
    {
        channelNames[0] = wxT("Gray Scale");
    }
    channelChoice_->Clear();
    for (int c = 0; c<numChannels; ++c)
    {
        channelChoice_->AppendString(channelNames[c]);
    }
    channelChoice_->SetSelection(0);
}

void ProcBox::RePopulateHistogramProfiles(const std::vector<cv::Mat> &imags, const cv::Mat &mask)
{
    int numChannels = static_cast<int>(imags.size());

    hist_->ClearProfiles();
    wxColor  colors[4] = { *wxBLUE, *wxGREEN, *wxRED, *wxBLACK };

    if (numChannels < 2)
    {
        colors[0] = *wxBLACK;
    }

    int c = 0;
    for (const cv::Mat &imag : imags)
    {
        const int channels[] = { 0 };
        const int histSize[] = { 256 };
        const float range[] = { 0, 256 };
        const float *ranges[] = { range };
        cv::Mat hist;
        cv::calcHist(&imag, 1, channels, mask, hist, 1, histSize, ranges, true, false);

        HistogramWidget::Profile profile{ wxT(""), colors[c++] };
        profile.seq.resize(256);
        std::copy(hist.ptr<float>(0, 0), hist.ptr<float>(0, 0) + 256, profile.seq.begin());
        hist_->AddProfile(std::move(profile));
    }

    hist_->SetPlane(0);
    hist_->Refresh(true);
}

void ProcBox::ReThreshold()
{
    int selChannel = channelChoice_->GetSelection();
    int minGray = hist_->GetThumbs()[0];
    int maxGray = hist_->GetThumbs()[1];
    cv::Mat grayImg = imgs_[selChannel];
    SPSpamRgn rgn = BasicImgProc::Threshold(grayImg, minGray, maxGray);

    CairoCanvas *cav = Spam::FindCanvas(uuidStation_);
    if (cav && nameText_)
    {
        SPSpamRgnVector rgns = std::make_shared<SpamRgnVector>(1);
        rgns->back().swap(*rgn);
        cav->PushRegionsIntoBufferZone(nameText_->GetValue().ToStdString(), rgns);
    }
}