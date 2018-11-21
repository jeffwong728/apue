#include "projpanel.h"
#include <wx/artprov.h>
#include <wx/wrapsizer.h>
#include <ui/spam.h>
#include <ui/evts.h>
#include <ui/cmds/geomcmd.h>
#include <ui/cmds/stationcmd.h>
#include <ui/projs/geomnode.h>
#include <ui/projs/stationnode.h>
#include <ui/projs/projtreemodel.h>
#include <ui/projs/stylecellrenderer.h>
#include <ui/cmds/stationcmd.h>
#include <pixmaps/eye.xpm>
#include <pixmaps/colours.xpm>
#include <pixmaps/circle_small.xpm>
#include <hdf5.h>
#include <H5Cpp.h>
#include <helper/h5db.h>
#include <helper/commondef.h>

ProjPanel::ProjPanel(wxWindow* parent)
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
, toolImageSize_(24, 24)
, cStation_(0)
{
    // Root sizer, vertical
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);
    sizerRoot->Add(MakeToolBar(), wxSizerFlags().Expand())->SetId(kSpamProjToolBar);
    sizerRoot->Add(MakeProjView(), wxSizerFlags(1).Expand())->SetId(kSpamProjTree);

    toolImages_.Create(toolImageSize_.GetWidth(), toolImageSize_.GetHeight());
    toolImages_.Add(wxArtProvider::GetIcon(wxART_PLUS, wxART_OTHER, toolImageSize_));
    toolImages_.Add(wxArtProvider::GetIcon(wxART_MINUS, wxART_OTHER, toolImageSize_));

    // Set sizer for the panel
    SetSizer(sizerRoot);
    GetSizer()->SetSizeHints(this);
    GetSizer()->GetItemById(kSpamProjTree)->SetMinSize(100, 100);
    Show();

    std::wstring dbPath = SpamConfig::Get<wxString>(CommonDef::GetDBPathCfgPath());
    std::wstring selProj = SpamConfig::Get<wxString>(CommonDef::GetProjCfgPath());
    LoadProject(dbPath, selProj);
    SetProjectModified(false);

    Bind(wxEVT_MENU, &ProjPanel::OnDeleteEntities,      this, kSpamID_DELETE_ENTITIES);
    Bind(wxEVT_MENU, &ProjPanel::OnShowEntities,        this, kSpamID_SHOW_ENTITIES);
    Bind(wxEVT_MENU, &ProjPanel::OnHideEntities,        this, kSpamID_HIDE_ENTITIES);
    Bind(wxEVT_MENU, &ProjPanel::OnShowOnlyEntities,    this, kSpamID_SHOW_ONLY_ENTITIES);
    Bind(wxEVT_MENU, &ProjPanel::OnShowReverseEntities, this, kSpamID_SHOW_REVERSE_ENTITIES);
    Bind(wxEVT_MENU, &ProjPanel::OnShowAllEntities,     this, kSpamID_SHOW_ALL_ENTITIES);
    Bind(wxEVT_MENU, &ProjPanel::OnHideAllEntities,     this, kSpamID_HIDE_ALL_ENTITIES);
}

wxString ProjPanel::GetProjectName() const
{
    auto projModel = GetProjTreeModel();
    return projModel ? projModel->GetProjectName() : wxEmptyString;
}

ProjTreeModel *ProjPanel::GetProjTreeModel() const
{
    auto projView = GetProjView();
    return projView ? dynamic_cast<ProjTreeModel *>(projView->GetModel()) : nullptr;
}

void ProjPanel::NewProject(const wxString &projName)
{
    auto projModel = GetProjTreeModel();
    if (projModel)
    {
        projModel->NewProject(projName);
        ModelEvent e(spamEVT_PROJECT_NEW, wxID_ANY, projModel);
        wxTheApp->AddPendingEvent(e);
    }
}

void ProjPanel::SaveProject(const wxString& dbPath)
{
    auto projModel = GetProjTreeModel();
    if (projModel)
    {
        projModel->Save(dbPath);
    }
}

void ProjPanel::LoadProject(const wxString& dbPath, const wxString &projName)
{
    auto projModel = GetProjTreeModel();
    if (projModel)
    {
        projModel->LoadProject(dbPath, projName);
        ModelEvent e(spamEVT_PROJECT_LOADED, wxID_ANY, projModel);
        wxTheApp->AddPendingEvent(e);
    }
}

void ProjPanel::SetProjectModified(bool modified)
{
    auto projModel = GetProjTreeModel();
    if (projModel)
    {
        projModel->SetModified(modified);
    }
}

