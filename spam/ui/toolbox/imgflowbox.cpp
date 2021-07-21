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

    auto toolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxTB_NODIVIDER);
    toolBar->SetMargins(0, 0);
    toolBar->SetToolPacking(0);
    toolBar->SetToolSeparation(0);
    toolBar->SetToolBitmapSize(wxSize(22, 22));
    toolBar->AddTool(9000000, wxT("Add"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.add")));
    toolBar->AddTool(9000001, wxT("Run"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.play")));
    toolBar->AddTool(9000002, wxT("Align Left"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.align.left")));
    toolBar->AddTool(9000003, wxT("Align Top"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.align.top")));
    toolBar->AddTool(9000004, wxT("Align Bottom"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.align.bottom")));
    toolBar->AddTool(9000005, wxT("Align Right"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.align.right")));
    toolBar->AddTool(9000006, wxT("Align Horizontal Center"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.align.hcenter")));
    toolBar->AddTool(9000007, wxT("Align Vertical Center"), Spam::GetBitmap(kICON_PURPOSE_TOOLBOX, std::string("proc.flowchart.align.vcenter")));
    toolBar->Realize();

    auto chartSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Pipeline"));
    chartSizer->Add(new FlowChart(this), wxSizerFlags(1).Expand());
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