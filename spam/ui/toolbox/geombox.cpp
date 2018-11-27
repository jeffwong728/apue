#include "geombox.h"
#include <wx/artprov.h>
#include <wx/collpane.h>
#include <wx/tglbtn.h>
#include <wx/wxhtml.h>

GeomBox::GeomBox(wxWindow* parent)
: ToolBox(parent, kSpamID_TOOLPAGE_GEOM, wxT("Geometries"), kSpamID_TOOLBOX_GEOM_GUARD- kSpamID_TOOLBOX_GEOM_TRANSFORM, kSpamID_TOOLBOX_GEOM_TRANSFORM)
{
    wxWindowID toolIds[] = {   
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

    wxString   toolTips[] = { 
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

    wxBitmap toolIcons[] = {
        {wxT("res/pointer.png"), wxBITMAP_TYPE_PNG},
        {wxT("res/node_edit.png"), wxBITMAP_TYPE_PNG},
        {wxT("res/box.png"), wxBITMAP_TYPE_PNG},
        {wxT("res/ellipse.png"), wxBITMAP_TYPE_PNG},
        {wxT("res/polygon.png"), wxBITMAP_TYPE_PNG},
        {wxT("res/beziergon.png"), wxBITMAP_TYPE_PNG},
        {wxT("res/line.png"), wxBITMAP_TYPE_PNG},
        {wxT("res/arc.png"), wxBITMAP_TYPE_PNG},
        {wxT("res/zigzagline.png"), wxBITMAP_TYPE_PNG},
        {wxT("res/polyline.png"), wxBITMAP_TYPE_PNG},
        {wxT("res/bezierline.png"), wxBITMAP_TYPE_PNG}
    };

    ToolBox::Init(toolIds, toolTips, toolIcons);
}

GeomBox::~GeomBox()
{
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

wxPanel *GeomBox::CreateNodeEditOption(wxWindow *parent)
{
    auto panel = new wxScrolledWindow(parent, wxID_ANY);
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);

    auto nodeEditMethod = new wxToolBar(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxTB_TEXT | wxTB_HORZ_TEXT | wxTB_NODIVIDER);
    nodeEditMethod->AddRadioTool(kSpamID_TOOLBOX_NODE_MOVE, wxT("Move nodes"), wxBitmap(wxT("res/node_move.png"), wxBITMAP_TYPE_PNG));
    nodeEditMethod->AddRadioTool(kSpamID_TOOLBOX_NODE_ADD, wxT("Insert new nodes"), wxBitmap(wxT("res/node_add.png"), wxBITMAP_TYPE_PNG));
    nodeEditMethod->AddRadioTool(kSpamID_TOOLBOX_NODE_DELETE, wxT("Delete nodes"), wxBitmap(wxT("res/node_delete.png"), wxBITMAP_TYPE_PNG));
    nodeEditMethod->AddRadioTool(kSpamID_TOOLBOX_NODE_SMOOTH, wxT("Make nodes smooth"), wxBitmap(wxT("res/node_smooth.png"), wxBITMAP_TYPE_PNG));
    nodeEditMethod->AddRadioTool(kSpamID_TOOLBOX_NODE_CUSP, wxT("Make nodes corner"), wxBitmap(wxT("res/node_cusp.png"), wxBITMAP_TYPE_PNG));
    nodeEditMethod->AddRadioTool(kSpamID_TOOLBOX_NODE_SYMMETRIC, wxT("Make nodes symmetric"), wxBitmap(wxT("res/node_symmetric.png"), wxBITMAP_TYPE_PNG));
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