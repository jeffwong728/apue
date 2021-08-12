#include "imgflowbox.h"
#include <wx/imaglist.h>
#include <wx/listctrl.h>
#include <wx/dnd.h>
#include <ui/spam.h>
#include <ui/procflow/flowchart.h>

ImgFlowBox::ImgFlowBox(wxWindow* parent)
: wxPanel(parent, kSpamID_TOOLPAGE_IMGFLOW)
, imgProcFlowChart_(nullptr)
, toolsList_(nullptr)
{
    auto catChoices = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, 0L);
    catChoices->Append(wxT("Basic"));
    catChoices->Append(wxT("Filter"));
    catChoices->Append(wxT("Morphology"));
    catChoices->Append(wxT("Measure"));
    catChoices->Append(wxT("Matching"));
    catChoices->Select(0);

    basicToolImages_ = std::make_unique<wxImageList>(32, 32, true);
    basicToolImages_->Add(Spam::GetBitmap(kICON_PURPOSE_BIG, std::string("probe.image")));
    basicToolImages_->Add(Spam::GetBitmap(kICON_PURPOSE_BIG, std::string("proc.resizing")));
    basicToolImages_->Add(Spam::GetBitmap(kICON_PURPOSE_BIG, std::string("proc.rgb")));

    toolsList_ = new wxListView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_ICON | wxLC_NO_HEADER | wxLC_SINGLE_SEL);
    toolsList_->SetImageList(basicToolImages_.get(), wxIMAGE_LIST_NORMAL);
    toolsList_->InsertItem(0, wxT("Start"), 0);
    toolsList_->InsertItem(1, wxT("End"), 1);
    toolsList_->InsertItem(2, wxT("Convert"), 2);
    toolsList_->Bind(wxEVT_LIST_BEGIN_DRAG, &ImgFlowBox::OnDragBegin, this);

    auto rootSizer = new wxBoxSizer(wxVERTICAL);
    auto styleSizer = new wxFlexGridSizer(2, 2, 2);
    styleSizer->AddGrowableCol(1, 1);
    styleSizer->SetFlexibleDirection(wxHORIZONTAL);
    styleSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Flow Chart:")), wxSizerFlags().Left().CentreVertical().Border(wxLEFT));
    styleSizer->Add(new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, 0L), wxSizerFlags(1).Expand().HorzBorder());
    styleSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Category:")), wxSizerFlags().Left().CentreVertical().Border(wxLEFT));
    styleSizer->Add(catChoices, wxSizerFlags(1).Expand().HorzBorder());

    auto chartSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Pipeline"));
    auto toolBar = new wxToolBar(chartSizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxTB_NODIVIDER);
    toolBar->SetMargins(0, 0);
    toolBar->SetToolPacking(0);
    toolBar->SetToolSeparation(0);
    toolBar->SetToolBitmapSize(wxSize(22, 22));
    toolBar->AddTool(kSpamID_TOOLBOX_FLOWCHART_ADD, wxT("Add"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.add")));
    toolBar->AddTool(kSpamID_TOOLBOX_FLOWCHART_RUN, wxT("Run"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.play")));
    toolBar->AddSeparator();
    toolBar->AddRadioTool(kSpamID_TOOLBOX_FLOWCHART_POINTER, wxT("Select and Edit Steps"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.pointer.edit")));
    toolBar->AddRadioTool(kSpamID_TOOLBOX_FLOWCHART_CONNECT, wxT("Connect Steps"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.pointer.connect")));
    toolBar->AddSeparator();
    toolBar->AddTool(kSpamID_TOOLBOX_FLOWCHART_ALIGN_LEFT, wxT("Align Left"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.align.left")));
    toolBar->AddTool(kSpamID_TOOLBOX_FLOWCHART_ALIGN_TOP, wxT("Align Top"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.align.top")));
    toolBar->AddTool(kSpamID_TOOLBOX_FLOWCHART_ALIGN_BOTTOM, wxT("Align Bottom"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.align.bottom")));
    toolBar->AddTool(kSpamID_TOOLBOX_FLOWCHART_ALIGN_RIGHT, wxT("Align Right"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.align.right")));
    toolBar->AddTool(kSpamID_TOOLBOX_FLOWCHART_ALIGN_HCENTER, wxT("Align Horizontal Center"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.align.hcenter")));
    toolBar->AddTool(kSpamID_TOOLBOX_FLOWCHART_ALIGN_VCENTER, wxT("Align Vertical Center"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.align.vcenter")));
    toolBar->ToggleTool(kSpamID_TOOLBOX_FLOWCHART_POINTER, true);
    toolBar->Bind(wxEVT_TOOL, &ImgFlowBox::OnFlowChartAdd, this, kSpamID_TOOLBOX_FLOWCHART_ADD);
    toolBar->Bind(wxEVT_TOOL, &ImgFlowBox::OnFlowChartRun, this, kSpamID_TOOLBOX_FLOWCHART_RUN);
    toolBar->Bind(wxEVT_TOOL, &ImgFlowBox::OnFlowChartAlign, this, kSpamID_TOOLBOX_FLOWCHART_ALIGN_LEFT, kSpamID_TOOLBOX_FLOWCHART_ALIGN_VCENTER);
    toolBar->Bind(wxEVT_TOOL, &ImgFlowBox::OnFlowChartMode, this, kSpamID_TOOLBOX_FLOWCHART_POINTER, kSpamID_TOOLBOX_FLOWCHART_CONNECT);

    toolBar->Realize();

    imgProcFlowChart_ = new FlowChart(chartSizer->GetStaticBox());
    chartSizer->Add(imgProcFlowChart_, wxSizerFlags(1).Expand());
    chartSizer->Add(toolBar, wxSizerFlags(0).Expand());

    rootSizer->AddSpacer(5);
    rootSizer->Add(styleSizer, wxSizerFlags(0).Expand());
    rootSizer->AddSpacer(5);
    rootSizer->Add(toolsList_, wxSizerFlags(0).HorzBorder().Expand());
    rootSizer->AddSpacer(5);
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

void ImgFlowBox::OnDragBegin(wxListEvent &e)
{
    wxString stepTypes[] = { wxT("initstep"), wxT("endstep") , wxT("cvtstep") };
    wxDropSource dragSource(toolsList_);
    wxTextDataObject my_data(stepTypes[e.GetIndex()]);
    dragSource.SetData(my_data);
    dragSource.DoDragDrop(wxDrag_CopyOnly);
}

void ImgFlowBox::OnColorChanged(wxColourPickerEvent &e)
{
    TransferDataFromUI();
}

void ImgFlowBox::OnStyleChanged(wxSpinEvent& e)
{
    TransferDataFromUI();
}

void ImgFlowBox::OnFlowChartAdd(wxCommandEvent &cmd)
{

}

void ImgFlowBox::OnFlowChartRun(wxCommandEvent &cmd)
{

}

void ImgFlowBox::OnFlowChartMode(wxCommandEvent &cmd)
{
    imgProcFlowChart_->ClearStatus();
    switch (cmd.GetId())
    {
    case kSpamID_TOOLBOX_FLOWCHART_POINTER: 
        imgProcFlowChart_->SwitchState(FlowChart::kStateFreeIdle);
        break;
    case kSpamID_TOOLBOX_FLOWCHART_CONNECT: 
        imgProcFlowChart_->SwitchState(FlowChart::kStateConnectIdle);
        imgProcFlowChart_->SetConnectionMarks();
        break;
    default: break;
    }
}

void ImgFlowBox::OnFlowChartAlign(wxCommandEvent &cmd)
{
    switch (cmd.GetId())
    {
    case kSpamID_TOOLBOX_FLOWCHART_ALIGN_LEFT: imgProcFlowChart_->AlignLeft(); break;
    case kSpamID_TOOLBOX_FLOWCHART_ALIGN_TOP: break;
    case kSpamID_TOOLBOX_FLOWCHART_ALIGN_BOTTOM: break;
    case kSpamID_TOOLBOX_FLOWCHART_ALIGN_RIGHT: break;
    case kSpamID_TOOLBOX_FLOWCHART_ALIGN_HCENTER: imgProcFlowChart_->AlignVCenter(); break;
    case kSpamID_TOOLBOX_FLOWCHART_ALIGN_VCENTER: break;
    default: break;
    }
}
