#include "modelnode.h"
#include "nodefactory.h"
#include <H5Cpp.h>
#include <ui/evts.h>
#include <helper/h5db.h>
#include <helper/commondef.h>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

ModelNode::ModelNode()
    : locked_(false)
    , visible_(true)
    , tag_(boost::uuids::random_generator()())
    , rank_(0)
{
}

ModelNode::ModelNode(const SPModelNode &parent)
    : ModelNode()
{
    parent_ = parent;
}

ModelNode::ModelNode(const SPModelNode &parent, const wxString &title)
    : ModelNode()
{
    parent_ = parent;
    title_  = title;
}

ModelNode::~ModelNode()
{
}

EntitySigType ModelNode::GetCreateSigType() const
{
    return EntitySigType::kEntityCreate;
}

EntitySigType ModelNode::GetAddSigType() const
{
    return EntitySigType::kEntityAdd;
}

EntitySigType ModelNode::GetDeleteSigType() const
{
    return EntitySigType::kEntityDelete;
}

void ModelNode::Save(const H5::Group &g) const
{
    H5DB::SetAttribute(g, std::string("UUID"),        GetUUIDTag());
    H5DB::SetAttribute(g, std::string("StrokeColor"), drawStyle_.strokeColor_);
    H5DB::SetAttribute(g, std::string("FillColor"),   drawStyle_.fillColor_);
    H5DB::SetAttribute(g, std::string("StrokeWidth"), drawStyle_.strokeWidth_);
    H5DB::SetAttribute(g, std::string("Visible"),     visible_);
    H5DB::SetAttribute(g, std::string("Locked"),      locked_);
    H5DB::SetAttribute(g, std::string("Rank"),        rank_);

    for (const auto &c : children_)
    {
        c->Save(g);
    }
}

void ModelNode::Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me)
{
    auto numObjs = g.getNumObjs();
    for (hsize_t i = 0; i<numObjs; ++i)
    {
        H5std_string tName;
        auto oType = g.getObjTypeByIdx(i, tName);
        if (H5G_GROUP == oType)
        {
            H5std_string gName = g.getObjnameByIdx(i);
            H5::Group gNode = g.openGroup(gName);

            const auto &v = H5DB::GetAttribute<std::string>(gNode, CommonDef::GetSpamDBNodeTypeAttrName());
            wxString wNodeName = wxString::FromUTF8(gName);

            auto child = nf.Create(v, me, wNodeName);
            Append(child);
            child->Load(gNode, nf, child);
        }
    }

    drawStyle_.strokeColor_ = H5DB::GetAttribute<wxColor>(g,     std::string("StrokeColor"));
    drawStyle_.fillColor_   = H5DB::GetAttribute<wxColor>(g,     std::string("FillColor"));
    drawStyle_.strokeWidth_ = H5DB::GetAttribute<double>(g,        std::string("StrokeWidth"));
    visible_                = H5DB::GetAttribute<bool>(g,        std::string("Visible"));
    locked_                 = H5DB::GetAttribute<bool>(g,        std::string("Locked"));
    rank_                   = H5DB::GetAttribute<long>(g,        std::string("Rank"));
    auto strUUID            = H5DB::GetAttribute<std::string>(g, std::string("UUID"));

    try
    {
        boost::uuids::string_generator gen;
        tag_ = gen(strUUID);
    }
    catch(const std::runtime_error &e)
    {
        wxLogError(e.what());
    }
}

SPModelNode ModelNode::FindChild(const ModelNode *const child)
{
    for (auto itCld = children_.begin(); itCld != children_.end(); ++itCld)
    {
        if (itCld->get() == child)
        {
            return *itCld;
        }
    }

    return SPModelNode();
}

void ModelNode::RemoveChild(const ModelNode *const child)
{
    for (auto itCld=children_.begin(); itCld!= children_.end(); ++itCld)
    {
        if (itCld->get()==child)
        {
            children_.erase(itCld);
            return;
        }
    }
}

void ModelNode::GetAllAncestors(SPModelNodeVector &ances) const
{
    auto cPar = parent_.lock();
    while (cPar)
    {
        ances.push_back(cPar);
        cPar = cPar->GetParent();
    }
}

std::string ModelNode::GetUUIDTag() const
{
    return boost::uuids::to_string(tag_);
}