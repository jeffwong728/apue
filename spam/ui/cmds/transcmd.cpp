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

wxString NodeEditCmd::GetDescription() const
{
    return wxString(wxT("Move Entity Node"));
}

NodeModifyCmd::NodeModifyCmd(ProjTreeModel *model, SPStationNode &station, const SPDrawableNode &drawable, const boost::any &memento)
    : SpamCmd()
    , model_(model)
    , station_(station)
    , drawable_(drawable)
    , memento_(memento)
{
}

void NodeModifyCmd::Do()
{
}

void NodeModifyCmd::Undo()
{
    SpamMany mementos;
    mementos.push_back(drawable_->CreateMemento());

    SPDrawableNodeVector drawables(1, drawable_);
    model_->RestoreTransform(drawables, SpamMany(1, memento_), true);
    memento_ = mementos.front();
}

void NodeModifyCmd::Redo()
{
    NodeModifyCmd::Undo();
}

wxString NodeModifyCmd::GetDescription() const
{
    return wxString(wxT("Modify Entity Node"));
}