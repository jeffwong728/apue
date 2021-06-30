#include "probebox.h"
#include <wx/artprov.h>
#include <wx/statline.h>
#include <wx/collpane.h>
#include <wx/tglbtn.h>
#include <wx/wxhtml.h>
#include <wx/valnum.h>
#include <opencv2/mvlab.hpp>
#include <ui/spam.h>
#include <ui/misc/spamutility.h>
#include <ui/cv/cairocanvas.h>

ProbeBox::ProbeBox(wxWindow* parent)
: ToolBox(parent, kSpamID_TOOLPAGE_PROBE, wxT("Infomation"), std::vector<wxString>(), kSpamID_TOOLBOX_PROBE_GUARD - kSpamID_TOOLBOX_PROBE_SELECT, kSpamID_TOOLBOX_PROBE_SELECT)
, hist_(nullptr)
, profile_(nullptr)
, profileChoice_(nullptr)
, profileWidth_(1.0f)
, profileSigma_(0.4f)
{
    wxWindowID toolIds[] = {
        kSpamID_TOOLBOX_PROBE_SELECT,
        kSpamID_TOOLBOX_PROBE_REGION,
        kSpamID_TOOLBOX_PROBE_HISTOGRAM,
        kSpamID_TOOLBOX_PROBE_PROFILE
    };

    wxString   toolTips[] = {
        wxT("Select entities to show infomation"),
        wxT("Select regions to show infomation"),
        wxT("Select entities to show histogram"),
        wxT("Select or draw line to show intensity profile")
    };

    const SpamIconPurpose ip = kICON_PURPOSE_TOOLBOX;
    wxBitmap toolIcons[] = {
        Spam::GetBitmap(ip, std::string("probe.image")),
        Spam::GetBitmap(ip, std::string("probe.geometry")),
        Spam::GetBitmap(ip, std::string("probe.histogram")),
        Spam::GetBitmap(ip, std::string("probe.profile"))
    };

    ToolBox::Init(toolIds, toolTips, toolIcons, WXSIZEOF(toolTips), 0, 0);
    sig_ToolEnter.connect(std::bind(&ProbeBox::OnToolEnter, this, std::placeholders::_1));
}

ProbeBox::~ProbeBox()
{
}

void ProbeBox::UpdateProfile(const cv::Mat &srcImg, const Geom::Point &sPt, const Geom::Point &ePt)
{
    const Geom::Point vec = ePt - sPt;
    const Geom::Coord len = vec.length();
    if (profile_)
    {
        if (len > 3)
        {
            imags_.clear();
            if (srcImg.channels() > 1)
            {
                cv::split(srcImg, imags_);
            }
            else
            {
                imags_.push_back(srcImg);
            }

            begPoint_ = sPt;
            endPoint_ = ePt;
            RePopulateProfileChoice(srcImg.channels());

            wxColor grayColor = SpamConfig::Get<bool>(cp_ThemeDarkMode, true) ? *wxWHITE : *wxBLACK;
            wxColor colors[4] = { wxColour(0xCF9F72), *wxGREEN, *wxRED, grayColor };
            if (srcImg.channels() < 2)
            {
                colors[0] = grayColor;
            }

            cv::Mat img = imags_[profileChoice_->GetSelection()];
            cv::Point2f pt1{ static_cast<float>(begPoint_.x()), static_cast<float>(begPoint_.y()) };
            cv::Point2f pt2{ static_cast<float>(endPoint_.x()), static_cast<float>(endPoint_.y()) };
            cv::RotatedRect box((pt1 + pt2) / 2, cv::Size2f(len, profileWidth_), Geom::deg_from_rad(std::atan2(vec.y(), vec.x())));
            cv::Ptr<cv::mvlab::MeasureBox> spMB = cv::mvlab::MeasureBox::Gen(box, cv::Point2f(1.f, 1.f));
            spMB->SetSigma(profileSigma_);
            if (spMB->Valid())
            {
                HistogramWidget::Profile profile{ wxT(""), colors[profileChoice_->GetSelection()] };
                if (cv::mvlab::MLR_SUCCESS == spMB->GetProfile(img, profile.seq))
                {
                    profile_->ClearProfiles();
                    profile_->AddProfile(std::move(profile));
                    profile_->Refresh(true);
                }
            }
        }
        else
        {
            profile_->ClearProfiles();
            profile_->Refresh(true);
        }
    }
}