bool ProjPanel::IsProjectModified() const
{
    auto projModel = GetProjTreeModel();
    return projModel? projModel->IsModified() : false;
}

void ProjPanel::CreateStation()
{
    auto projModel = GetProjTreeModel();
    if (projModel)
    {
        wxString title = wxString::Format(wxT("station%d"), cStation_++);

        auto cmd = std::make_shared<CreateStationCmd>(projModel, title);
        cmd->Do();
        SpamUndoRedo::AddCommand(cmd);
        wxLogStatus(cmd->GetDescription());
    }
}

void ProjPanel::GlowEntity(const SPDrawableNode &de)
{
    auto projView = GetProjView();
    auto model = GetProjTreeModel();

    if (projView && model)
    {
        auto dvi = wxDataViewItem(de.get());
        projView->EnsureVisible(dvi);
        int r = projView->FindRowByItem(dvi);
        if (r >= 0)
        {
            projView->SetHighlightLine(r);
            model->ItemChanged(dvi);
        }
    }
}

void ProjPanel::SelectEntity(const SPDrawableNodeVector &des)
{
    auto projView = GetProjView();
    auto model = GetProjTreeModel();

    if (projView && model)
    {
        wxDataViewItemArray items;
        for (const auto &de : des)
        {
            auto dvi = wxDataViewItem(de.get());
            projView->EnsureVisible(dvi);
            items.Add(dvi);
        }

        wxDataViewItemArray sel;
        projView->GetSelections(sel);
        sel.insert(sel.begin(), items.cbegin(), items.cend());
        projView->SetSelections(sel);
    }
    wxLogMessage(wxT("Select %zd Entities."), des.size());
}

void ProjPanel::DeselectEntity(const SPDrawableNodeVector &des)
{
    auto projView = GetProjView();
    auto model = GetProjTreeModel();

    if (projView && model)
    {
        for (const auto &de : des)
        {
            auto dvi = wxDataViewItem(de.get());
            projView->Unselect(dvi);
        }
    }
    wxLogMessage(wxT("Deselect %zd Entities."), des.size());
}

void ProjPanel::DimEntity(const SPDrawableNode &de)
{
    auto projView = GetProjView();
    auto model = GetProjTreeModel();

    if (projView && model)
    {
        auto dvi = wxDataViewItem(de.get());
        projView->EnsureVisible(dvi);
        int r = projView->FindRowByItem(dvi);
        if (r >= 0)
        {
            projView->SetHighlightLine(-1);
            model->ItemChanged(dvi);
        }
    }
}

void ProjPanel::AddToolBarButton(wxToolBar *tb, const wxString& label, const wxString& artid)
{
    wxBitmap bm = wxArtProvider::GetBitmap(artid, wxART_TOOLBAR, wxSize(16, 16));
    tb->AddTool(wxID_ANY, label, bm);
}

wxToolBar *ProjPanel::MakeToolBar()
{
    wxToolBar *tb = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
    tb->SetToolBitmapSize(wxSize(16, 16));

    tb->AddTool(kSpamID_ADD_STATION, wxT("Add"),       wxBitmap(wxT("res/add_layer_24.png"), wxBITMAP_TYPE_PNG));
    tb->AddTool(kSpamID_DELETE_ENTITIES, wxT("Delete"), wxBitmap(wxT("res/remove_layer_24.png"), wxBITMAP_TYPE_PNG));
    tb->Realize();

    tb->Bind(wxEVT_TOOL, &ProjPanel::OnAddStation, this, kSpamID_ADD_STATION);
    tb->Bind(wxEVT_TOOL, &ProjPanel::OnDeleteEntities, this, kSpamID_DELETE_ENTITIES);

    return tb;
}

