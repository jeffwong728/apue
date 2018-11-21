#include "ellipsenode.h"
#include <ui/evts.h>
#include <helper/h5db.h>
#include <helper/commondef.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/2geom.h>
#include <2geom/circle.h>
#include <2geom/path-sink.h>
#include <2geom/path-intersection.h>
#pragma warning( pop )

GenericEllipseArcNode::GenericEllipseArcNode(const SPModelNode &parent, const wxString &title)
    : GeomNode(parent, title)
{
    InitData(data_);
}

GenericEllipseArcNode::~GenericEllipseArcNode()
{
}

void GenericEllipseArcNode::BuildPath(Geom::PathVector &pv) const
{
}

void GenericEllipseArcNode::BuildNode(Geom::PathVector &pv, NodeIdVector &ids) const
{
}

SelectionData GenericEllipseArcNode::HitTest(const Geom::Point &pt) const
{
    SelectionData sd{ selData_.ss, HitState::kHsNone, -1, -1};
    Geom::PathVector pv;
    BuildPath(pv);

    if (!pv.empty())
    {
        if (Geom::contains(pv.front(), pt))
        {
            sd.hs = HitState::kHsFace;
            sd.id = 0;
            sd.subid = 0;
            sd.master = 0;
        }
    }

    return sd;
}

SelectionData GenericEllipseArcNode::HitTest(const Geom::Point &pt, const double sx, const double sy) const
{
    return DrawableNode::HitTest(pt, sx, sy);
}

bool GenericEllipseArcNode::IsIntersection(const Geom::Rect &box) const
{
    Geom::PathVector pv;
    BuildPath(pv);

    Geom::OptRect rect = pv.boundsFast();
    if (rect)
    {
        return box.contains(rect);
    }

    return false;
}

void GenericEllipseArcNode::StartTransform()
{
    base_ = data_;
    DrawableNode::StartTransform();
}

void GenericEllipseArcNode::EndTransform()
{
    InitData(base_);
    DrawableNode::EndTransform();
}

void GenericEllipseArcNode::ResetTransform()
{
    data_ = base_;
    InitData(base_);
    DrawableNode::EndTransform();
}

void GenericEllipseArcNode::NodeEdit(const Geom::Point &anchorPt, const Geom::Point &freePt, const double dx, const double dy)
{
}

void GenericEllipseArcNode::ResetNodeEdit()
{
}

boost::any GenericEllipseArcNode::CreateMemento() const
{
    auto mem = std::make_shared<Memento>();
    mem->style   = drawStyle_;
    mem->data    = data_;
    mem->rank    = rank_;
    mem->visible = visible_;
    mem->locked  = locked_;

    return mem;
}

bool GenericEllipseArcNode::RestoreFromMemento(const boost::any &memento)
{
    return false;
}

void GenericEllipseArcNode::InitData(GenericEllipseArcData &data)
{
    Geom::Affine ide = Geom::Affine::identity();
    for (int i = 0; i < data.transform.size(); ++i)
    {
        data.transform[i] = ide[i];
    }

    data.points.fill({ 0, 0 });
    data.angles[0] = 0;
    data.angles[1] = 360;
    data.type = GenericEllipseArcType::kAtSlice;
}

void GenericEllipseArcNode::Save(const H5::Group &g) const
{
    std::string utf8Title = title_.ToUTF8();
    if (g.nameExists(utf8Title))
    {
        H5Ldelete(g.getId(), utf8Title.data(), H5P_DEFAULT);
    }

    if (!g.nameExists(utf8Title))
    {
        H5::LinkCreatPropList lcpl;
        lcpl.setCharEncoding(H5T_CSET_UTF8);
        H5::Group cwg = g.createGroup(title_, lcpl);
        H5DB::SetAttribute(cwg, CommonDef::GetSpamDBNodeTypeAttrName(), GetTypeName());
        H5DB::Save(cwg, std::string("Transform"), data_.transform);
        H5DB::Save(cwg, std::string("Points"),  std::vector<std::array<double, 2>>(data_.points.cbegin(), data_.points.cend()));
        //H5DB::Save(cwg, std::string("Angles"),  std::vector<std::array<double, 2>>(data_.angles.cbegin(), data_.angles.cend()));
        ModelNode::Save(cwg);
    }
}

void GenericEllipseArcNode::Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me)
{
    H5DB::Load(g, std::string("Transform"), data_.transform);
    std::vector<std::array<double, 2>> points, radii;
    H5DB::Load(g, std::string("Points"), points);
    //H5DB::Load(g, std::string("Radii"), radii);

    if (4 == points.size())
    {
        std::copy(points.cbegin(), points.cend(), data_.points.begin());
    }

    if (4 == radii.size())
    {
        //std::copy(radii.cbegin(), radii.cend(), data_.radii.begin());
    }

    ModelNode::Load(g, nf, me);
}

void GenericEllipseArcNode::DoTransform(const Geom::Affine &aff, const double dx, const double dy)
{
    if (HitState::kHsFace == selData_.hs)
    {
        for (auto &pt : data_.points)
        {
            pt[0] += dx;
            pt[1] += dy;
        }
    }
    else
    {
        if (!aff.isIdentity())
        {
            for (int i = 0; i<static_cast<int>(base_.points.size()); ++i)
            {
                Geom::Point pt2{ base_.points[i][0], base_.points[i][1] };
                pt2 *= aff;
                data_.points[i][0] = pt2.x();
                data_.points[i][1] = pt2.y();
            }
        }
    }
}