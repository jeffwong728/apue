#include "geomnode.h"
#include <ui/evts.h>
#include <ui/projs/stationnode.h>

GeomNode::GeomNode(const SPModelNode &parent, const wxString &title)
    : DrawableNode(parent, title)
{
}

GeomNode::~GeomNode()
{
}

EntitySigType GeomNode::GetCreateSigType() const
{
    return EntitySigType::kGeomCreate;
}

EntitySigType GeomNode::GetAddSigType() const
{
    return EntitySigType::kGeomAdd;
}

EntitySigType GeomNode::GetDeleteSigType() const
{
    return EntitySigType::kGeomDelete;
}