wxDataViewCtrl *ProjPanel::MakeProjView()
{
    auto projView  = new wxDataViewCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_MULTIPLE | wxDV_HORIZ_RULES | wxDV_ROW_LINES);
    auto projModel = new ProjTreeModel();

    projView->AssociateModel(projModel);
    projModel->DecRef();

    auto rdr0 = new wxDataViewTextRenderer(wxDataViewTextRenderer::GetDefaultType(), wxDATAVIEW_CELL_EDITABLE);
    auto col0 = new wxDataViewColumn(wxT("Name"), rdr0, 0, 120, wxALIGN_LEFT);
    projView->AppendColumn(col0);

    auto rdr1 = new wxDataViewToggleRenderer();
    auto col1 = new wxDataViewColumn(wxT(""), rdr1, 1, 24, wxALIGN_CENTER);
    col1->SetBitmap(wxBitmap(eye_xpm));
    projView->AppendColumn(col1);

    auto *rdr2 = new StyleCellRenderer(wxDATAVIEW_CELL_ACTIVATABLE);
    auto *col2 = new wxDataViewColumn(wxT(""), rdr2, 2, 24, wxALIGN_CENTER);
    col2->SetBitmap(wxBitmap(colours_xpm));;
    projView->AppendColumn(col2);

    auto rdr3 = new wxDataViewToggleRenderer();
    auto col3 = new wxDataViewColumn(wxT(""), rdr3, 3, 24, wxALIGN_LEFT);
    col3->SetBitmap(wxBitmap(circle_small_xpm));
    projView->AppendColumn(col3);

    //wxEVT_DATAVIEW_ITEM_CONTEXT_MENU wxEVT_DATAVIEW_SELECTION_CHANGED
    projView->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &ProjPanel::OnSelectionChanged, this, wxID_ANY);
    projView->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &ProjPanel::OnContextMenu, this, wxID_ANY);
    projView->GetMainWindow()->Bind(wxEVT_LEAVE_WINDOW, &ProjPanel::OnLeaveModelBrowser, this, wxID_ANY);
    projView->GetMainWindow()->Bind(wxEVT_MOTION, &ProjPanel::OnModelBrowserMouseMotion, this, wxID_ANY);

    projModel->sig_GeomCreate.connect(std::bind(&ProjPanel::OnExpandNode, this, std::placeholders::_1));
    projModel->sig_GeomAdd.connect(std::bind(&ProjPanel::OnExpandNode, this, std::placeholders::_1));
    projView->SetAlternateRowColour(wxColour(135, 206, 235));
    return projView;
}

wxDataViewCtrl *ProjPanel::GetProjView() const
{
    return dynamic_cast<wxDataViewCtrl *>(GetSizer()->GetItemById(kSpamProjTree)->GetWindow());
}

void ProjPanel::OnAddStation(wxCommandEvent &cmd)
{
    CreateStation();
}

void ProjPanel::OnDeleteEntities(wxCommandEvent &cmd)
{
    auto projView = GetProjView();
    if (projView)
    {
        wxDataViewItemArray sels;
        projView->GetSelections(sels);

        std::vector<SPModelNode> selNodes;
        for (const auto &sel : sels)
        {
            auto node = static_cast<ModelNode*>(sel.GetID());
            if (node)
            {
                selNodes.push_back(node->GetParent()->FindChild(node));
            }
        }

        SPGeomNodeVector delGeoms;
        SPStationNodeVector delStations;

        for (const auto &selNode : selNodes)
        {
            SPModelNodeVector ancesNodes;
            selNode->GetAllAncestors(ancesNodes);

            bool ancesDelToo = false;
            for (const auto &ancesNode : ancesNodes)
            {
                if (std::any_of(selNodes.cbegin(), selNodes.cend(), [&ancesNode](const SPModelNode& n)->bool { return n->GetUUIDTag() == ancesNode->GetUUIDTag(); }))
                {
                    ancesDelToo = true;
                    break;
                }
            }

            if (!ancesDelToo)
            {
                auto station = std::dynamic_pointer_cast<StationNode>(selNode);
                if (station)
                {
                    delStations.push_back(station);
                }

                auto geom = std::dynamic_pointer_cast<GeomNode>(selNode);
                if (geom)
                {
                    delGeoms.push_back(geom);
                }
            }
        }

        SPSpamCmdVector cmds;
        if (!delGeoms.empty())
        {
            auto cmd = std::make_shared<DeleteGeomsCmd>(Spam::GetModel(), delGeoms);
            cmds.push_back(cmd);
        }

        if (!delStations.empty())
        {
            auto cmd = std::make_shared<DeleteStationsCmd>(Spam::GetModel(), delStations);
            cmds.push_back(cmd);
        }

        if (!cmds.empty())
        {
            auto cmd = std::make_shared<MacroCmd>(cmds);
            cmd->Do();
            SpamUndoRedo::AddCommand(cmd);
            wxLogStatus(cmd->GetDescription());
        }
    }
}

void ProjPanel::OnShowEntities(wxCommandEvent &cmd)
{

}

void ProjPanel::OnHideEntities(wxCommandEvent &cmd)
{

}

void ProjPanel::OnShowOnlyEntities(wxCommandEvent &cmd)
{

}

void ProjPanel::OnShowReverseEntities(wxCommandEvent &cmd)
{

}

void ProjPanel::OnShowAllEntities(wxCommandEvent &cmd)
{

}

void ProjPanel::OnHideAllEntities(wxCommandEvent &cmd)
{

}

