#include "geombox.h"
#include <ui/spam.h>
#include <wx/artprov.h>
#include <wx/collpane.h>
#include <wx/tglbtn.h>
#include <wx/wxhtml.h>

GeomBox::GeomBox(wxWindow* parent)
    : ToolBox(parent, kSpamID_TOOLPAGE_GEOM, wxT("Geometries:"), {wxString(wxT("Create Tools:")), wxString(wxT("Boolean Tools:"))},
        kSpamID_TOOLBOX_GEOM_GUARD - kSpamID_TOOLBOX_GEOM_TRANSFORM, kSpamID_TOOLBOX_GEOM_TRANSFORM)
{
    wxWindowID cToolIds[] = {   
        kSpamID_TOOLBOX_GEOM_TRANSFORM,
        kSpamID_TOOLBOX_GEOM_EDIT,
        kSpamID_TOOLBOX_GEOM_RECT,
        kSpamID_TOOLBOX_GEOM_ELLIPSE,
        kSpamID_TOOLBOX_GEOM_POLYGON,
        kSpamID_TOOLBOX_GEOM_BEZIERGON,
        kSpamID_TOOLBOX_GEOM_LINE,
        kSpamID_TOOLBOX_GEOM_ARC,
        kSpamID_TOOLBOX_GEOM_ZIGZAGLINE,
        kSpamID_TOOLBOX_GEOM_POLYLINE,
        kSpamID_TOOLBOX_GEOM_BEZIERLINE
    };

    wxString   cToolTips[] = { 
        wxT("Select and transform entities"),
        wxT("Edit paths by nodes"),
        wxT("Create rectangles and squares"),
        wxT("Create circles, ellipses and fans"),
        wxT("Create polygons"),
        wxT("Create closed bezier regions"),
        wxT("Create lines"),
        wxT("Create arcs"),
        wxT("Create zigzag lines"),
        wxT("Create polylines"),
        wxT("Create bezier curves")
    };

    const SpamIconPurpose ip = kICON_PURPOSE_TOOLBOX;
    wxBitmap cToolIcons[] = {
        Spam::GetBitmap(ip, bm_Pointer),
        Spam::GetBitmap(ip, bm_NodeEdit),
        Spam::GetBitmap(ip, bm_Box),
        Spam::GetBitmap(ip, bm_Ellipse),
        Spam::GetBitmap(ip, bm_Polygon),
        Spam::GetBitmap(ip, bm_Beziergon),
        Spam::GetBitmap(ip, bm_Line),
        Spam::GetBitmap(ip, bm_Arc),
        Spam::GetBitmap(ip, bm_Zigzagline),
        Spam::GetBitmap(ip, bm_Polyline),
        Spam::GetBitmap(ip, bm_Bezierline)
    };

    wxWindowID bToolIds[] = {
        kSpamID_TOOLBOX_GEOM_UNION,
        kSpamID_TOOLBOX_GEOM_INTERS,
        kSpamID_TOOLBOX_GEOM_DIFF,
        kSpamID_TOOLBOX_GEOM_SYMDIFF
    };

    wxString   bToolTips[] = {
        wxT("Create union of selected regions"),
        wxT("Create intersection of selected regions"),
        wxT("Create difference of selected regions"),
        wxT("Create exclusive OR of selected regions")
    };

    wxBitmap bToolIcons[] = {
        Spam::GetBitmap(ip, bm_PathUnion),
        Spam::GetBitmap(ip, bm_PathInter),
        Spam::GetBitmap(ip, bm_PathDiff),
        Spam::GetBitmap(ip, bm_PathXOR)
    };

    ToolBox::Init(cToolIds, cToolTips, cToolIcons, WXSIZEOF(cToolIds), 0, 0);
    ToolBox::Init(bToolIds, bToolTips, bToolIcons, WXSIZEOF(bToolIds), WXSIZEOF(cToolIds), 1);
    sig_ToolEnter.connect(std::bind(&GeomBox::OnToolEnter, this, std::placeholders::_1));
}

GeomBox::~GeomBox()
{
}

void GeomBox::OnNodeEditMode(wxCommandEvent &cmd)
{
    toolEditMode_ = cmd.GetId();
    UpdateSelectionFilter();

    ToolOptions tos = GeomBox::GetToolOptions();
    tos[cp_ToolId] = kSpamID_TOOLBOX_GEOM_EDIT;
    sig_OptionsChanged(tos);
}

