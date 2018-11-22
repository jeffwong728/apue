#include "projtreemodel.h"
#include "projnode.h"
#include "rectnode.h"
#include "ellipsenode.h"
#include "polygonnode.h"
#include "nodefactory.h"
#include <ui/cmndef.h>
#include <ui/evts.h>
#include <ui/spam.h>
#include <H5Cpp.h>
#include <boost/filesystem.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/2geom.h>
#include <2geom/path-intersection.h>
#pragma warning( pop )

ProjTreeModel::ProjTreeModel()
    : ProjTreeModel(wxT("项目1"))
{
}

ProjTreeModel::ProjTreeModel(const wxString &title)
    : modified_(true)
{
    root_ = std::make_shared<ProjNode>(SPModelNode(), title);
}

ProjTreeModel::~ProjTreeModel()
{
}

wxString ProjTreeModel::GetTitle(const wxDataViewItem &item) const
{
    auto node = static_cast<ModelNode*>(item.GetID());
    if (node)
    {
        return node->title_;
    }
    else
    {
        return wxEmptyString;
    }
}

wxString ProjTreeModel::GetProjectName() const
{
    if (root_)
    {
        return root_->title_;
    }
    else
    {
        return wxEmptyString;
    }
}

SPProjNode ProjTreeModel::GetProject() const
{
    return std::dynamic_pointer_cast<ProjNode>(root_);
}

void ProjTreeModel::SetCurrentStation(const SPStationNode &station)
{
    if (station && !station->IsCurrentStation())
    {
        auto spCurrStation = currentStation_.lock();
        if (spCurrStation)
        {
            spCurrStation->current_ = false;
        }
        station->current_ = true;
        currentStation_ = station;
        ItemChanged(wxDataViewItem(station.get()));
        SetModified();
    }
}

bool ProjTreeModel::GetVisible(const wxDataViewItem &item) const
{
    auto node = static_cast<ModelNode*>(item.GetID());
    if (node)
    {
        return node->isVisible();
    }
    else
    {
        return false;
    }
}

SPStationNode ProjTreeModel::CreateStation(const wxString &title)
{
    SPStationNode newStation;
    if (root_)
    {
        newStation = std::make_shared<StationNode>(root_, GetUnusedName(root_, title));
        newStation->current_ = true;
        AddStation(newStation);
    }

    return newStation;
}

void ProjTreeModel::AddStation(const SPStationNode &station)
{
    if (station->IsCurrentStation())
    {
        auto spCurrStation = currentStation_.lock();
        if (spCurrStation)
        {
            spCurrStation->current_ = false;
        }
        currentStation_ = station;
    }

    root_->Append(station);
    wxDataViewItem child(station.get());
    wxDataViewItem parent(root_.get());
    ItemAdded(parent, child);
    SetModified();

    SPModelNodeVector stations(1, station);
    sig_StationAdd(stations);
}

void ProjTreeModel::AddStations(const SPStationNodeVector &stations)
{
    for (const auto& station : stations)
    {
        if (station->IsCurrentStation())
        {
            auto spCurrStation = currentStation_.lock();
            if (spCurrStation)
            {
                spCurrStation->current_ = false;
            }
            currentStation_ = station;
        }

        root_->Append(station);
        wxDataViewItem child(station.get());
        wxDataViewItem parent(root_.get());
        ItemAdded(parent, child);
        SetModified();
    }

    sig_StationAdd(SPModelNodeVector(stations.cbegin(), stations.cend()));
}

void ProjTreeModel::DeleteStation(const SPStationNode &station, bool fireEvent)
{
    if (station)
    {
        wxDataViewItem parent(station->GetParent().get());
        if (parent.IsOk())
        {
            station->GetParent()->RemoveChild(station.get());
            ItemDeleted(parent, wxDataViewItem(station.get()));
            SetModified();

            if (fireEvent)
            {
                SPModelNodeVector stations(1, station);
                sig_StationDelete(stations);
            }
        }
    }
}

void ProjTreeModel::AddToStation(SPStationNode &station, SPGeomNode &geom, bool fireEvent)
{
    station->Append(geom);
    wxDataViewItem child(geom.get());
    wxDataViewItem parent(station.get());
    ItemAdded(parent, child);
    SetModified();

    if (fireEvent)
    {
        SPModelNodeVector geoms(1, geom);
        sig_GeomAdd(geoms);
    }
}

void ProjTreeModel::AddToStations(SPStationNode &station, SPGeomNodeVector &geoms, bool fireEvent)
{
    for (const auto &geom : geoms)
    {
        station->Append(geom);
        wxDataViewItem child(geom.get());
        wxDataViewItem parent(station.get());
        ItemAdded(parent, child);
        SetModified();
    }

    if (fireEvent && !geoms.empty())
    {
        sig_GeomAdd(SPModelNodeVector(geoms.cbegin(), geoms.cend()));
    }
}

