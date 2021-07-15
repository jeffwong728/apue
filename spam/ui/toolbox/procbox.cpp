#include "procbox.h"
#include <ui/spam.h>
#include <ui/misc/spamutility.h>
#include <ui/imgproc/basic.h>
#include <ui/cv/cairocanvas.h>
#include <wx/artprov.h>
#include <wx/statline.h>
#include <wx/collpane.h>
#include <wx/tglbtn.h>
#include <wx/wxhtml.h>
#include <wx/valnum.h>

ProcBox::ProcBox(wxWindow* parent)
: ToolBox(parent, kSpamID_TOOLPAGE_PROC, wxT("Process"), std::vector<wxString>(), kSpamID_TOOLBOX_PROC_GUARD - kSpamID_TOOLBOX_PROC_ENHANCEMENT, kSpamID_TOOLBOX_PROC_ENHANCEMENT)
, edgeChannelChoice_(nullptr)
, threshChannelChoice_(nullptr)
, convertChannelChoice_(nullptr)
, hist_(nullptr)
{
    wxWindowID toolIds[] = {
        kSpamID_TOOLBOX_PROC_ENHANCEMENT,
        kSpamID_TOOLBOX_PROC_THRESHOLD,
        kSpamID_TOOLBOX_PROC_FILTER,
        kSpamID_TOOLBOX_PROC_EDGE,
        kSpamID_TOOLBOX_PROC_PYRAMID,
        kSpamID_TOOLBOX_PROC_CONVERT
    };

    wxString   toolTips[] = {
        wxT("Image enhancement"),
        wxT("Image threshold"),
        wxT("Image filtering"),
        wxT("Image edge extraction"),
        wxT("Construct image pyramid"),
        wxT("Converts an image from one color space to another")
    };

    const SpamIconPurpose ip = kICON_PURPOSE_TOOLBOX;
    wxBitmap toolIcons[] = {
        Spam::GetBitmap(ip, std::string("proc.enhancement")),
        Spam::GetBitmap(ip, std::string("proc.threshold")),
        Spam::GetBitmap(ip, std::string("proc.filter")),
        Spam::GetBitmap(ip, std::string("proc.edge")),
        Spam::GetBitmap(ip, std::string("proc.pyramid")),
        Spam::GetBitmap(ip, std::string("proc.rgb"))
    };

    ToolBox::Init(toolIds, toolTips, toolIcons, WXSIZEOF(toolTips), 0, 0);
    sig_ToolEnter.connect(std::bind(&ProcBox::OnToolEnter, this, std::placeholders::_1));

    iParams_[cp_ToolProcPyramidLevel] = 5;
    iParams_[cp_ToolProcThresholdMin] = 50;
    iParams_[cp_ToolProcThresholdMax] = 200;
    iParams_[cp_ToolProcFilterType] = 0;
    iParams_[cp_ToolProcFilterBorderType] = 0;
    iParams_[cp_ToolProcFilterBoxKernelWidth] = 5;
    iParams_[cp_ToolProcFilterBoxKernelHeight] = 5;
    iParams_[cp_ToolProcFilterGaussianKernelWidth] = 5;
    iParams_[cp_ToolProcFilterGaussianKernelHeight] = 5;
    fParams_[cp_ToolProcFilterGaussianSigmaX] = 1.5;
    fParams_[cp_ToolProcFilterGaussianSigmaY] = 1.5;
    iParams_[cp_ToolProcFilterMedianKernelWidth] = 5;
    iParams_[cp_ToolProcFilterMedianKernelHeight] = 5;
    iParams_[cp_ToolProcFilterBilateralDiameter] = 5;
    fParams_[cp_ToolProcFilterBilateralSigmaColor] = 1.5;
    fParams_[cp_ToolProcFilterBilateralSigmaSpace] = 1.5;
    iParams_[cp_ToolProcEdgeType] = 0;
    iParams_[cp_ToolProcEdgeChannel] = 0;
    iParams_[cp_ToolProcEdgeApertureSize] = 3;
    iParams_[cp_ToolProcEdgeCannyThresholdLow] = 10;
    iParams_[cp_ToolProcEdgeCannyThresholdHigh] = 25;
    iParams_[cp_ToolProcConvertChannel] = 0;
}

ProcBox::~ProcBox()
{
}

