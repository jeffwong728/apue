#include "geomcmd.h"
#include <ui/projs/stationnode.h>
#include <ui/projs/geomnode.h>
#include <ui/projs/projtreemodel.h>

GeomCmd::GeomCmd(ProjTreeModel *model, SPStationNode &station)
    : SpamCmd()
    , model_(model)
    , station_(station)
{
}

GeomCmd::GeomCmd(ProjTreeModel *model, SPStationNode &station, SPGeomNode &geom)
    : SpamCmd()
    , model_(model)
    , station_(station)
    , geom_(geom)
{
}

DeleteGeomsCmd::DeleteGeomsCmd(ProjTreeModel *model, const SPGeomNodeVector &geoms)
    : SpamCmd()
    , model_(model)
{
    for (const auto &geom : geoms)
    {
        auto station = std::dynamic_pointer_cast<StationNode>(geom->GetParent());
        if (station)
        {
            auto &geomData = geoms_[station->GetUUIDTag()];
            geomData.first = station;
            geomData.second.push_back(geom);
        }
    }
}

void DeleteGeomsCmd::Do()
{
    if (model_ && !geoms_.empty())
    {
        wxDataViewItemArray geoms;
        for (const auto &geomItem : geoms_)
        {
            for (const auto &geom : geomItem.second.second)
            {
                geoms.push_back(wxDataViewItem(geom.get()));
            }
        }

        if (!geoms.empty())
        {
            model_->Delete(geoms, true);
        }
    }
}

void DeleteGeomsCmd::Undo()
{
    if (model_)
    {
        for (auto &geomItem : geoms_)
        {
            model_->AddToStations(geomItem.second.first, geomItem.second.second, true);
        }
    }
}

void DeleteGeomsCmd::Redo()
{
    Do();
}

wxString DeleteGeomsCmd::GetDescription() const
{
    return wxString(wxT("Delete "));
}

CreateRectCmd::CreateRectCmd(ProjTreeModel *model, SPStationNode &station, const wxString &wouldTitle, const RectData &data)
    : GeomCmd(model, station)
    , wouldTitle_(wouldTitle)
    , data_(data)
{
}

void CreateRectCmd::Do()
{
    if (model_ && station_)
    {
        geom_ = model_->CreateToStation(station_, data_);
    }
}

void CreateRectCmd::Undo()
{
    if (model_ && geom_)
    {
        model_->Delete(wxDataViewItem(geom_.get()), true);
    }
}

void CreateRectCmd::Redo()
{
    if (model_ && station_ && geom_)
    {
        model_->AddToStation(station_, geom_, true);
    }
}

wxString CreateRectCmd::GetDescription() const
{
    return wxString(wxT("Create rectangle ") + geom_->GetTitle());
}

CreatePolygonCmd::CreatePolygonCmd(ProjTreeModel *model, SPStationNode &station, const wxString &wouldTitle, const PolygonData &data)
    : GeomCmd(model, station)
    , wouldTitle_(wouldTitle)
    , data_(data)
{
}

void CreatePolygonCmd::Do()
{
    if (model_ && station_)
    {
        geom_ = model_->CreateToStation(station_, data_);
    }
}

void CreatePolygonCmd::Undo()
{
    if (model_ && geom_)
    {
        model_->Delete(wxDataViewItem(geom_.get()), true);
    }
}

void CreatePolygonCmd::Redo()
{
    if (model_ && station_ && geom_)
    {
        model_->AddToStation(station_, geom_, true);
    }
}

wxString CreatePolygonCmd::GetDescription() const
{
    return wxString(wxT("Create polygon ") + geom_->GetTitle());
}

CreateEllipseCmd::CreateEllipseCmd(ProjTreeModel *model, SPStationNode &station, const wxString &wouldTitle, const GenericEllipseArcData &data)
    : GeomCmd(model, station)
    , wouldTitle_(wouldTitle)
    , data_(data)
{
}

void CreateEllipseCmd::Do()
{
    if (model_ && station_)
    {
        geom_ = model_->CreateToStation(station_, data_);
    }
}

void CreateEllipseCmd::Undo()
{
    if (model_ && geom_)
    {
        model_->Delete(wxDataViewItem(geom_.get()), true);
    }
}

void CreateEllipseCmd::Redo()
{
    if (model_ && station_ && geom_)
    {
        model_->AddToStation(station_, geom_, true);
    }
}

wxString CreateEllipseCmd::GetDescription() const
{
    return wxString(wxT("Create ellipse ") + geom_->GetTitle());
}

CreateBeziergonCmd::CreateBeziergonCmd(ProjTreeModel *model, SPStationNode &station, const wxString &wouldTitle, const BezierData &data)
    : GeomCmd(model, station)
    , wouldTitle_(wouldTitle)
    , data_(data)
{
}

void CreateBeziergonCmd::Do()
{
    if (model_ && station_)
    {
        geom_ = model_->CreateToStation(station_, data_);
    }
}

void CreateBeziergonCmd::Undo()
{
    if (model_ && geom_)
    {
        model_->Delete(wxDataViewItem(geom_.get()), true);
    }
}

void CreateBeziergonCmd::Redo()
{
    if (model_ && station_ && geom_)
    {
        model_->AddToStation(station_, geom_, true);
    }
}

wxString CreateBeziergonCmd::GetDescription() const
{
    return wxString(wxT("Create beziergon ") + geom_->GetTitle());
}