void ProbeBox::UpdateHistogram(const cv::Mat &srcImg, const boost::any &roi)
{
    const Geom::OptRect *rect = boost::any_cast<Geom::OptRect>(&roi);
    cv::Mat mask;
    if (rect)
    {
        if (!rect->empty())
        {
            Geom::PathVector pv{ Geom::Path(**rect) };
            mask = SpamUtility::GetMaskFromPath(pv, cv::Size(srcImg.cols, srcImg.rows));
        }
    }
    else
    {
        const Geom::PathVector *pv = boost::any_cast<Geom::PathVector>(&roi);
        if (pv)
        {
            mask = SpamUtility::GetMaskFromPath(*pv, cv::Size(srcImg.cols, srcImg.rows));
        }
    }

    std::vector<cv::Mat> imags;
    cv::split(srcImg, imags);
    hist_->ClearProfiles();
    wxColor  colors[4] = { wxColour(0xCF9F72), *wxGREEN, *wxRED, *wxLIGHT_GREY };
    if (imags.size() < 2)
    {
        colors[0] = *wxLIGHT_GREY;
    }

    int c = 0;
    for (const cv::Mat &imag : imags)
    {
        const int channels[] = {0};
        const int histSize[] = { 256 };
        const float range[] = {0, 256};
        const float *ranges[] = { range };
        cv::Mat hist;
        cv::calcHist(&imag, 1, channels, mask, hist, 1, histSize, ranges, true, false);

        HistogramWidget::Profile profile{wxT(""), colors[c++]};
        profile.seq.resize(256);
        std::copy(hist.ptr<float>(0, 0), hist.ptr<float>(0, 0)+256, profile.seq.begin());
        hist_->AddProfile(std::move(profile));
    }
    hist_->Refresh(true);
}

void ProbeBox::UpdateUI(const int toolId, const std::string &uuidTag, const boost::any &params)
{
    if (kSpamID_TOOLBOX_PROBE_HISTOGRAM == toolId)
    {
        cv::Mat srcImg;
        CairoCanvas *cav = Spam::FindCanvas(uuidTag);
        if (cav)
        {
            srcImg = cav->GetOriginalImage();
            UpdateHistogram(srcImg, params);
        }
    }
}

wxPanel *ProbeBox::GetOptionPanel(const int toolIndex, wxWindow *parent)
{
    constexpr int numTools = kSpamID_TOOLBOX_PROBE_GUARD - kSpamID_TOOLBOX_PROBE_SELECT;
    wxPanel *(ProbeBox::*createOption[numTools])(wxWindow *parent) = 
    { 
        &ProbeBox::CreateSelectOption,
        &ProbeBox::CreateRegionOption,
        &ProbeBox::CreateHistOption,
        &ProbeBox::CreateProfileOption
    };

    if (createOption[toolIndex])
    {
        return (this->*createOption[toolIndex])(parent);
    }

    return nullptr;
}

ToolOptions ProbeBox::GetToolOptions() const
{
    ToolOptions tos;
    tos[cp_ToolProbeMode] = probeMode_;
    tos[cp_ToolProbeRegionMask] = regionProbeMask_;
    return tos;
}

void ProbeBox::OnProbeEntity(wxCommandEvent &cmd)
{
    probeMode_ = cmd.GetId();
    UpdateSelectionFilter();

    ToolOptions tos = ProbeBox::GetToolOptions();
    tos[cp_ToolId] = kSpamID_TOOLBOX_PROBE_SELECT;
    sig_OptionsChanged(tos);
}