void ProcBox::UpdateThresholdUI(const std::string &uuidTag, const boost::any &roi)
{
    cv::Mat srcImg;
    uuidStation_ = uuidTag;
    CairoCanvas *cav = Spam::FindCanvas(uuidStation_);
    if (cav)
    {
        srcImg = cav->GetOriginalImage();
        cv::Mat mask;
        const Geom::OptRect *rect = boost::any_cast<Geom::OptRect>(&roi);
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

        std::vector<cv::Mat> imgs;
        cv::split(srcImg, imgs);
        if (3 == srcImg.channels())
        {
            cv::Mat grayMat;
            cv::cvtColor(srcImg, grayMat, cv::COLOR_BGR2GRAY);
            imgs.push_back(grayMat);
        }

        if (4 == srcImg.channels())
        {
            cv::Mat grayMat;
            cv::cvtColor(srcImg, grayMat, cv::COLOR_BGRA2GRAY);
            imgs.push_back(grayMat);
        }

        PopulateChannelChoice(threshChannelChoice_, srcImg.channels());
        PopulateHistogramProfiles(imgs, mask);
    }
}

void ProcBox::UpdateConvertUI(const std::string &uuidTag, const boost::any &roi)
{
    cv::Mat srcImg;
    uuidStation_ = uuidTag;
    CairoCanvas *cav = Spam::FindCanvas(uuidStation_);
    if (cav)
    {
        srcImg = cav->GetOriginalImage();
        if ((1 == srcImg.channels() && 1 == convertChannelChoice_->GetCount()) ||
            (3 == srcImg.channels() && 7 == convertChannelChoice_->GetCount()) ||
            (4 == srcImg.channels() && 8 == convertChannelChoice_->GetCount()))
        {
            return;
        }
        else
        {
            convertChannelChoice_->Clear();
        }

        switch (srcImg.channels())
        {
        case 1:
            convertChannelChoice_->Append(wxT("Gray"));
            break;

        case 3:
            convertChannelChoice_->Append(wxT("Blue"));
            convertChannelChoice_->Append(wxT("Green"));
            convertChannelChoice_->Append(wxT("Red"));
            convertChannelChoice_->Append(wxT("Gray"));
            convertChannelChoice_->Append(wxT("Hue"));
            convertChannelChoice_->Append(wxT("Lightness"));
            convertChannelChoice_->Append(wxT("Saturation"));
            break;

        case 4:
            convertChannelChoice_->Append(wxT("Blue"));
            convertChannelChoice_->Append(wxT("Green"));
            convertChannelChoice_->Append(wxT("Red"));
            convertChannelChoice_->Append(wxT("Alpha"));
            convertChannelChoice_->Append(wxT("Gray"));
            convertChannelChoice_->Append(wxT("Hue"));
            convertChannelChoice_->Append(wxT("Lightness"));
            convertChannelChoice_->Append(wxT("Saturation"));
            break;

        default:
            break;
        }
        convertChannelChoice_->Select(0);
    }
}

void ProcBox::UpdateEdgeUI(const std::string &uuidTag, const boost::any &roi)
{
    cv::Mat srcImg;
    uuidStation_ = uuidTag;
    CairoCanvas *cav = Spam::FindCanvas(uuidStation_);
    if (cav)
    {
        srcImg = cav->GetOriginalImage();
        PopulateChannelChoice(edgeChannelChoice_, static_cast<int>(srcImg.channels()));
    }
}

void ProcBox::UpdateUI(const int toolId, const std::string &uuidTag, const boost::any &params)
{
    switch (toolId)
    {
    case kSpamID_TOOLBOX_PROC_THRESHOLD: UpdateThresholdUI(uuidTag, params); break;
    case kSpamID_TOOLBOX_PROC_EDGE: UpdateEdgeUI(uuidTag, params); break;
    case kSpamID_TOOLBOX_PROC_CONVERT: UpdateConvertUI(uuidTag, params); break;
    default: break;
    }
}