SPGeomNode ProjTreeModel::CreateToStation(SPStationNode &station, const RectData &rd)
{
    SPRectNode newRect = std::make_shared<RectNode>(station, GetUnusedName(station, wxT("rect")));
    newRect->SetData(rd);
    newRect->drawStyle_.strokeWidth_ = SpamConfig::Get<int>(cp_ToolGeomStrokeWidth, 1);
    newRect->drawStyle_.strokeColor_.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomStrokePaint, wxBLUE->GetRGBA()));
    newRect->drawStyle_.fillColor_.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomFillPaint, wxYELLOW->GetRGBA()));
    station->Append(newRect);

    wxDataViewItem child(newRect.get());
    wxDataViewItem parent(station.get());
    ItemAdded(parent, child);
    SetModified();

    SPModelNodeVector geoms(1, newRect);
    sig_GeomCreate(geoms);

    return newRect;
}

SPGeomNode ProjTreeModel::CreateToStation(SPStationNode &station, const GenericEllipseArcData &ed)
{
    SPGenericEllipseArcNode newEllip = std::make_shared<GenericEllipseArcNode>(station, GetUnusedName(station, wxT("ellipse")));
    newEllip->SetData(ed);
    newEllip->drawStyle_.strokeWidth_ = SpamConfig::Get<int>(cp_ToolGeomStrokeWidth, 1);
    newEllip->drawStyle_.strokeColor_.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomStrokePaint, wxBLUE->GetRGBA()));
    newEllip->drawStyle_.fillColor_.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomFillPaint, wxYELLOW->GetRGBA()));
    station->Append(newEllip);

    wxDataViewItem child(newEllip.get());
    wxDataViewItem parent(station.get());
    ItemAdded(parent, child);
    SetModified();

    SPModelNodeVector geoms(1, newEllip);
    sig_GeomCreate(geoms);

    return newEllip;
}

SPGeomNode ProjTreeModel::CreateToStation(SPStationNode &station, const PolygonData &pd)
{
    SPPolygonNode newPg = std::make_shared<PolygonNode>(station, GetUnusedName(station, wxT("polygon")));
    newPg->SetData(pd);
    newPg->drawStyle_.strokeWidth_ = SpamConfig::Get<int>(cp_ToolGeomStrokeWidth, 1);
    newPg->drawStyle_.strokeColor_.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomStrokePaint, wxBLUE->GetRGBA()));
    newPg->drawStyle_.fillColor_.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomFillPaint, wxYELLOW->GetRGBA()));
    station->Append(newPg);

    wxDataViewItem child(newPg.get());
    wxDataViewItem parent(station.get());
    ItemAdded(parent, child);
    SetModified();

    SPModelNodeVector geoms(1, newPg);
    sig_GeomCreate(geoms);

    return newPg;
}

void ProjTreeModel::Delete(const wxDataViewItem &item, bool fireEvent)
{
    auto node = static_cast<ModelNode*>(item.GetID());
    if (node)
    {
        wxDataViewItem parent(node->GetParent().get());
        if (parent.IsOk())
        {
            auto spCurrStation = currentStation_.lock();
            if (spCurrStation && spCurrStation.get()==node)
            {
                NewCurrentStation();
            }

            auto child = node->GetParent()->FindChild(node);
            node->GetParent()->RemoveChild(node);
            ItemDeleted(parent, item);
            SetModified();

            if (fireEvent)
            {
                SPModelNodeVector ents(1, child);
                FireDeleteSignal(ents);
            }
        }
        else
        {
            wxASSERT(node == root_.get());
            wxLogError(wxT("Cannot remove the root item!"));
        }
    }
}

void ProjTreeModel::Delete(const wxDataViewItemArray &items, bool fireEvent)
{
    SPModelNodeVector ents;
    for (const auto &item : items)
    {
        auto node = static_cast<ModelNode*>(item.GetID());
        if (node)
        {
            wxDataViewItem parent(node->GetParent().get());
            if (parent.IsOk())
            {
                auto spCurrStation = currentStation_.lock();
                if (spCurrStation && spCurrStation.get() == node)
                {
                    NewCurrentStation();
                }

                auto child = node->GetParent()->FindChild(node);
                node->GetParent()->RemoveChild(node);
                ItemDeleted(parent, item);
                SetModified();

                if (fireEvent)
                {
                    ents.push_back(child);
                }
            }
        }
    }

    if (fireEvent)
    {
        FireDeleteSignal(ents);
    }
}

void ProjTreeModel::NewCurrentStation()
{
    for (const auto &s : GetAllStations())
    {
        if (s)
        {
            if (!s->IsCurrentStation())
            {
                currentStation_ = s;
                s->current_ = true;
                SetModified();
                break;
            }
        }
    }
}