void ProbeBox::OnProbeRegion(wxCommandEvent &cmd)
{
    void (ProbeBox::*flagOp)(const RegionFeatureFlag) = cmd.IsChecked() ? &ProbeBox::SetFeature : &ProbeBox::ClearFeature;
    switch (cmd.GetId())
    {
    case kSpamID_TOOLBOX_PROBE_REGION_AREA: (this->*flagOp)(RegionFeatureFlag::kRFF_AREA); break;
    case kSpamID_TOOLBOX_PROBE_REGION_CIRCULARITY: (this->*flagOp)(RegionFeatureFlag::kRFF_CIRCULARITY); break;
    case kSpamID_TOOLBOX_PROBE_REGION_CONVEXITY: (this->*flagOp)(RegionFeatureFlag::kRFF_CONVEXITY); break;
    case kSpamID_TOOLBOX_PROBE_REGION_BBOX: (this->*flagOp)(RegionFeatureFlag::kRFF_RECT1); break;
    case kSpamID_TOOLBOX_PROBE_REGION_CENTROID: (this->*flagOp)(RegionFeatureFlag::kRFF_CENTROID); break;
    case kSpamID_TOOLBOX_PROBE_REGION_CONVEX: (this->*flagOp)(RegionFeatureFlag::kRFF_CONVEX_HULL); break;
    case kSpamID_TOOLBOX_PROBE_REGION_DIAMETER: (this->*flagOp)(RegionFeatureFlag::kRFF_DIAMETER); break;
    case kSpamID_TOOLBOX_PROBE_REGION_SMALLESTRECT: (this->*flagOp)(RegionFeatureFlag::kRFF_RECT2); break;
    case kSpamID_TOOLBOX_PROBE_REGION_SMALLESTCIRCLE: (this->*flagOp)(RegionFeatureFlag::kRFF_SMALLEST_CIRCLE); break;
    case kSpamID_TOOLBOX_PROBE_REGION_ORIENTATION: (this->*flagOp)(RegionFeatureFlag::kRFF_ORIENTATION); break;
    case kSpamID_TOOLBOX_PROBE_REGION_ELLIPTIC_AXIS: (this->*flagOp)(RegionFeatureFlag::kRFF_ELLIPTIC_AXIS); break;
    default: break;
    }
    ToolOptions tos = ProbeBox::GetToolOptions();
    tos[cp_ToolId] = kSpamID_TOOLBOX_PROBE_REGION;
    sig_OptionsChanged(tos);
}

void ProbeBox::OnToolEnter(const ToolOptions &toolOpts)
{
    const int toolId = boost::get<int>(toolOpts.at(cp_ToolId));
    switch (toolId)
    {
    case kSpamID_TOOLBOX_PROBE_SELECT:
        UpdateSelectionFilter();
        break;

    case kSpamID_TOOLBOX_PROBE_REGION:
        Spam::GetSelectionFilter()->ReplacePassType(SpamEntityType::kET_REGION);
        Spam::GetSelectionFilter()->SetEntitySelectionMode(SpamEntitySelectionMode::kESM_MULTIPLE);
        Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_REGION_PROBE);
        break;

    case kSpamID_TOOLBOX_PROBE_HISTOGRAM:
        Spam::GetSelectionFilter()->ReplacePassType(SpamEntityType::kET_GEOM);
        Spam::GetSelectionFilter()->SetEntitySelectionMode(SpamEntitySelectionMode::kESM_BOX_SINGLE);
        Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_GENERAL);
        break;

    case kSpamID_TOOLBOX_PROBE_PROFILE:
        Spam::GetSelectionFilter()->Clear();
        Spam::GetSelectionFilter()->SetEntitySelectionMode(SpamEntitySelectionMode::kESM_NONE);
        Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_GENERAL);
        break;

    default:
        break;
    }
}