wxPanel *ProcBox::GetOptionPanel(const int toolIndex, wxWindow *parent)
{
    constexpr int numTools = kSpamID_TOOLBOX_PROC_GUARD - kSpamID_TOOLBOX_PROC_ENHANCEMENT;
    wxPanel *(ProcBox::*createOption[numTools])(wxWindow *parent) = 
    {
        nullptr,
        &ProcBox::CreateThresholdOption,
        &ProcBox::CreateFilterOption,
        &ProcBox::CreateEdgeOption,
        &ProcBox::CreatePyramidOption,
        &ProcBox::CreateConvertOption,
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
    tos[cp_ToolProcPyramidLevel]                = iParams_.at(cp_ToolProcPyramidLevel);
    tos[cp_ToolProcThresholdMin]                = iParams_.at(cp_ToolProcThresholdMin);
    tos[cp_ToolProcThresholdMax]                = iParams_.at(cp_ToolProcThresholdMax);
    tos[cp_ToolProcThresholdChannel]            = std::max(0, threshChannelChoice_ ? threshChannelChoice_->GetSelection() : 0);
    tos[cp_ToolProcFilterType]                  = iParams_.at(cp_ToolProcFilterType);
    tos[cp_ToolProcFilterBorderType]            = iParams_.at(cp_ToolProcFilterBorderType);
    tos[cp_ToolProcFilterBoxKernelWidth]        = iParams_.at(cp_ToolProcFilterBoxKernelWidth);
    tos[cp_ToolProcFilterBoxKernelHeight]       = iParams_.at(cp_ToolProcFilterBoxKernelHeight);
    tos[cp_ToolProcFilterGaussianKernelWidth]   = iParams_.at(cp_ToolProcFilterGaussianKernelWidth);
    tos[cp_ToolProcFilterGaussianKernelHeight]  = iParams_.at(cp_ToolProcFilterGaussianKernelHeight);
    tos[cp_ToolProcFilterGaussianSigmaX]        = fParams_.at(cp_ToolProcFilterGaussianSigmaX);
    tos[cp_ToolProcFilterGaussianSigmaY]        = fParams_.at(cp_ToolProcFilterGaussianSigmaY);
    tos[cp_ToolProcFilterMedianKernelWidth]     = iParams_.at(cp_ToolProcFilterMedianKernelWidth);
    tos[cp_ToolProcFilterMedianKernelHeight]    = iParams_.at(cp_ToolProcFilterMedianKernelHeight);
    tos[cp_ToolProcFilterBilateralDiameter]     = iParams_.at(cp_ToolProcFilterBilateralDiameter);
    tos[cp_ToolProcFilterBilateralSigmaColor]   = fParams_.at(cp_ToolProcFilterBilateralSigmaColor);
    tos[cp_ToolProcFilterBilateralSigmaSpace]   = fParams_.at(cp_ToolProcFilterBilateralSigmaSpace);
    tos[cp_ToolProcEdgeType]                    = iParams_.at(cp_ToolProcEdgeType);
    tos[cp_ToolProcEdgeChannel]                 = std::max(0, edgeChannelChoice_ ? edgeChannelChoice_->GetSelection() : 0);
    tos[cp_ToolProcEdgeApertureSize]            = iParams_.at(cp_ToolProcEdgeApertureSize);
    tos[cp_ToolProcEdgeCannyThresholdLow]       = iParams_.at(cp_ToolProcEdgeCannyThresholdLow);
    tos[cp_ToolProcEdgeCannyThresholdHigh]      = iParams_.at(cp_ToolProcEdgeCannyThresholdHigh);
    tos[cp_ToolProcConvertChannel]              = std::max(0, convertChannelChoice_ ? convertChannelChoice_->GetSelection() : 0);

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
    case kSpamID_TOOLBOX_PROC_CONVERT:
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
    int sel = threshChannelChoice_->GetSelection();
    if (sel != hist_->GetPlane())
    {
        hist_->SetPlane(sel);
        hist_->Refresh(true);

        ToolOptions tos = ProcBox::GetToolOptions();
        tos[cp_ToolId] = kSpamID_TOOLBOX_PROC_THRESHOLD;
        sig_OptionsChanged(tos);
    }
}

void ProcBox::OnFilterTypeChanged(wxCommandEvent& e)
{
    auto filterTypeChoice = dynamic_cast<wxChoice *>(e.GetEventObject());
    if (filterTypeChoice)
    {
        iParams_[cp_ToolProcFilterType] = filterTypeChoice->GetSelection();
        for (auto cc = 0U; cc < filterTypeChoice->GetCount(); ++cc)
        {
            wxSizer *optSizer = reinterpret_cast<wxSizer *>(filterTypeChoice->GetClientData(cc));
            optSizer->Show(false);
        }

        wxSizer *currSizer = reinterpret_cast<wxSizer *>(filterTypeChoice->GetClientData(filterTypeChoice->GetSelection()));
        currSizer->Show(true);
        filterTypeChoice->GetParent()->GetSizer()->Layout();

        auto txtParent = filterTypeChoice->GetParent();
        if (txtParent)
        {
            if (txtParent->TransferDataFromWindow())
            {
                ToolOptions tos = ProcBox::GetToolOptions();
                tos[cp_ToolId] = kSpamID_TOOLBOX_PROC_FILTER;
                sig_OptionsChanged(tos);
            }
        }
    }
}

void ProcBox::OnFilterBorderTypeChanged(wxCommandEvent& e)
{
    auto typeChoice = dynamic_cast<wxChoice *>(e.GetEventObject());
    if (typeChoice)
    {
        switch (typeChoice->GetSelection())
        {
        case 0: iParams_[cp_ToolProcFilterBorderType] = cv::BORDER_DEFAULT; break;
        case 1: iParams_[cp_ToolProcFilterBorderType] = cv::BORDER_REPLICATE; break;
        case 2: iParams_[cp_ToolProcFilterBorderType] = cv::BORDER_REFLECT; break;
        case 3: iParams_[cp_ToolProcFilterBorderType] = cv::BORDER_WRAP; break;
        case 4: iParams_[cp_ToolProcFilterBorderType] = cv::BORDER_REFLECT_101; break;
        case 5: iParams_[cp_ToolProcFilterBorderType] = cv::BORDER_TRANSPARENT; break;
        case 6: iParams_[cp_ToolProcFilterBorderType] = cv::BORDER_ISOLATED; break;
        default: iParams_[cp_ToolProcFilterBorderType] = cv::BORDER_DEFAULT; break;
        }

        auto wParent = typeChoice->GetParent();
        if (wParent)
        {
            if (wParent->TransferDataFromWindow())
            {
                ToolOptions tos = ProcBox::GetToolOptions();
                tos[cp_ToolId] = kSpamID_TOOLBOX_PROC_FILTER;
                sig_OptionsChanged(tos);
            }
        }
    }
}

void ProcBox::OnFilterEnter(wxCommandEvent &e)
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
                tos[cp_ToolId] = kSpamID_TOOLBOX_PROC_FILTER;
                sig_OptionsChanged(tos);
            }
        }
    }
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

