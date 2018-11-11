#include "stationcmd.h"
#include <ui/projs/stationnode.h>
#include <ui/projs/projtreemodel.h>

CreateStationCmd::CreateStationCmd(ProjTreeModel *model, const wxString &wouldTitle)
: SpamCmd()
, model_(model)
, wouldTitle_(wouldTitle)
{
}

void CreateStationCmd::Do()
{
    currentStation_ = model_->GetCurrentStation();
    station_ = model_->CreateStation(wouldTitle_);
}

void CreateStationCmd::Undo()
{
    model_->DeleteStation(station_, true);
    model_->SetCurrentStation(currentStation_);
}

void CreateStationCmd::Redo()
{
    model_->AddStation(station_);
    model_->SetCurrentStation(station_);
}

wxString CreateStationCmd::GetDescription() const
{
    return wxString(wxT("Create station ") + station_->GetTitle());
}

DeleteStationsCmd::DeleteStationsCmd(ProjTreeModel *model, const SPStationNodeVector &stations)
: SpamCmd()
, model_(model)
, stations_(stations)
{
}

void DeleteStationsCmd::Do()
{
    wxDataViewItemArray stations;
    for (const auto &s : stations_)
    {
        stations.push_back(wxDataViewItem(s.get()));
    }

    model_->Delete(stations, true);
}

void DeleteStationsCmd::Undo()
{
    model_->AddStations(stations_);
}

void DeleteStationsCmd::Redo()
{
    Do();
}

wxString DeleteStationsCmd::GetDescription() const
{
    return wxString(wxT("Delete station"));
}