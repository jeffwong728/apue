#include "rectnode.h"
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

RectNode::RectNode(const SPModelNode &parent, const wxString &title)
    : GeomNode(parent, title)
{
    InitData();
}

RectNode::~RectNode()
{
}

void RectNode::BuildPath(Geom::PathVector &pv) const
{
    Geom::Point p0{ data_.points[0][0], data_.points[0][1] };
    Geom::Point p1{ data_.points[1][0], data_.points[1][1] };
    Geom::Point p2{ data_.points[2][0], data_.points[2][1] };
    Geom::Point p3{ data_.points[3][0], data_.points[3][1] };
    Geom::Point cp = Geom::lerp(0.5, p0, p2);

    constexpr double C2 = 1-0.554;
    const double w   = Geom::distance(p0, p1);
    const double h   = Geom::distance(p0, p3);
    const double w2  = w / 2;
    const double h2  = h / 2;
    const double tol = 1e-9;

    if (w>tol && h>tol)
    {
        std::array<std::array<double, 2>, 4>  r;
        bool rightAngle[4] = { false, false, false, false };
        for (int p = 0; p<4; ++p)
        {
            if (data_.radii[p][0]>tol && data_.radii[p][1]>tol)
            {
                rightAngle[p] = false;
                r[p][0] = data_.radii[p][0];
                r[p][1] = data_.radii[p][1];
            }
            else
            {
                rightAngle[p] = true;
                r[p][0] = 0.0;
                r[p][1] = 0.0;
            }
        }

        Geom::PathBuilder pb(pv);
        if (rightAngle[0] && rightAngle[1] && rightAngle[2] && rightAngle[3]) {
            pb.moveTo(p0);
            pb.lineTo(p1);
            pb.lineTo(p2);
            pb.lineTo(p3);
        }
        else
        {
            pb.moveTo(Geom::lerp(r[0][0] / w, p0, p1));
            if (rightAngle[0] || rightAngle[1] || r[0][0]<w2 || r[1][0]<w2)
            {
                pb.lineTo(Geom::lerp(1 - r[1][0] / w, p0, p1));
            }

            if (!rightAngle[1])
            {
                pb.curveTo(Geom::lerp(1 - r[1][0] * C2 / w, p0, p1), Geom::lerp(r[1][1] * C2 / h, p1, p2), Geom::lerp(r[1][1] / h, p1, p2));
            }

            if (rightAngle[1] || rightAngle[2] || r[1][1]<h2 || r[2][1]<h2)
            {
                pb.lineTo(Geom::lerp(1 - r[2][1] / h, p1, p2));
            }

            if (!rightAngle[2])
            {
                pb.curveTo(Geom::lerp(1 - r[2][1] * C2 / h, p1, p2), Geom::lerp(r[2][0] * C2 / w, p2, p3), Geom::lerp(r[2][0] / w, p2, p3));
            }

            if (rightAngle[2] || rightAngle[3] || r[2][0]<w2 || r[3][0]<w2)
            {
                pb.lineTo(Geom::lerp(1 - r[3][0] / w, p2, p3));
            }

            if (!rightAngle[3])
            {
                pb.curveTo(Geom::lerp(1 - r[3][0] * C2 / w, p2, p3), Geom::lerp(r[3][1] * C2 / h, p3, p0), Geom::lerp(r[3][1] / h, p3, p0));
            }

            if (rightAngle[3] || rightAngle[0] || r[3][1]<h2 || r[0][1]<h2)
            {
                pb.lineTo(Geom::lerp(1 - r[0][1] / h, p3, p0));
            }

            if (!rightAngle[0])
            {
                pb.curveTo(Geom::lerp(1 - r[0][1] * C2 / h, p3, p0), Geom::lerp(r[0][0] * C2 / w, p0, p1), Geom::lerp(r[0][0] / w, p0, p1));
            }
        }

        pb.closePath();
    }
}

void RectNode::BuildCorners(const Geom::PathVector &pv, Geom::Point(&corners)[4]) const
{
    corners[0] = { data_.points[0][0], data_.points[0][1] };
    corners[1] = { data_.points[1][0], data_.points[1][1] };
    corners[2] = { data_.points[2][0], data_.points[2][1] };
    corners[3] = { data_.points[3][0], data_.points[3][1] };
}

SelectionData RectNode::HitTest(const Geom::Point &pt) const
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
        }
    }

    return sd;
}

SelectionData RectNode::HitTest(const Geom::Point &pt, const double sx, const double sy) const
{
    return DrawableNode::HitTest(pt, sx, sy);
}

bool RectNode::IsIntersection(const Geom::Rect &box) const
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

void RectNode::Translate(const double dx, const double dy)
{
    for (auto &pt : data_.points)
    {
        pt[0] += dx;
        pt[1] += dy;
    }
}

void RectNode::Transform(const Geom::Point &anchorPt, const Geom::Point &freePt, const double dx, const double dy)
{
    for (auto &pt : data_.points)
    {
        pt[0] += dx;
        pt[1] += dy;
    }
}

void RectNode::InitData()
{
    Geom::Affine ide = Geom::Affine::identity();
    for (int i = 0; i < data_.transform.size(); ++i)
    {
        data_.transform[i] = ide[i];
    }

    data_.points.fill({ 0, 0 });
    data_.radii.fill({ 0, 0 });
}

void RectNode::Save(const H5::Group &g) const
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
        H5DB::Save(cwg, std::string("Points"), std::vector<std::array<double, 2>>(data_.points.cbegin(), data_.points.cend()));
        H5DB::Save(cwg, std::string("Radii"),  std::vector<std::array<double, 2>>(data_.radii.cbegin(), data_.radii.cend()));
        ModelNode::Save(cwg);
    }
}

void RectNode::Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me)
{
    H5DB::Load(g, std::string("Transform"), data_.transform);
    std::vector<std::array<double, 2>> points, radii;
    H5DB::Load(g, std::string("Points"), points);
    H5DB::Load(g, std::string("Radii"), radii);

    if (4 == points.size())
    {
        std::copy(points.cbegin(), points.cend(), data_.points.begin());
    }

    if (4 == radii.size())
    {
        std::copy(radii.cbegin(), radii.cend(), data_.radii.begin());
    }

    ModelNode::Load(g, nf, me);
}