void ProjTreeModel::NewProject(const wxString &projName)
{
    wxDataViewItem rItem(root_.get());
    wxDataViewItemArray children;
    auto cChildren = static_cast<int>(GetChildren(rItem, children));
    for (int c = 0; c<cChildren; ++c)
    {
        Delete(children[c], false);
    }

    wxVariant varTitle = projName;
    if (SetValue(varTitle, rItem, kStation_LABEL_COL))
    {
        ItemChanged(rItem);
    }


    root_->title_ = projName;
    SetModified();
}

void ProjTreeModel::Save(const wxString &path) const
{
    const H5std_string  fileName(path);
    try
    {
        H5::Exception::dontPrint();

        boost::filesystem::path p(path.ToStdWstring());
        boost::system::error_code ec;
        unsigned int flags = 0;
        if (boost::filesystem::exists(p, ec) && boost::filesystem::is_regular_file(p, ec))
        {
            flags = H5F_ACC_RDWR;
        }
        else
        {
            flags = H5F_ACC_TRUNC;
        }

        H5::H5File file(fileName, flags);
        H5::Group rootGroup = file.openGroup("/");
        if (root_)
        {
            root_->Save(rootGroup);
            SetModified(false);
        }
    }
    catch (const H5::Exception &e)
    {
        wxLogError(e.getCDetailMsg());
    }
}

void ProjTreeModel::LoadProject(const wxString& dbPath, const wxString &projName)
{
    NewProject(projName);
    try
    {
        const H5std_string  fileName(dbPath);
        H5::Exception::dontPrint();

        H5::H5File file(fileName, H5F_ACC_RDONLY);
        H5::Group rootGroup(file.openGroup("/"));
        H5::Group projGroup(rootGroup.openGroup(projName.ToUTF8()));
        root_->Load(projGroup, NodeFactory(), root_);
        ScanSetCurrentSattion();
        SetModified();
    }
    catch (const H5::Exception &e)
    {
        wxLogError(e.getCDetailMsg());
    }
}

SPStationNodeVector ProjTreeModel::GetAllStations() const
{
    SPStationNodeVector allStations;
    if (root_)
    {
        const auto& children = root_->GetChildren();
        for (const auto &c : children)
        {
            auto station = std::dynamic_pointer_cast<StationNode>(c);
            if (station)
            {
                allStations.push_back(station);
            }
        }
    }

    return allStations;
}

SPStationNode ProjTreeModel::FindStationByUUID(const std::string &uuidTag) const
{
    if (root_)
    {
        const auto& children = root_->GetChildren();
        for (const auto &c : children)
        {
            auto station = std::dynamic_pointer_cast<StationNode>(c);
            if (station && (uuidTag==station->GetUUIDTag()))
            {
                return station;
            }
        }
    }

    return SPStationNode();
}

void ProjTreeModel::RestoreTransform(SPDrawableNodeVector &drawables, const SpamMany &mementos, const bool fireEvent)
{
    if (drawables.size() == mementos.size())
    {
        Geom::OptRect rect;
        SPDrawableNodeVector changedDrawables;

        int numDrawables = static_cast<int>(drawables.size());
        for (int i=0; i<numDrawables; ++i)
        {
            auto &drawable = drawables[i];
            const auto &memento  = mementos[i];

            if (drawable)
            {
                Geom::PathVector pv;
                drawable->BuildPath(pv);
                rect.unionWith(pv.boundsFast());

                if (drawable->RestoreFromMemento(memento))
                {
                    changedDrawables.push_back(drawable);
                }
            }
        }

        if (fireEvent)
        {
            sig_DrawableShapeChange(changedDrawables, rect);
        }
    }
}

wxString ProjTreeModel::GetColumnType(unsigned int col) const
{
    wxVariant clrVar;
    switch (col)
    {
    case kStation_LABEL_COL       : clrVar = wxT(""); break;
    case kStation_VISIBLE_COL     : clrVar = false; break;
    case kStation_DRAW_STYLE_COL  : clrVar << DrawStyle(); break;
    case kStation_LOCK_COL        : clrVar = false; break;

    default: break;
    }

    return clrVar.GetType();
}

void ProjTreeModel::GetValue(wxVariant &variant,
    const wxDataViewItem &item,
    unsigned int col) const
{
    wxASSERT(item.IsOk());
    auto node = static_cast<ModelNode*>(item.GetID());
    if (node)
    {
        switch (col)
        {
        case kStation_LABEL_COL       : variant =  node->title_;     break;
        case kStation_VISIBLE_COL     : variant =  node->visible_;   break;
        case kStation_DRAW_STYLE_COL  : variant << node->drawStyle_; break;
        case kStation_LOCK_COL        : variant =  node->locked_;    break;

        default: wxLogError(wxT("ProjTreeModel::GetValue: wrong column %d"), col); break;
        }
    }
}

