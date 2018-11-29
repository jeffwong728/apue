#include "toolbox.h"
#include <ui/spam.h>
#include <wx/artprov.h>
#include <wx/collpane.h>
#include <wx/tglbtn.h>
#include <wx/wxhtml.h>

ToolBox::ToolBox(wxWindow* parent, const wxWindowID winid, const wxString &toolboxName, const int numTools, const int startToolId)
: wxPanel(parent, winid)
, startToolId_(startToolId)
, collOptPane_(nullptr)
, tools_(numTools)
{
    for (auto &tool : tools_)
    {
        std::get<0>(tool) = nullptr;
        std::get<1>(tool) = nullptr;
        std::get<2>(tool) = nullptr;
    }

    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);
    collToolPane_ = new wxCollapsiblePane(this, wxID_ANY, toolboxName, wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
    collToolPane_->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &ToolBox::OnToolCollapse, this, wxID_ANY);
    collToolPane_->Collapse(false);
    sizerRoot->Add(collToolPane_, wxSizerFlags(0).Expand());

    wxWindow *winTool = collToolPane_->GetPane();
    auto paneSizer = new wxGridSizer(6, 2, 2);

    collOptPane_ = new wxCollapsiblePane(this, wxID_ANY, wxT("Options"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
    collOptPane_->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &ToolBox::OnToolCollapse, this, wxID_ANY);
    sizerRoot->Add(collOptPane_, wxSizerFlags(1).Expand());
    wxWindow *winOpt = collOptPane_->GetPane();
    wxSizer *sizerOpt = new wxBoxSizer(wxVERTICAL);

    winTool->SetSizer(paneSizer);
    winOpt->SetSizer(sizerOpt);
    SetSizer(sizerRoot);

    collOptPane_->Disable();

    sig_ToolQuit.connect(std::bind(&ToolBox::OnToolQuit, this, std::placeholders::_1));
}

ToolBox::~ToolBox()
{
}

void ToolBox::Init(const wxWindowID toolIds[], const wxString toolTips[], const wxBitmap toolIcons[])
{
    wxSizer * const sizerRoot = GetSizer();
    wxWindow *winTool = collToolPane_->GetPane();
    wxSizer * const paneSizer = winTool->GetSizer();
    wxWindow *winOpt = collOptPane_->GetPane();
    wxSizer * const sizerOpt = winOpt->GetSizer();

    for (int t = 0; t<static_cast<int>(tools_.size()); ++t)
    {
        auto tBtn = new wxBitmapToggleButton(winTool, toolIds[t], toolIcons[t], wxDefaultPosition, wxSize(32, 32), wxBU_LEFT | wxBU_EXACTFIT);
        tBtn->SetToolTip(toolTips[t]);
        tBtn->Bind(wxEVT_TOGGLEBUTTON, &ToolBox::OnTool, this, toolIds[t]);
        paneSizer->Add(tBtn, wxSizerFlags(1).Expand());
        std::get<0>(tools_[t]) = tBtn;
    }

    sizerRoot->SetMinSize(GetBestSize());

    if (!tools_.empty())
    {
        wxToggleButton *tBtn = std::get<0>(tools_[0]);
        if (tBtn)
        {
            tBtn->SetValue(true);
        }
    }
    else
    {
        GetSizer()->Layout();
    }
}

void ToolBox::OpenToolbox()
{
    int toolIndex = 0;
    for (auto &tool : tools_)
    {
        wxToggleButton *tBtn = std::get<0>(tool);
        if (tBtn && tBtn->GetValue())
        {
            if (CreateOption(toolIndex))
            {
                GetSizer()->Layout();
            }
            sig_ToolEnter(tBtn->GetId());
        }

        toolIndex += 1;
    }
}

void ToolBox::QuitToolbox()
{
    for (auto &tool : tools_)
    {
        wxToggleButton *tBtn = std::get<0>(tool);
        if (tBtn && tBtn->GetValue())
        {
            sig_ToolQuit(tBtn->GetId());
        }
    }
}

void ToolBox::OnToolCollapse(wxCollapsiblePaneEvent &e)
{
    GetSizer()->Layout();
}

void ToolBox::OnHelpCollapse(wxCollapsiblePaneEvent &e)
{
    collOptPane_->GetPane()->GetSizer()->Layout();
}

void ToolBox::OnTool(wxCommandEvent &e)
{
    bool needLayout = false;

    for (auto &tool : tools_)
    {
        wxToggleButton *tBtn = std::get<0>(tool);
        if (tBtn && tBtn->GetValue() && e.GetId()!=tBtn->GetId())
        {
            tBtn->SetValue(false);
            sig_ToolQuit(tBtn->GetId());

            wxSizerItem *optWnd = std::get<2>(tool);
            if (optWnd)
            {
                optWnd->Show(false);
                collOptPane_->Collapse(true);
                collOptPane_->Disable();
                needLayout = true;
            }
        }
    }

    if (e.IsChecked())
    {
        int tIndex = e.GetId() - startToolId_;
        if (tIndex >= 0 && tIndex<tools_.size())
        {
            wxSizerItem *optWnd = std::get<2>(tools_[tIndex]);
            if (optWnd)
            {
                collOptPane_->Enable();
                optWnd->Show(true);
                collOptPane_->Collapse(false);
                needLayout = true;
            }
            else
            {
                if (CreateOption(tIndex))
                {
                    needLayout = true;
                }
            }

            wxToggleButton *tBtn = std::get<0>(tools_[tIndex]);
            if (tBtn)
            {
                sig_ToolEnter(tBtn->GetId());
            }
        }
    }
    else
    {
        int tIndex = e.GetId() - startToolId_;
        if (tIndex >= 0 && tIndex<tools_.size())
        {
            wxToggleButton *tBtn = std::get<0>(tools_[tIndex]);
            if (tBtn)
            {
                tBtn->SetValue(true);;
            }
        }
    }

    if (needLayout)
    {
        GetSizer()->Layout();
    }
}

bool ToolBox::CreateOption(const int toolIndex)
{
    if (!std::get<2>(tools_[toolIndex]))
    {
        wxPanel *optPanel = GetOptionPanel(toolIndex, collOptPane_->GetPane());
        if (optPanel)
        {
            wxWindow *winOpt = collOptPane_->GetPane();
            wxSizer * const sizerOpt = winOpt->GetSizer();
            auto sizerItem = sizerOpt->Add(optPanel, wxSizerFlags(1).Expand());
            std::get<2>(tools_[toolIndex]) = sizerItem;

            collOptPane_->Enable();
            sizerItem->Show(true);
            collOptPane_->Collapse(false);

            return true;
        }
    }

    return false;
}

void ToolBox::OnToolQuit(int toolId)
{
    Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_NONE);
    Spam::GetSelectionFilter()->AddAllPassType();
}