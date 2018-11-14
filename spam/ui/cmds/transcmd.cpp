#include "transcmd.h"
#include <ui/projs/stationnode.h>
#include <ui/projs/geomnode.h>
#include <ui/projs/projtreemodel.h>

TransformCmd::TransformCmd(ProjTreeModel *model, SPStationNode &station, SPDrawableNodeVector &drawables, SpamMany &mementos)
    : SpamCmd()
    , model_(model)
    , station_(station)
    , drawables_(drawables)
    , mementos_(mementos)
{
}

void TransformCmd::Do()
{
}

void TransformCmd::Undo()
{
}

void TransformCmd::Redo()
{
}

wxString TransformCmd::GetDescription() const
{
    return wxString(wxT("Transform Entities"));
}