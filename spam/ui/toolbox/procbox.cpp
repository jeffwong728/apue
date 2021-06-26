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
#include <wx/valnum.h>

ProcBox::ProcBox(wxWindow* parent)
: ToolBox(parent, kSpamID_TOOLPAGE_PROC, wxT("Process"), std::vector<wxString>(), kSpamID_TOOLBOX_PROC_GUARD - kSpamID_TOOLBOX_PROC_ENHANCEMENT, kSpamID_TOOLBOX_PROC_ENHANCEMENT)
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
        Spam::GetBitmap(ip, std::string("proc.pyramid"))
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
    cv::split(srcImg, imgs_);

    RePopulateChannelChoice(static_cast<int>(imgs_.size()));
    RePopulateHistogramProfiles(imgs_, mask_);
    ReThreshold();
}

wxPanel *ProcBox::GetOptionPanel(const int toolIndex, wxWindow *parent)
{
    constexpr int numTools = kSpamID_TOOLBOX_PROC_GUARD - kSpamID_TOOLBOX_PROC_ENHANCEMENT;
    wxPanel *(ProcBox::*createOption[numTools])(wxWindow *parent) = 
    {
        nullptr,
        &ProcBox::CreateThresholdOption,
        nullptr,
        nullptr,
        &ProcBox::CreatePyramidOption
    };

    if (createOption[toolIndex])
    {
        return (this->*createOption[toolIndex])(parent);
    }

    return nullptr;
}

ToolOptions ProcBox::GetToolOptions() const
{
    ToolOptions tos;
    tos[cp_ToolProcPyramidLevel] = pyraLevel_;
    tos[cp_ToolProcThresholdMin] = minGray_;
    tos[cp_ToolProcThresholdMax] = maxGray_;
    tos[cp_ToolProcThresholdChannel] = channelChoice_ ? channelChoice_->GetSelection() : 0;
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
        Spam::GetSelectionFilter()->Clear();
        Spam::GetSelectionFilter()->SetEntitySelectionMode(SpamEntitySelectionMode::kESM_BOX_SINGLE);
        Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_GENERAL);
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

        ToolOptions tos = ProcBox::GetToolOptions();
        tos[cp_ToolId] = kSpamID_TOOLBOX_PROC_THRESHOLD;
        sig_OptionsChanged(tos);
    }
}

void ProcBox::OnThreshold(HistogramWidget *hist)
{
    ReThreshold();
}

void ProcBox::OnPyramidEnter(wxCommandEvent &evt)
{
    auto txtCtrl = dynamic_cast<wxTextCtrl *>(evt.GetEventObject());
    if (txtCtrl)
    {
        auto txtParent = txtCtrl->GetParent();
        if (txtParent)
        {
            if (txtParent->TransferDataFromWindow())
            {
                ToolOptions tos = ProcBox::GetToolOptions();
                tos[cp_ToolId] = kSpamID_TOOLBOX_PROC_PYRAMID;
                sig_OptionsChanged(tos);
            }
        }
    }
}

void ProcBox::OnThresholdEnter(wxCommandEvent &e)
{
    auto txtCtrl = dynamic_cast<wxTextCtrl *>(e.GetEventObject());
    if (txtCtrl)
    {
        auto txtParent = txtCtrl->GetParent();
        if (txtParent)
        {
            if (txtParent->TransferDataFromWindow())
            {
                ToolOptions tos = ProcBox::GetToolOptions();
                tos[cp_ToolId] = kSpamID_TOOLBOX_PROC_THRESHOLD;
                sig_OptionsChanged(tos);
            }
        }
    }
}

