#include "polygonnode.h"
#include <ui/evts.h>
#include <helper/h5db.h>
#include <helper/commondef.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/2geom.h>
#include <2geom/path-sink.h>
#include <2geom/path-intersection.h>
#include <2geom/svg-path-parser.h>
#pragma warning( pop )

PolygonNode::PolygonNode(const SPModelNode &parent, const wxString &title)
    : GeomNode(parent, title)
{
    Geom::Affine ide = Geom::Affine::identity();
    for (int i = 0; i < data_.transform.size(); ++i)
    {
        data_.transform[i] = ide[i];
    }
}

PolygonNode::~PolygonNode()
{
}

void PolygonNode::BuildPath(Geom::PathVector &pv) const
{
    Geom::PathBuilder pb(pv);
    if (GetNumCorners()>0)
    {
        pb.moveTo(Geom::Point(data_.points.front()[0], data_.points.front()[1]));
        for (int c = 1; c<GetNumCorners(); ++c)
        {
            pb.lineTo(Geom::Point(data_.points[c][0], data_.points[c][1]));
        }

        if (GetNumCorners()>2)
        {
            pb.closePath();
        }
        else
        {
            pb.flush();
        }
    }
}

SelectionData PolygonNode::HitTest(const Geom::Point &pt) const
{
    SelectionData sd{ selData_.ss , HitState ::kHsNone, -1, -1};
    Geom::PathVector pv;
    BuildPath(pv);

    if (Geom::contains(pv.front(), pt))
    {
        sd.hs    = HitState::kHsFace;
        sd.id    = 0;
        sd.subid = 0;
    }

    return sd;
}

SelectionData PolygonNode::HitTest(const Geom::Point &pt, const double sx, const double sy) const
{
    return DrawableNode::HitTest(pt, sx, sy);
}

bool PolygonNode::IsIntersection(const Geom::Rect &box) const
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

void PolygonNode::Translate(const double dx, const double dy)
{
    for (auto &pt : data_.points)
    {
        pt[0] += dx;
        pt[1] += dy;
    }
}

void PolygonNode::Transform(const Geom::Point &anchorPt, const Geom::Point &freePt, const double dx, const double dy)
{
    for (auto &pt : data_.points)
    {
        pt[0] += dx;
        pt[1] += dy;
    }
}

void PolygonNode::AddCorner(const Geom::Point &pt)
{
    data_.points.push_back({ pt.x(), pt.y() });
}

void PolygonNode::PopCorner()
{
    data_.points.pop_back();
}

void PolygonNode::GetCorner(int pos, Geom::Point &corner) const
{
    corner.x() = data_.points[pos][0];
    corner.y() = data_.points[pos][1];
}

void PolygonNode::BuildOpenPath(Geom::PathVector &pv)
{
    Geom::PathBuilder pb(pv);
    if (GetNumCorners()>0)
    {
        pb.moveTo(Geom::Point(data_.points.front()[0], data_.points.front()[1]));
        for (int c = 1; c<GetNumCorners(); ++c)
        {
            pb.lineTo(Geom::Point(data_.points[c][0], data_.points[c][1]));
        }

        pb.flush();
    }
}

void PolygonNode::Save(const H5::Group &g) const
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

        H5DB::Save(cwg, std::string("Points"),    data_.points);
        H5DB::Save(cwg, std::string("Transform"), data_.transform);

        ModelNode::Save(cwg);
    }
}

void PolygonNode::Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me)
{
    H5DB::Load(g, std::string("Points"),    data_.points);
    H5DB::Load(g, std::string("Transform"), data_.transform);

    ModelNode::Load(g, nf, me);
}