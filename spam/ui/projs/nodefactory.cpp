#include "nodefactory.h"
#include "projnode.h"
#include "stationnode.h"
#include "rectnode.h"
#include "linenode.h"
#include "ellipsenode.h"
#include "polygonnode.h"
#include "beziergonnode.h"

NodeFactory::NodeFactory()
{
    nodeMakers_[ProjNode::GetTypeName()]              = ProjNode::Create;
    nodeMakers_[StationNode::GetTypeName()]           = StationNode::Create;
    nodeMakers_[RectNode::GetTypeName()]              = RectNode::Create;
    nodeMakers_[LineNode::GetTypeName()]              = LineNode::Create;
    nodeMakers_[GenericEllipseArcNode::GetTypeName()] = GenericEllipseArcNode::Create;
    nodeMakers_[PolygonNode::GetTypeName()]           = PolygonNode::Create;
    nodeMakers_[BeziergonNode::GetTypeName()]         = BeziergonNode::Create;
}

SPModelNode NodeFactory::Create(const std::string &typeName,
    const SPModelNode &parent,
    const wxString &title) const
{
    auto fIt = nodeMakers_.find(typeName);
    if (fIt != nodeMakers_.cend())
    {
        return fIt->second(parent, title);
    }
    else
    {
        return SPModelNode();
    };
}