void ProcBox::OnEdgeEnter(wxCommandEvent &e)
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
                tos[cp_ToolId] = kSpamID_TOOLBOX_PROC_EDGE;
                sig_OptionsChanged(tos);
            }
        }
    }
}

void ProcBox::OnEdgeTypeChanged(wxCommandEvent &e)
{
    auto tCtrl = dynamic_cast<wxChoice *>(e.GetEventObject());
    if (tCtrl)
    {
        iParams_[cp_ToolProcEdgeType] = tCtrl->GetSelection();
        auto tParent = tCtrl->GetParent();
        if (tParent)
        {
            if (tParent->TransferDataFromWindow())
            {
                ToolOptions tos = ProcBox::GetToolOptions();
                tos[cp_ToolId] = kSpamID_TOOLBOX_PROC_EDGE;
                sig_OptionsChanged(tos);
            }
        }
    }
}

void ProcBox::OnEdgeChannelChanged(wxCommandEvent &e)
{
    auto tCtrl = dynamic_cast<wxChoice *>(e.GetEventObject());
    if (tCtrl)
    {
        iParams_[cp_ToolProcEdgeChannel] = tCtrl->GetSelection();
        auto tParent = tCtrl->GetParent();
        if (tParent)
        {
            if (tParent->TransferDataFromWindow())
            {
                ToolOptions tos = ProcBox::GetToolOptions();
                tos[cp_ToolId] = kSpamID_TOOLBOX_PROC_EDGE;
                sig_OptionsChanged(tos);
            }
        }
    }
}

void ProcBox::OnConvertChannelChanged(wxCommandEvent &e)
{
    auto cCtrl = dynamic_cast<wxChoice *>(e.GetEventObject());
    if (cCtrl)
    {
        iParams_[cp_ToolProcConvertChannel] = cCtrl->GetSelection();
        auto cParent = cCtrl->GetParent();
        if (cParent)
        {
            if (cParent->TransferDataFromWindow())
            {
                ToolOptions tos = ProcBox::GetToolOptions();
                tos[cp_ToolId] = kSpamID_TOOLBOX_PROC_CONVERT;
                sig_OptionsChanged(tos);
            }
        }
    }
}