void ProbeBox::OnProfileChanged(wxCommandEvent &WXUNUSED(e))
{
    const Geom::Point vec = endPoint_ - begPoint_;
    const Geom::Coord len = vec.length();
    if (profile_ && !imags_.empty())
    {
        if (len > 3)
        {
            wxColor grayColor = SpamConfig::Get<bool>(cp_ThemeDarkMode, true) ? *wxWHITE : *wxBLACK;
            wxColor colors[4] = { wxColour(0xCF9F72), *wxGREEN, *wxRED, grayColor };
            if (imags_.size() < 2)
            {
                colors[0] = grayColor;
            }

            cv::Mat img = imags_[profileChoice_->GetSelection()];
            cv::Point2f pt1{ static_cast<float>(begPoint_.x()), static_cast<float>(begPoint_.y()) };
            cv::Point2f pt2{ static_cast<float>(endPoint_.x()), static_cast<float>(endPoint_.y()) };
            cv::RotatedRect box((pt1 + pt2) / 2, cv::Size2f(len, profileWidth_), Geom::deg_from_rad(std::atan2(vec.y(), vec.x())));
            cv::Ptr<cv::mvlab::MeasureBox> spMB = cv::mvlab::MeasureBox::Gen(box, cv::Point2f(1.f, 1.f));
            spMB->SetSigma(profileSigma_);
            if (spMB->Valid())
            {
                HistogramWidget::Profile profile{ wxT(""), colors[profileChoice_->GetSelection()] };
                if (cv::mvlab::MLR_SUCCESS == spMB->GetProfile(img, profile.seq))
                {
                    profile_->ClearProfiles();
                    profile_->AddProfile(std::move(profile));
                    profile_->Refresh(true);
                }
            }
        }
        else
        {
            profile_->ClearProfiles();
            profile_->Refresh(true);
        }
    }
}

void ProbeBox::OnEnter(wxCommandEvent &evt)
{
    auto txtCtrl = dynamic_cast<wxTextCtrl *>(evt.GetEventObject());
    if (txtCtrl)
    {
        auto txtParent = txtCtrl->GetParent();
        if (txtParent)
        {
            if (txtParent->TransferDataFromWindow())
            {
                OnProfileChanged(evt);
            }
        }
    }
}