void GeomBox::OnToolEnter(const ToolOptions &toolOpts)
{
    const int toolId = boost::get<int>(toolOpts.at(cp_ToolId));
    switch (toolId)
    {
    case kSpamID_TOOLBOX_GEOM_TRANSFORM:
        Spam::GetSelectionFilter()->ReplacePassType(SpamEntityType::kET_GEOM);
        Spam::GetSelectionFilter()->SetEntitySelectionMode(SpamEntitySelectionMode::kESM_MULTIPLE);
        Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_GEOM_TRANSFORM);
        break;

    case kSpamID_TOOLBOX_GEOM_EDIT:
        UpdateSelectionFilter();
        break;

    case kSpamID_TOOLBOX_GEOM_RECT:
    case kSpamID_TOOLBOX_GEOM_ELLIPSE:
    case kSpamID_TOOLBOX_GEOM_POLYGON:
    case kSpamID_TOOLBOX_GEOM_BEZIERGON:
        Spam::GetSelectionFilter()->Clear();
        Spam::GetSelectionFilter()->SetEntitySelectionMode(SpamEntitySelectionMode::kESM_NONE);
        Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_GEOM_CREATE);
        break;

    case kSpamID_TOOLBOX_GEOM_DIFF:
    case kSpamID_TOOLBOX_GEOM_SYMDIFF:
        Spam::GetSelectionFilter()->Clear();
        Spam::GetSelectionFilter()->AddPassType(SpamEntityType::kET_GEOM_RECT);
        Spam::GetSelectionFilter()->AddPassType(SpamEntityType::kET_GEOM_ELLIPSE);
        Spam::GetSelectionFilter()->AddPassType(SpamEntityType::kET_GEOM_POLYGON);
        Spam::GetSelectionFilter()->AddPassType(SpamEntityType::kET_GEOM_BEZIERGON);
        Spam::GetSelectionFilter()->SetEntitySelectionMode(SpamEntitySelectionMode::kESM_SINGLE);
        break;

    default:
        break;
    }
}

wxPanel *GeomBox::GetOptionPanel(const int toolIndex, wxWindow *parent)
{
    constexpr int numTools = kSpamID_TOOLBOX_GEOM_GUARD - kSpamID_TOOLBOX_GEOM_TRANSFORM;
    wxPanel *(GeomBox::*createOption[numTools])(wxWindow *parent) = { nullptr, &GeomBox::CreateNodeEditOption, &GeomBox::CreateRectOption };

    if (createOption[toolIndex])
    {
        return (this->*createOption[toolIndex])(parent);
    }

    return nullptr;
}

ToolOptions GeomBox::GetToolOptions() const
{
    ToolOptions tos;
    tos[cp_ToolGeomVertexEditMode] = toolEditMode_;
    return tos;
}

void GeomBox::UpdateSelectionFilter(void)
{
    switch (toolEditMode_)
    {
    case kSpamID_TOOLBOX_NODE_MOVE:
        Spam::GetSelectionFilter()->AddAllPassType();
        break;

    case kSpamID_TOOLBOX_NODE_ADD:
    case kSpamID_TOOLBOX_NODE_DELETE:
        Spam::GetSelectionFilter()->Clear();
        Spam::GetSelectionFilter()->AddPassType(SpamEntityType::kET_GEOM_POLYGON);
        Spam::GetSelectionFilter()->AddPassType(SpamEntityType::kET_GEOM_BEZIERGON);
        Spam::GetSelectionFilter()->AddPassType(SpamEntityType::kET_GEOM_ZIGZAGLINE);
        Spam::GetSelectionFilter()->AddPassType(SpamEntityType::kET_GEOM_POLYLINE);
        Spam::GetSelectionFilter()->AddPassType(SpamEntityType::kET_GEOM_BEZIERLINE);
        break;

    case kSpamID_TOOLBOX_NODE_SMOOTH:
    case kSpamID_TOOLBOX_NODE_CUSP:
    case kSpamID_TOOLBOX_NODE_SYMMETRIC:
        Spam::GetSelectionFilter()->Clear();
        Spam::GetSelectionFilter()->AddPassType(SpamEntityType::kET_GEOM_BEZIERGON);
        Spam::GetSelectionFilter()->AddPassType(SpamEntityType::kET_GEOM_BEZIERLINE);
        break;

    default:
        break;
    }

    switch (toolEditMode_)
    {
    case kSpamID_TOOLBOX_NODE_MOVE:      Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_VERTEX_MOVE);      break;
    case kSpamID_TOOLBOX_NODE_ADD:       Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_VERTEX_ADD);       break;
    case kSpamID_TOOLBOX_NODE_DELETE:    Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_VERTEX_DELETE);    break;
    case kSpamID_TOOLBOX_NODE_SMOOTH:    Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_VERTEX_SMOOTH);    break;
    case kSpamID_TOOLBOX_NODE_CUSP:      Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_VERTEX_CUSP);      break;
    case kSpamID_TOOLBOX_NODE_SYMMETRIC: Spam::GetSelectionFilter()->SetEntityOperation(SpamEntityOperation::kEO_VERTEX_SYMMETRIC); break;

    default:
        break;
    }

    if (kSpamID_TOOLBOX_NODE_MOVE == toolEditMode_)
    {
        Spam::GetSelectionFilter()->SetEntitySelectionMode(SpamEntitySelectionMode::kESM_MULTIPLE);
    }
    else
    {
        Spam::GetSelectionFilter()->SetEntitySelectionMode(SpamEntitySelectionMode::kESM_SINGLE);
    }
}

