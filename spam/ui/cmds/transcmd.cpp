#include "transcmd.h"
#include <ui/projs/stationnode.h>
#include <ui/projs/geomnode.h>
#include <ui/projs/projtreemodel.h>

TransformCmd::TransformCmd(ProjTreeModel *model, SPStationNode &station, const SPDrawableNodeVector &drawables, const SpamMany &mementos)
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
    SpamMany mementos;
    for (const auto &drawable : drawables_)
    {
        mementos.push_back(drawable->CreateMemento());
    }

    model_->RestoreTransform(drawables_, mementos_, true);
    mementos_.swap(mementos);
}

void TransformCmd::Redo()
{
    TransformCmd::Undo();
}

wxString TransformCmd::GetDescription() const
{
    return wxString(wxT("Transform Entities"));
}