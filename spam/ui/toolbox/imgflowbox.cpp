#include "imgflowbox.h"
#include <ui/spam.h>
#include <ui/procflow/flowchart.h>

ImgFlowBox::ImgFlowBox(wxWindow* parent)
: wxPanel(parent, kSpamID_TOOLPAGE_IMGFLOW)
, imgProcFlowChart_(nullptr)
{
    auto rootSizer = new wxBoxSizer(wxVERTICAL);
    rootSizer->AddSpacer(5);
    auto styleSizer = new wxFlexGridSizer(2, 2, 2);
    styleSizer->AddGrowableCol(1, 1);
    styleSizer->SetFlexibleDirection(wxHORIZONTAL);
    styleSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Image Processing Flow:")), wxSizerFlags().Left().CentreVertical().Border(wxLEFT));
    styleSizer->Add(new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, 0L), wxSizerFlags(1).Expand().HorzBorder());
    rootSizer->Add(styleSizer, wxSizerFlags(0).Expand());
    rootSizer->AddSpacer(5);

    auto chartSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Pipeline");
    chartSizer->Add(new FlowChart(this), wxSizerFlags(1).Expand());
    rootSizer->Add(chartSizer, wxSizerFlags(1).HorzBorder().Expand());
    rootSizer->AddSpacer(5);

    TransferDataToUI();

    SetSizer(rootSizer);
}

ImgFlowBox::~ImgFlowBox()
{
}

void ImgFlowBox::TransferDataToUI()
{
}

void ImgFlowBox::TransferDataFromUI()
{
}

void ImgFlowBox::OnColorChanged(wxColourPickerEvent &e)
{
    TransferDataFromUI();
}

void ImgFlowBox::OnStyleChanged(wxSpinEvent& e)
{
    TransferDataFromUI();
}