bool ProjTreeModel::SetValue(const wxVariant &variant,
    const wxDataViewItem &item,
    unsigned int col)
{
    wxASSERT(item.IsOk());
    auto node = static_cast<ModelNode*>(item.GetID());
    if (node)
    {
        switch (col)
        {
        case kStation_LABEL_COL:        node->title_ = variant.GetString(); SetModified(); return true;
        case kStation_VISIBLE_COL:      node->visible_ = variant.GetBool(); SetModified(); return true;
        case kStation_DRAW_STYLE_COL:   node->drawStyle_ << variant;        SetModified(); return true;
        case kStation_LOCK_COL:         node->locked_ = variant.GetBool();  SetModified(); return true;

        default: wxLogError(wxT("ProjTreeModel::SetValue: wrong column %d"), col); break;
        }
    }

    return false;
}

bool ProjTreeModel::IsEnabled(const wxDataViewItem &WXUNUSED(item),
    unsigned int WXUNUSED(col)) const
{
    return true;
}

wxDataViewItem ProjTreeModel::GetParent(const wxDataViewItem &item) const
{
    // the invisible root node has no parent
    if (item.IsOk())
    {
        auto node = static_cast<ModelNode*>(item.GetID());

        if (node == root_.get())
        {
            return wxDataViewItem(nullptr);
        }
        else
        {
            return wxDataViewItem(node->GetParent().get());
        }
    }
    else
    {
        return wxDataViewItem(nullptr);
    }
}

bool ProjTreeModel::IsContainer(const wxDataViewItem &item) const
{
    if (item.IsOk())
    {
        auto node = static_cast<ModelNode*>(item.GetID());
        return node->IsContainer();
    }
    else
    {
        return true;
    }
}

unsigned int ProjTreeModel::GetChildren(const wxDataViewItem &parent,
    wxDataViewItemArray &arr) const
{
    auto node = static_cast<ModelNode*>(parent.GetID());
    if (node)
    {
        for (const auto &cld : node->GetChildren())
        {
            arr.Add(wxDataViewItem(cld.get()));
        }

        return node->GetChildCount();
    }
    else
    {
        arr.Add(wxDataViewItem(root_.get()));
        return 1;
    }
}

bool ProjTreeModel::GetAttr(const wxDataViewItem &item,
    unsigned int col,
    wxDataViewItemAttr &attr) const
{
    if (item.IsOk() && 0==col)
    {
        auto node = static_cast<ModelNode*>(item.GetID());
        if (node && node->IsCurrentStation())
        {
            attr.SetBold(true);
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool ProjTreeModel::IsNameExisting(const SPModelNode &parent, const wxString &baseName)
{
    if (parent)
    {
        const auto& children = parent->GetChildren();
        for (const auto &c : children)
        {
            if (baseName == c->GetTitle())
            {
                return true;
            }
        }
    }

    return false;
}

wxString ProjTreeModel::GetUnusedName(const SPModelNode &parent, const wxString &seedName)
{
    if (IsNameExisting(parent, seedName))
    {
        int cName = 2;
        wxString newName;
        do
        {
            newName = seedName + wxT("(")+ wxString::FromDouble(cName++)+ wxT(")");

        } while (IsNameExisting(parent, newName));

        return newName;
    }
    else
    {
        return seedName;
    }
}

void ProjTreeModel::ScanSetCurrentSattion(void)
{
    bool haveCurrent = false;
    for (const auto &s : GetAllStations())
    {
        if (s)
        {
            if (s->IsCurrentStation())
            {
                haveCurrent = true;
                currentStation_ = s;
                SetModified();
                break;
            }
        }
    }

    if (!haveCurrent)
    {
        NewCurrentStation();
    }
}

void ProjTreeModel::FireDeleteSignal(const SPModelNodeVector &ents) const
{
    std::map<EntitySigType, SPModelNodeVector> entsBySigs;
    for (const auto &ent : ents)
    {
        entsBySigs[ent->GetDeleteSigType()].push_back(ent);
    }

    for (const auto &entsBySig : entsBySigs)
    {
        switch (entsBySig.first)
        {
        case EntitySigType::kEntityDelete:
            if(!entsBySig.second.empty()) sig_EntityDelete(entsBySig.second);
            break;

        case EntitySigType::kStationDelete:
            if (!entsBySig.second.empty()) sig_StationDelete(entsBySig.second);
            break;

        case EntitySigType::kGeomDelete:
            if (!entsBySig.second.empty()) sig_GeomDelete(entsBySig.second);
            break;

        default:
            break;
        }
    }
}