wxPanel *ProcBox::CreateFilterOption(wxWindow *parent)
{
    auto panel = new wxScrolledWindow(parent, wxID_ANY);
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    wxIntegerValidator<int> boxKernelWidth(&iParams_[cp_ToolProcFilterBoxKernelWidth]);
    wxIntegerValidator<int> boxKernelHeight(&iParams_[cp_ToolProcFilterBoxKernelHeight]);
    wxIntegerValidator<int> gaussKernelWidth(&iParams_[cp_ToolProcFilterGaussianKernelWidth]);
    wxIntegerValidator<int> gaussKernelHeight(&iParams_[cp_ToolProcFilterGaussianKernelHeight]);
    wxFloatingPointValidator<double> gaussSigmaX(&fParams_[cp_ToolProcFilterGaussianSigmaX], wxNUM_VAL_NO_TRAILING_ZEROES);
    wxFloatingPointValidator<double> gaussSigmaY(&fParams_[cp_ToolProcFilterGaussianSigmaY], wxNUM_VAL_NO_TRAILING_ZEROES);
    wxIntegerValidator<int> medianKernelWidth(&iParams_[cp_ToolProcFilterMedianKernelWidth]);
    wxIntegerValidator<int> medianKernelHeight(&iParams_[cp_ToolProcFilterMedianKernelHeight]);
    wxIntegerValidator<int> biDiameter(&iParams_[cp_ToolProcFilterBilateralDiameter]);
    wxFloatingPointValidator<double> biSigmaColor(&fParams_[cp_ToolProcFilterBilateralSigmaColor], wxNUM_VAL_NO_TRAILING_ZEROES);
    wxFloatingPointValidator<double> biSigmaSpace(&fParams_[cp_ToolProcFilterBilateralSigmaSpace], wxNUM_VAL_NO_TRAILING_ZEROES);

    boxKernelWidth.SetRange(3, 999);
    boxKernelHeight.SetRange(3, 999);
    gaussKernelWidth.SetRange(3, 999);
    gaussKernelHeight.SetRange(3, 999);
    gaussSigmaX.SetRange(0.1, 999);
    gaussSigmaY.SetRange(0.1, 999);
    medianKernelWidth.SetRange(3, 999);
    medianKernelHeight.SetRange(3, 999);
    biDiameter.SetRange(2, 999);
    biSigmaColor.SetRange(0.1, 999);
    biSigmaSpace.SetRange(0.1, 999);

    std::vector<wxWindow *> editCtrls;
    auto filterTypeChoice = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, 0L);
    auto borderTypeChoice = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, 0L);
    wxFlexGridSizer *boxSizer = new wxFlexGridSizer(2, 2, 2);
    wxFlexGridSizer *gaussianSizer = new wxFlexGridSizer(2, 2, 2);
    wxFlexGridSizer *medianSizer = new wxFlexGridSizer(2, 2, 2);
    wxFlexGridSizer *bilateralSizer = new wxFlexGridSizer(2, 2, 2);
    filterTypeChoice->Append(wxT("Box"), boxSizer);
    filterTypeChoice->Append(wxT("Gaussian"), gaussianSizer);
    filterTypeChoice->Append(wxT("Median"), medianSizer);
    filterTypeChoice->Append(wxT("Bilateral"), bilateralSizer);
    filterTypeChoice->SetSelection(0);
    filterTypeChoice->Bind(wxEVT_CHOICE, &ProcBox::OnFilterTypeChanged, this);
    borderTypeChoice->AppendString(wxT("Default"));
    borderTypeChoice->AppendString(wxT("Replicate"));
    borderTypeChoice->AppendString(wxT("Reflect"));
    borderTypeChoice->AppendString(wxT("Wrap"));
    borderTypeChoice->AppendString(wxT("Reflect101"));
    borderTypeChoice->AppendString(wxT("Transparent"));
    borderTypeChoice->AppendString(wxT("Isolated"));
    borderTypeChoice->SetSelection(0);
    borderTypeChoice->Bind(wxEVT_CHOICE, &ProcBox::OnFilterBorderTypeChanged, this);

    auto filterTypeSizer = new wxFlexGridSizer(2, 2, 2);
    filterTypeSizer->AddGrowableCol(1, 1);
    filterTypeSizer->SetFlexibleDirection(wxHORIZONTAL);
    filterTypeSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Filter Type:")), wxSizerFlags().Left().CentreVertical().HorzBorder());
    filterTypeSizer->Add(filterTypeChoice, wxSizerFlags(1).Border(wxRIGHT).Expand());
    filterTypeSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Border Type:")), wxSizerFlags().Left().CentreVertical().HorzBorder());
    filterTypeSizer->Add(borderTypeChoice, wxSizerFlags(1).Border(wxRIGHT).Expand());
    sizerRoot->AddSpacer(3);
    sizerRoot->Add(filterTypeSizer, wxSizerFlags(0).Expand());
    sizerRoot->AddSpacer(9);
    sizerRoot->Add(boxSizer, wxSizerFlags(1).Expand());
    sizerRoot->Add(gaussianSizer, wxSizerFlags(1).Expand());
    sizerRoot->Add(medianSizer, wxSizerFlags(1).Expand());
    sizerRoot->Add(bilateralSizer, wxSizerFlags(1).Expand());

    boxSizer->AddGrowableCol(1, 1);
    boxSizer->SetFlexibleDirection(wxHORIZONTAL);
    boxSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Kernel Width:")), wxSizerFlags().Right().CentreVertical().Border(wxTOP | wxLEFT));
    editCtrls.push_back(boxSizer->Add(new wxTextCtrl(panel, wxID_ANY, wxT("5"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, boxKernelWidth), wxSizerFlags(1).Expand().HorzBorder())->GetWindow());
    boxSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Kernel Height:")), wxSizerFlags().Right().CentreVertical().Border(wxLEFT));
    editCtrls.push_back(boxSizer->Add(new wxTextCtrl(panel, wxID_ANY, wxT("5"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, boxKernelHeight), wxSizerFlags(1).Expand().HorzBorder())->GetWindow());
    boxSizer->Show(true);

    gaussianSizer->AddGrowableCol(1, 1);
    gaussianSizer->SetFlexibleDirection(wxHORIZONTAL);
    gaussianSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Kernel Width:")), wxSizerFlags().Right().CentreVertical().Border(wxTOP | wxLEFT));
    editCtrls.push_back(gaussianSizer->Add(new wxTextCtrl(panel, wxID_ANY, wxT("5"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, gaussKernelWidth), wxSizerFlags(1).Expand().HorzBorder())->GetWindow());
    gaussianSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Kernel Height:")), wxSizerFlags().Right().CentreVertical().Border(wxLEFT));
    editCtrls.push_back(gaussianSizer->Add(new wxTextCtrl(panel, wxID_ANY, wxT("5"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, gaussKernelHeight), wxSizerFlags(1).Expand().HorzBorder())->GetWindow());
    gaussianSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Sigma X:")), wxSizerFlags().Right().CentreVertical().Border(wxLEFT));
    editCtrls.push_back(gaussianSizer->Add(new wxTextCtrl(panel, wxID_ANY, wxT("1.5"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, gaussSigmaX), wxSizerFlags(1).Expand().HorzBorder())->GetWindow());
    gaussianSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Sigma Y:")), wxSizerFlags().Right().CentreVertical().Border(wxLEFT));
    editCtrls.push_back(gaussianSizer->Add(new wxTextCtrl(panel, wxID_ANY, wxT("1.5"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, gaussSigmaY), wxSizerFlags(1).Expand().HorzBorder())->GetWindow());
    gaussianSizer->Show(false);

    medianSizer->AddGrowableCol(1, 1);
    medianSizer->SetFlexibleDirection(wxHORIZONTAL);
    medianSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Kernel Size:")), wxSizerFlags().Right().CentreVertical().Border(wxTOP | wxLEFT));
    editCtrls.push_back(medianSizer->Add(new wxTextCtrl(panel, wxID_ANY, wxT("5"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, medianKernelWidth), wxSizerFlags(1).Expand().HorzBorder())->GetWindow());
    medianSizer->Show(false);

    bilateralSizer->AddGrowableCol(1, 1);
    bilateralSizer->SetFlexibleDirection(wxHORIZONTAL);
    bilateralSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Diameter:")), wxSizerFlags().Right().CentreVertical().Border(wxTOP | wxLEFT));
    editCtrls.push_back(bilateralSizer->Add(new wxTextCtrl(panel, wxID_ANY, wxT("5"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, biDiameter), wxSizerFlags(1).Expand().HorzBorder())->GetWindow());
    bilateralSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Sigma Color:")), wxSizerFlags().Right().CentreVertical().Border(wxLEFT));
    editCtrls.push_back(bilateralSizer->Add(new wxTextCtrl(panel, wxID_ANY, wxT("1.5"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, biSigmaColor), wxSizerFlags(1).Expand().HorzBorder())->GetWindow());
    bilateralSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Sigma Space:")), wxSizerFlags().Right().CentreVertical().Border(wxLEFT));
    editCtrls.push_back(bilateralSizer->Add(new wxTextCtrl(panel, wxID_ANY, wxT("1.5"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, biSigmaSpace), wxSizerFlags(1).Expand().HorzBorder())->GetWindow());
    bilateralSizer->Show(false);

    for (auto editCtrl : editCtrls)
    {
        editCtrl->Bind(wxEVT_TEXT_ENTER, &ProcBox::OnFilterEnter, this);
    }

    panel->SetScrollRate(6, 6);
    panel->SetVirtualSize(panel->GetBestSize());
    panel->SetSizerAndFit(sizerRoot);
    panel->TransferDataToWindow();
    return panel;
}

wxPanel *ProcBox::CreateThresholdOption(wxWindow *parent)
{
    auto panel = new wxScrolledWindow(parent, wxID_ANY);
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    hist_ = new HistogramWidget(panel);
    sizerRoot->Add(hist_, wxSizerFlags(0).Expand().Border());
    hist_->SetRangeX(std::make_pair(0, 255));

    wxIntegerValidator<int> minVal(&iParams_[cp_ToolProcThresholdMin]);
    wxIntegerValidator<int> maxVal(&iParams_[cp_ToolProcThresholdMax]);
    minVal.SetRange(0, 255);
    maxVal.SetRange(1, 255);

    auto optSizer = new wxFlexGridSizer(2, 2, 2);
    auto minCtrl_ = new wxTextCtrl(panel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, minVal);
    auto maxCtrl_ = new wxTextCtrl(panel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, maxVal);
    threshChannelChoice_ = new wxChoice(panel, wxID_ANY);
    optSizer->AddGrowableCol(1, 1);
    optSizer->SetFlexibleDirection(wxHORIZONTAL);
    optSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Channel:")), wxSizerFlags().Right().Border(wxLEFT));
    optSizer->Add(threshChannelChoice_, wxSizerFlags(1).Expand().HorzBorder());
    optSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Threshold Min:")), wxSizerFlags().Right().Border(wxLEFT));
    optSizer->Add(minCtrl_, wxSizerFlags(1).Expand().HorzBorder());
    optSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Threshold Max:")), wxSizerFlags().Right().Border(wxLEFT));
    optSizer->Add(maxCtrl_, wxSizerFlags(1).Expand().HorzBorder());
    sizerRoot->Add(optSizer, wxSizerFlags().Expand());
    threshChannelChoice_->Bind(wxEVT_CHOICE, &ProcBox::OnChannelChanged, this);
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

    wxIntegerValidator<int> levelVal(&iParams_[cp_ToolProcPyramidLevel]);
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

wxPanel *ProcBox::CreateEdgeOption(wxWindow *parent)
{
    auto panel = new wxScrolledWindow(parent, wxID_ANY);
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    wxIntegerValidator<int> apertureSize(&iParams_[cp_ToolProcEdgeApertureSize]);
    wxIntegerValidator<int> threshLow(&iParams_[cp_ToolProcEdgeCannyThresholdLow]);
    wxIntegerValidator<int> threshHigh(&iParams_[cp_ToolProcEdgeCannyThresholdHigh]);
    apertureSize.SetRange(1, 31);
    threshLow.SetRange(1, 255);
    threshHigh.SetRange(1, 255);

    auto optSizer = new wxFlexGridSizer(2, 2, 2);
    optSizer->AddGrowableCol(1, 1);
    optSizer->SetFlexibleDirection(wxHORIZONTAL);

    std::vector<wxWindow *> editCtrls;
    auto edgeTypeChoice = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, 0L);
    edgeTypeChoice->Append(wxT("Canny with Sobel"));
    edgeTypeChoice->Append(wxT("Canny with Scharr"));
    edgeTypeChoice->SetSelection(0);

    optSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Edge Type:")), wxSizerFlags().Right().CentreVertical().Border(wxLEFT));
    optSizer->Add(edgeTypeChoice, wxSizerFlags(1).Border(wxRIGHT).Expand());

    edgeChannelChoice_ = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, 0L);
    optSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Channel:")), wxSizerFlags().Right().CentreVertical().Border(wxLEFT));
    optSizer->Add(edgeChannelChoice_, wxSizerFlags(1).Border(wxRIGHT).Expand());
    edgeChannelChoice_->Append("Default");

    optSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Aperture Size:")), wxSizerFlags().Right().CentreVertical().Border(wxLEFT));
    editCtrls.push_back(optSizer->Add(new wxTextCtrl(panel, wxID_ANY, wxT("3"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, apertureSize), wxSizerFlags(1).Expand().Border(wxRIGHT))->GetWindow());
    optSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Threshold Low:")), wxSizerFlags().Right().CentreVertical().Border(wxLEFT));
    editCtrls.push_back(optSizer->Add(new wxTextCtrl(panel, wxID_ANY, wxT("10"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, threshLow), wxSizerFlags(1).Expand().Border(wxRIGHT))->GetWindow());
    optSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Threshold High:")), wxSizerFlags().Right().CentreVertical().Border(wxLEFT));
    editCtrls.push_back(optSizer->Add(new wxTextCtrl(panel, wxID_ANY, wxT("25"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, threshHigh), wxSizerFlags(1).Expand().Border(wxRIGHT))->GetWindow());

    for (auto editCtrl : editCtrls)
    {
        editCtrl->Bind(wxEVT_TEXT_ENTER, &ProcBox::OnEdgeEnter, this);
    }

    edgeTypeChoice->Bind(wxEVT_CHOICE, &ProcBox::OnEdgeTypeChanged, this);
    edgeChannelChoice_->Bind(wxEVT_CHOICE, &ProcBox::OnEdgeChannelChanged, this);

    sizerRoot->Add(optSizer, wxSizerFlags().Expand());
    panel->SetScrollRate(6, 6);
    panel->SetVirtualSize(panel->GetBestSize());
    panel->SetSizerAndFit(sizerRoot);
    panel->TransferDataToWindow();

    return panel;
}

wxPanel *ProcBox::CreateConvertOption(wxWindow *parent)
{
    auto panel = new wxScrolledWindow(parent, wxID_ANY);
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    auto optSizer = new wxFlexGridSizer(2, 2, 2);
    optSizer->AddGrowableCol(1, 1);
    optSizer->SetFlexibleDirection(wxHORIZONTAL);

    convertChannelChoice_ = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, 0L);
    optSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Select Channel:")), wxSizerFlags().Right().CentreVertical().Border(wxLEFT));
    optSizer->Add(convertChannelChoice_, wxSizerFlags(1).Border(wxRIGHT).Expand());
    convertChannelChoice_->Bind(wxEVT_CHOICE, &ProcBox::OnConvertChannelChanged, this);

    sizerRoot->Add(optSizer, wxSizerFlags().Expand());
    panel->SetScrollRate(6, 6);
    panel->SetVirtualSize(panel->GetBestSize());
    panel->SetSizerAndFit(sizerRoot);
    panel->TransferDataToWindow();

    return panel;
}

void ProcBox::UpdateSelectionFilter(void)
{
}

void ProcBox::PopulateChannelChoice(wxChoice *choiceCtrl, const int numChannels)
{
    int cChannels = numChannels;
    wxString  channelNames[5] = { wxT("Blue"), wxT("Green"), wxT("Red"), wxT("Alpha"), wxT("Gray Scale") };
    if (1 == numChannels)
    {
        channelNames[0] = wxT("Gray Scale");
    }
    else
    {
        cChannels += 1;
    }

    if (3 == numChannels)
    {
        channelNames[3] = wxT("Gray Scale");
    }

    if (choiceCtrl && choiceCtrl->GetCount() != cChannels)
    {
        choiceCtrl->Clear();
        for (int c = 0; c < cChannels; ++c)
        {
            choiceCtrl->AppendString(channelNames[c]);
        }
        choiceCtrl->SetSelection(0);
    }
}

void ProcBox::PopulateHistogramProfiles(const std::vector<cv::Mat> &imags, const cv::Mat &mask)
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

    hist_->SetPlane(threshChannelChoice_->GetCurrentSelection());
    hist_->Refresh(true);
}