wxPanel *GeomBox::CreateNodeEditOption(wxWindow *parent)
{
    auto panel = new wxScrolledWindow(parent, wxID_ANY);
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    const SpamIconPurpose ip = kICON_PURPOSE_TOOLBOX;
    auto nodeEditMethod = new wxToolBar(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxTB_TEXT | wxTB_HORZ_TEXT | wxTB_NODIVIDER);
    nodeEditMethod->AddRadioTool(kSpamID_TOOLBOX_NODE_MOVE, wxT("Move nodes"),                Spam::GetBitmap(ip, bm_NodeMove));
    nodeEditMethod->AddRadioTool(kSpamID_TOOLBOX_NODE_ADD, wxT("Insert new nodes"),           Spam::GetBitmap(ip, bm_NodeAdd));
    nodeEditMethod->AddRadioTool(kSpamID_TOOLBOX_NODE_DELETE, wxT("Delete nodes"),            Spam::GetBitmap(ip, bm_NodeDelete));
    nodeEditMethod->AddRadioTool(kSpamID_TOOLBOX_NODE_SMOOTH, wxT("Make nodes smooth"),       Spam::GetBitmap(ip, bm_NodeSmooth));
    nodeEditMethod->AddRadioTool(kSpamID_TOOLBOX_NODE_CUSP, wxT("Make nodes corner"),         Spam::GetBitmap(ip, bm_NodeCusp));
    nodeEditMethod->AddRadioTool(kSpamID_TOOLBOX_NODE_SYMMETRIC, wxT("Make nodes symmetric"), Spam::GetBitmap(ip, bm_NodeSymmetric));
    nodeEditMethod->ToggleTool(kSpamID_TOOLBOX_NODE_MOVE, true);
    nodeEditMethod->Bind(wxEVT_TOOL, &GeomBox::OnNodeEditMode, this, kSpamID_TOOLBOX_NODE_MOVE, kSpamID_TOOLBOX_NODE_SYMMETRIC);
    nodeEditMethod->Realize();

    sizerRoot->Add(nodeEditMethod, wxSizerFlags(0).Expand().Border());

    auto helpPane = new wxCollapsiblePane(panel, wxID_ANY, wxT("Instructions"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
    helpPane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &GeomBox::OnHelpCollapse, this, wxID_ANY);
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

wxPanel *GeomBox::CreateRectOption(wxWindow *parent)
{
    auto panel = new wxScrolledWindow(parent, wxID_ANY);
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);
    wxString rectMethods[] = {wxT("2 Points Method"), wxT("3 Points Method")};
    auto rectMethod = new wxRadioBox(panel, wxID_ANY, wxT("Rect Create Method"), wxDefaultPosition, wxDefaultSize, WXSIZEOF(rectMethods), rectMethods, 0, wxRA_SPECIFY_ROWS);
    sizerRoot->Add(rectMethod, wxSizerFlags(0).Expand().DoubleBorder());

    auto helpPane = new wxCollapsiblePane(panel, wxID_ANY, wxT("Instructions"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
    helpPane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &GeomBox::OnHelpCollapse, this, wxID_ANY);
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