wxPanel *ProcBox::CreateThresholdOption(wxWindow *parent)
{
    auto panel = new wxScrolledWindow(parent, wxID_ANY);
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    hist_ = new HistogramWidget(panel);
    sizerRoot->Add(hist_, wxSizerFlags(0).Expand().Border());
    hist_->SetRangeX(std::make_pair(0, 255));

    wxIntegerValidator<int> minVal(&minGray_);
    wxIntegerValidator<int> maxVal(&maxGray_);
    minVal.SetRange(0, 255);
    maxVal.SetRange(1, 255);

    auto optSizer = new wxFlexGridSizer(2, 2, 2);
    auto minCtrl_ = new wxTextCtrl(panel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, minVal);
    auto maxCtrl_ = new wxTextCtrl(panel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, maxVal);
    channelChoice_ = new wxChoice(panel, wxID_ANY);
    optSizer->AddGrowableCol(1, 1);
    optSizer->SetFlexibleDirection(wxHORIZONTAL);
    optSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Channel:")), wxSizerFlags().Right().Border(wxLEFT));
    optSizer->Add(channelChoice_, wxSizerFlags(1).Expand().HorzBorder());
    optSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Threshold Min:")), wxSizerFlags().Right().Border(wxLEFT));
    optSizer->Add(minCtrl_, wxSizerFlags(1).Expand().HorzBorder());
    optSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Threshold Max:")), wxSizerFlags().Right().Border(wxLEFT));
    optSizer->Add(maxCtrl_, wxSizerFlags(1).Expand().HorzBorder());
    sizerRoot->Add(optSizer, wxSizerFlags().Expand());
    channelChoice_->Bind(wxEVT_CHOICE, &ProcBox::OnChannelChanged, this);
    minCtrl_->Bind(wxEVT_TEXT_ENTER, &ProcBox::OnThresholdEnter, this);
    maxCtrl_->Bind(wxEVT_TEXT_ENTER, &ProcBox::OnThresholdEnter, this);

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
    panel->TransferDataToWindow();
    return panel;
}

wxPanel *ProcBox::CreatePyramidOption(wxWindow *parent)
{
    auto panel = new wxScrolledWindow(parent, wxID_ANY);
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    wxIntegerValidator<int> levelVal(&pyraLevel_);
    levelVal.SetRange(2, 64);

    auto optSizer = new wxFlexGridSizer(2, 2, 2);
    auto levelCtrl = new wxTextCtrl(panel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, levelVal);
    optSizer->AddGrowableCol(1, 1);
    optSizer->SetFlexibleDirection(wxHORIZONTAL);
    optSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Pyramid Level:")), wxSizerFlags().Right().Border(wxTOP | wxLEFT));
    optSizer->Add(levelCtrl, wxSizerFlags(1).Expand().HorzBorder());
    sizerRoot->Add(optSizer, wxSizerFlags().Expand());
    levelCtrl->Bind(wxEVT_TEXT_ENTER, &ProcBox::OnPyramidEnter, this);

    panel->SetScrollRate(6, 6);
    panel->SetVirtualSize(panel->GetBestSize());
    panel->SetSizerAndFit(sizerRoot);
    panel->TransferDataToWindow();
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

    if (channelChoice_->GetCount() != numChannels)
    {
        channelChoice_->Clear();
        for (int c = 0; c < numChannels; ++c)
        {
            channelChoice_->AppendString(channelNames[c]);
        }
        channelChoice_->SetSelection(0);
    }
}

void ProcBox::RePopulateHistogramProfiles(const std::vector<cv::Mat> &imags, const cv::Mat &mask)
{
    int numChannels = static_cast<int>(imags.size());

    hist_->ClearProfiles();
    wxColor grayColor = SpamConfig::Get<bool>(cp_ThemeDarkMode, true) ? *wxWHITE : *wxBLACK;
    wxColor  colors[4] = { wxColour(0xCF9F72), *wxGREEN, *wxRED, grayColor };
    if (numChannels < 2)
    {
        colors[0] = grayColor;;
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

    hist_->SetPlane(channelChoice_->GetCurrentSelection());
    hist_->Refresh(true);
}

void ProcBox::ReThreshold()
{
    int selChannel = channelChoice_->GetSelection();
    if (selChannel>=0 && selChannel<static_cast<int>(imgs_.size()))
    {
        CairoCanvas *cav = Spam::FindCanvas(uuidStation_);
        if (cav)
        {
        }
    }
}