wxPanel *ProbeBox::CreateSelectOption(wxWindow *parent)
{
    auto panel = new wxScrolledWindow(parent, wxID_ANY);
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    const SpamIconPurpose ip = kICON_PURPOSE_TOOLBOX;
    auto probeMode = new wxToolBar(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxTB_TEXT | wxTB_HORZ_TEXT | wxTB_NODIVIDER);
    probeMode->AddRadioTool(kSpamID_TOOLBOX_PROBE_PIXEL,  wxT("Probe pixel"),  Spam::GetBitmap(ip, bm_Pointer));
    probeMode->AddRadioTool(kSpamID_TOOLBOX_PROBE_ENTITY, wxT("Probe entity"), Spam::GetBitmap(ip, bm_Pointer));
    probeMode->AddRadioTool(kSpamID_TOOLBOX_PROBE_IMAGE,  wxT("Probe image"),  Spam::GetBitmap(ip, bm_Pointer));
    probeMode->ToggleTool(kSpamID_TOOLBOX_PROBE_PIXEL, true); SetFeature(RegionFeatureFlag::kRFF_AREA);
    probeMode->Bind(wxEVT_TOOL, &ProbeBox::OnProbeEntity, this, kSpamID_TOOLBOX_PROBE_PIXEL, kSpamID_TOOLBOX_PROBE_IMAGE);
    probeMode->Realize();

    sizerRoot->Add(probeMode, wxSizerFlags(0).Expand().Border());

    auto helpPane = new wxCollapsiblePane(panel, wxID_ANY, wxT("Instructions"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
    helpPane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &ProbeBox::OnHelpCollapse, this, wxID_ANY);
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

wxPanel *ProbeBox::CreateRegionOption(wxWindow *parent)
{
    auto panel = new wxScrolledWindow(parent, wxID_ANY);
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    const SpamIconPurpose ip = kICON_PURPOSE_TOOLBOX;
    auto probeMode = new wxToolBar(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxTB_TEXT | wxTB_HORZ_TEXT | wxTB_NODIVIDER);
    probeMode->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT));
    probeMode->AddCheckTool(kSpamID_TOOLBOX_PROBE_REGION_AREA, wxT("Probe Area"), Spam::GetBitmap(ip, bm_Pointer));
    probeMode->AddCheckTool(kSpamID_TOOLBOX_PROBE_REGION_CIRCULARITY, wxT("Probe Circularity"), Spam::GetBitmap(ip, bm_Pointer));
    probeMode->AddCheckTool(kSpamID_TOOLBOX_PROBE_REGION_CONVEXITY, wxT("Probe Convexity"), Spam::GetBitmap(ip, bm_Pointer));
    probeMode->AddCheckTool(kSpamID_TOOLBOX_PROBE_REGION_CENTROID, wxT("Probe Centroid"), Spam::GetBitmap(ip, bm_Pointer));
    probeMode->AddCheckTool(kSpamID_TOOLBOX_PROBE_REGION_BBOX, wxT("Probe Bounding Box"), Spam::GetBitmap(ip, bm_Pointer));
    probeMode->AddCheckTool(kSpamID_TOOLBOX_PROBE_REGION_CONVEX, wxT("Probe Convex Hull"), Spam::GetBitmap(ip, bm_Pointer));
    probeMode->AddCheckTool(kSpamID_TOOLBOX_PROBE_REGION_DIAMETER, wxT("Probe Diameter"), Spam::GetBitmap(ip, bm_Pointer));
    probeMode->AddCheckTool(kSpamID_TOOLBOX_PROBE_REGION_SMALLESTRECT, wxT("Probe Smallest Rectangle"), Spam::GetBitmap(ip, bm_Pointer));
    probeMode->AddCheckTool(kSpamID_TOOLBOX_PROBE_REGION_SMALLESTCIRCLE, wxT("Probe Smallest Circle"), Spam::GetBitmap(ip, bm_Pointer));
    probeMode->AddCheckTool(kSpamID_TOOLBOX_PROBE_REGION_ORIENTATION, wxT("Probe Orientation"), Spam::GetBitmap(ip, bm_Pointer));
    probeMode->AddCheckTool(kSpamID_TOOLBOX_PROBE_REGION_ELLIPTIC_AXIS, wxT("Probe Elliptic Axis"), Spam::GetBitmap(ip, bm_Pointer));
    probeMode->ToggleTool(kSpamID_TOOLBOX_PROBE_REGION_AREA, true);
    probeMode->Bind(wxEVT_TOOL, &ProbeBox::OnProbeRegion, this, kSpamID_TOOLBOX_PROBE_REGION_AREA, kSpamID_TOOLBOX_PROBE_REGION_ELLIPTIC_AXIS);
    probeMode->Realize();

    sizerRoot->Add(probeMode, wxSizerFlags(0).Expand().Border());

    auto helpPane = new wxCollapsiblePane(panel, wxID_ANY, wxT("Instructions"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
    helpPane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &ProbeBox::OnHelpCollapse, this, wxID_ANY);
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

wxPanel *ProbeBox::CreateHistOption(wxWindow *parent)
{
    auto panel = new wxScrolledWindow(parent, wxID_ANY);
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    hist_ = new HistogramWidget(panel);
    sizerRoot->Add(hist_, wxSizerFlags(0).Expand().Border());

    auto helpPane = new wxCollapsiblePane(panel, wxID_ANY, wxT("Instructions"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
    helpPane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &ProbeBox::OnHelpCollapse, this, wxID_ANY);
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

wxPanel *ProbeBox::CreateProfileOption(wxWindow *parent)
{
    auto panel = new wxScrolledWindow(parent, wxID_ANY);
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    profile_ = new HistogramWidget(panel);
    sizerRoot->Add(profile_, wxSizerFlags(0).Expand().Border());

    wxFloatingPointValidator<float> sigmVal(2, &profileSigma_, wxNUM_VAL_ZERO_AS_BLANK | wxNUM_VAL_NO_TRAILING_ZEROES);
    wxFloatingPointValidator<float> widthVal(2, &profileWidth_, wxNUM_VAL_ZERO_AS_BLANK | wxNUM_VAL_NO_TRAILING_ZEROES);
    sigmVal.SetRange(0.f, 100.f);
    widthVal.SetRange(0.01f, 9999999.f);

    auto styleSizer = new wxFlexGridSizer(2, 2, 2);
    styleSizer->AddGrowableCol(1, 1);
    styleSizer->SetFlexibleDirection(wxHORIZONTAL);
    profileChoice_ = new wxChoice(panel, wxID_ANY);
    auto sigmCtrl = new wxTextCtrl(panel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, sigmVal);
    auto widthCtrl = new wxTextCtrl(panel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, widthVal);
    styleSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Channel:")), wxSizerFlags().Right().Border(wxLEFT));
    styleSizer->Add(profileChoice_, wxSizerFlags(1).Expand().HorzBorder());
    styleSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Width:")), wxSizerFlags().Right().Border(wxLEFT));
    styleSizer->Add(widthCtrl, wxSizerFlags(1).Expand().HorzBorder());
    styleSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Sigma:")), wxSizerFlags().Right().Border(wxLEFT));
    styleSizer->Add(sigmCtrl, wxSizerFlags(1).Expand().HorzBorder());
    sizerRoot->Add(styleSizer, wxSizerFlags().Expand());
    profileChoice_->Bind(wxEVT_CHOICE, &ProbeBox::OnProfileChanged, this);
    sigmCtrl->Bind(wxEVT_TEXT_ENTER, &ProbeBox::OnEnter, this);
    widthCtrl->Bind(wxEVT_TEXT_ENTER, &ProbeBox::OnEnter, this);

    auto helpPane = new wxCollapsiblePane(panel, wxID_ANY, wxT("Instructions"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
    helpPane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &ProbeBox::OnHelpCollapse, this, wxID_ANY);
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
    panel->TransferDataToWindow();
    return panel;
}

void ProbeBox::UpdateSelectionFilter(void)
{
    switch (probeMode_)
    {
    case kSpamID_TOOLBOX_PROBE_PIXEL:
        Spam::GetSelectionFilter()->Clear();
        break;

    case kSpamID_TOOLBOX_PROBE_ENTITY:
        Spam::GetSelectionFilter()->AddAllPassType();
        break;

    case kSpamID_TOOLBOX_PROBE_IMAGE:
        Spam::GetSelectionFilter()->Clear();
        break;

    default:
        break;
    }

    if (kSpamID_TOOLBOX_PROBE_ENTITY == probeMode_)
    {
        Spam::GetSelectionFilter()->SetEntitySelectionMode(SpamEntitySelectionMode::kESM_SINGLE);
    }
    else
    {
        Spam::GetSelectionFilter()->SetEntitySelectionMode(SpamEntitySelectionMode::kESM_NONE);
    }

    Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_GENERAL);
}

void ProbeBox::RePopulateProfileChoice(const int numChannels)
{
    if (numChannels != profileChoice_->GetCount())
    {
        wxString  channelNames[4] = { wxT("Blue"), wxT("Green"), wxT("Red"), wxT("Alpha") };
        if (numChannels < 2)
        {
            channelNames[0] = wxT("Gray Scale");
        }
        profileChoice_->Clear();
        for (int c = 0; c < numChannels; ++c)
        {
            profileChoice_->AppendString(channelNames[c]);
        }
        profileChoice_->SetSelection(0);
    }
}
