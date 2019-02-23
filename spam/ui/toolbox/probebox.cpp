#include "probebox.h"
#include <ui/spam.h>
#include <ui/misc/spamutility.h>
#include <wx/artprov.h>
#include <wx/statline.h>
#include <wx/collpane.h>
#include <wx/tglbtn.h>
#include <wx/wxhtml.h>

ProbeBox::ProbeBox(wxWindow* parent)
: ToolBox(parent, kSpamID_TOOLPAGE_PROBE, wxT("Infomation"), std::vector<wxString>(), kSpamID_TOOLBOX_PROBE_GUARD - kSpamID_TOOLBOX_PROBE_SELECT, kSpamID_TOOLBOX_PROBE_SELECT)
{
    wxWindowID toolIds[] = {
        kSpamID_TOOLBOX_PROBE_SELECT,
        kSpamID_TOOLBOX_PROBE_HISTOGRAM
    };

    wxString   toolTips[] = {
        wxT("Select entities to show infomation"),
        wxT("Select entities to show histogram")
    };

    const SpamIconPurpose ip = kICON_PURPOSE_TOOLBOX;
    wxBitmap toolIcons[] = {
        Spam::GetBitmap(ip, bm_Pointer),
        Spam::GetBitmap(ip, bm_NodeEdit)
    };

    ToolBox::Init(toolIds, toolTips, toolIcons, WXSIZEOF(toolTips), 0, 0);
    sig_ToolEnter.connect(std::bind(&ProbeBox::OnToolEnter, this, std::placeholders::_1));
}

ProbeBox::~ProbeBox()
{
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
    wxColor  colors[4] = { *wxBLUE, *wxGREEN, *wxRED, *wxBLACK };
    if (imags.size() < 2)
    {
        colors[0] = *wxBLACK;
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

wxPanel *ProbeBox::GetOptionPanel(const int toolIndex, wxWindow *parent)
{
    constexpr int numTools = kSpamID_TOOLBOX_PROBE_GUARD - kSpamID_TOOLBOX_PROBE_SELECT;
    wxPanel *(ProbeBox::*createOption[numTools])(wxWindow *parent) = { &ProbeBox::CreateSelectOption, &ProbeBox::CreateHistOption };

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
    return tos;
}

void ProbeBox::OnProbeMode(wxCommandEvent &cmd)
{
    probeMode_ = cmd.GetId();
    UpdateSelectionFilter();

    ToolOptions tos = ProbeBox::GetToolOptions();
    tos[cp_ToolId] = kSpamID_TOOLBOX_PROBE_SELECT;
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

    case kSpamID_TOOLBOX_PROBE_HISTOGRAM:
        Spam::GetSelectionFilter()->ReplacePassType(SpamEntityType::kET_GEOM);
        Spam::GetSelectionFilter()->SetEntitySelectionMode(SpamEntitySelectionMode::kESM_BOX_SINGLE);
        Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_GENERAL);
        break;

    default:
        break;
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
    probeMode->ToggleTool(kSpamID_TOOLBOX_PROBE_PIXEL, true);
    probeMode->Bind(wxEVT_TOOL, &ProbeBox::OnProbeMode, this, kSpamID_TOOLBOX_PROBE_PIXEL, kSpamID_TOOLBOX_PROBE_IMAGE);
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