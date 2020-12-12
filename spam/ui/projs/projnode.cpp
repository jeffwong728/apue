#include "projnode.h"
#include <helper/h5db.h>
#include <helper/commondef.h>

ProjNode::ProjNode(const SPModelNode &parent, const wxString &title)
    : ModelNode(parent, title)
{
}

ProjNode::~ProjNode()
{
}

void ProjNode::Save(const H5::Group &g) const
{
    std::string utf8Title(title_.ToUTF8().data());
    if (g.nameExists(utf8Title))
    {
        H5Ldelete(g.getId(), utf8Title.data(), H5P_DEFAULT);
    }

    if (!g.nameExists(utf8Title))
    {
        H5::LinkCreatPropList lcpl;
        lcpl.setCharEncoding(H5T_CSET_UTF8);
        H5::Group cwg = g.createGroup(utf8Title, lcpl);
        H5DB::SetAttribute(cwg, CommonDef::GetSpamDBNodeTypeAttrName(), GetTypeName());
        H5DB::SetAttribute(cwg, CommonDef::GetProjPerspectiveAttrName(), perspective_);
        ModelNode::Save(cwg);
    }
}

void ProjNode::Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me)
{
    perspective_ = H5DB::GetAttribute<std::string>(g, CommonDef::GetProjPerspectiveAttrName());
    ModelNode::Load(g, nf, me);
}