void ProjPanel::OnExpandNode(const SPModelNodeVector &nodes)
{
    for (const auto &node : nodes)
    {
        if (node)
        {
            auto par = node->GetParent();
            auto projView = GetProjView();
            if (par && projView)
            {
                if (par->GetChildCount() > 0)
                {
                    if (!projView->IsExpanded(wxDataViewItem(par.get())))
                    {
                        projView->Expand(wxDataViewItem(par.get()));
                    }
                }
            }
        }
    }
}

void ProjPanel::OnSelectionChanged(wxDataViewEvent &e)
{
    auto projView = GetProjView();
    auto model = GetProjTreeModel();
    if (projView && model)
    {
        wxDataViewItemArray sels;
        projView->GetSelections(sels);

        SPDrawableNodeVector drawables;
        for (const auto &sel : sels)
        {
            auto node = static_cast<ModelNode*>(sel.GetID());
            if (node && node->GetParent())
            {
                auto drawable = std::dynamic_pointer_cast<DrawableNode>(node->GetParent()->FindChild(node));
                if (drawable)
                {
                    drawables.push_back(drawable);
                }
            }
        }

        sig_EntitySelect(drawables);
    }
}

void ProjPanel::OnContextMenu(wxDataViewEvent &e)
{
    auto projView = GetProjView();
    if (projView)
    {
        int numSel = projView->GetSelectedItemsCount();
        wxMenu menu;
        menu.Append(kSpamID_DELETE_ENTITIES, wxT("Delete"))->Enable(numSel);
        menu.AppendSeparator();
        menu.Append(kSpamID_SHOW_ENTITIES, wxT("Show"))->Enable(numSel);
        menu.Append(kSpamID_HIDE_ENTITIES, wxT("Hide"))->Enable(numSel);
        menu.Append(kSpamID_SHOW_ONLY_ENTITIES, wxT("Show Only"))->Enable(numSel);
        menu.Append(kSpamID_SHOW_REVERSE_ENTITIES, wxT("Show Reverse"))->Enable(numSel);
        menu.Append(kSpamID_SHOW_ALL_ENTITIES, wxT("Show All"));
        menu.Append(kSpamID_HIDE_ALL_ENTITIES, wxT("Hide All"));

        GetProjView()->PopupMenu(&menu);
    }
}

void ProjPanel::OnModelBrowserMouseMotion(wxMouseEvent &e)
{
    auto projView = GetProjView();
    auto model = GetProjTreeModel();
    auto wnd = dynamic_cast<wxWindow *>(e.GetEventObject());
    if (projView && model && wnd)
    {
        wxDataViewItem tItem;
        wxDataViewColumn* pColumn = nullptr;
        wxPoint tPos = projView->ScreenToClient(wnd->ClientToScreen(e.GetPosition()));
        projView->HitTest(tPos, tItem, pColumn);

        int oldHL = projView->GetHighlightLine();
        int newHL = oldHL;

        if (tItem.IsOk())
        {
            newHL = projView->FindRowByItem(tItem);
        }
        else
        { 
            newHL = -1;
        }

        if (oldHL != newHL)
        {
            projView->SetHighlightLine(newHL);
            if (oldHL >= 0)
            {
                wxDataViewItem dvi = projView->FindItemByRow(oldHL);
                if (dvi)
                {
                    model->ItemChanged(dvi);
                    auto node = static_cast<ModelNode*>(dvi.GetID());
                    if (node && node->GetParent())
                    {
                        sig_EntityDim(node->GetParent()->FindChild(node));
                    }
                }
                wxLogMessage(wxT("Row %d Lose Highlight"), oldHL);
            }

            if (newHL>=0)
            {
                model->ItemChanged(tItem);
                auto node = static_cast<ModelNode*>(tItem.GetID());
                if (node && node->GetParent())
                {
                    sig_EntityGlow(node->GetParent()->FindChild(node));
                }
                wxLogMessage(wxT("Row %d Get Highlight"), newHL);
            }
        }
    }
}

void ProjPanel::OnLeaveModelBrowser(wxMouseEvent &e)
{
    auto projView = GetProjView();
    auto model = GetProjTreeModel();
    if (projView && model)
    {
        int oldHL = projView->GetHighlightLine();
        if (oldHL >= 0)
        {
            wxDataViewItem dvi = projView->FindItemByRow(oldHL);
            if (dvi)
            {
                model->ItemChanged(dvi);
                auto node = static_cast<ModelNode*>(dvi.GetID());
                if (node && node->GetParent())
                {
                    sig_EntityDim(node->GetParent()->FindChild(node));
                }
            }
            projView->SetHighlightLine(-1);
            wxLogMessage(wxT("Row %d Lose Highlight"), oldHL);
        }
    }
}