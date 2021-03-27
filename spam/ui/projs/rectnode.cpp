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
    InitData(data_);
}

RectNode::~RectNode()
{
}

bool RectNode::IsTypeOf(const SpamEntityType t) const
{
    switch (t)
    {
    case SpamEntityType::kET_GEOM:
    case SpamEntityType::kET_GEOM_RECT:
        return true;

    default: return false;
    }
}

bool RectNode::IsLegalHit(const SpamEntityOperation entityOp) const
{
    switch (entityOp)
    {
    case SpamEntityOperation::kEO_GENERAL:
    case SpamEntityOperation::kEO_GEOM_CREATE:
    case SpamEntityOperation::kEO_GEOM_TRANSFORM:
    case SpamEntityOperation::kEO_VERTEX_MOVE:
        return true;

    default:
        return false;
    }
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

void RectNode::BuildNode(Geom::PathVector &pv, NodeIdVector &ids, const double sx, const double sy) const
{
    if (selData_.ss == SelectionState::kSelNodeEdit)
    {
        Geom::Point pts[] = {
            Geom::Point(data_.points[0][0], data_.points[0][1]),
            Geom::Point(data_.points[1][0], data_.points[1][1]),
            Geom::Point(data_.points[2][0], data_.points[2][1]),
            Geom::Point(data_.points[3][0], data_.points[3][1])
        };


        const double w = Geom::distance(pts[0], pts[1]);
        const double h = Geom::distance(pts[0], pts[3]);

        const int indices[4][2] = { { 3, 1 },{ 0, 2 },{ 1, 3 },{ 2, 0 } };
        const double lens[4][2] = { { h, w },{ w, h },{ h, w },{ w, h } };
        const double radi[4][2] = {
            { data_.radii[0][1], data_.radii[0][0] },
            { data_.radii[1][0], data_.radii[1][1] },
            { data_.radii[2][1], data_.radii[2][0] },
            { data_.radii[3][0], data_.radii[3][1] }
        };

        for (int c = 0; c<4; ++c)
        {
            if (selData_.master == c)
            {
                Geom::Point r0 = Geom::lerp(radi[c][0] / lens[c][0], pts[c], pts[indices[c][0]]);
                Geom::Point r1 = Geom::lerp(radi[c][1] / lens[c][1], pts[c], pts[indices[c][1]]);
                pv.push_back(Geom::Path(Geom::Circle(r0, 3*sx)));
                pv.push_back(Geom::Path(Geom::Circle(r1, 3*sx)));
                ids.push_back({ c, 1 });
                ids.push_back({ c, 2 });
            }
            else
            {
                Geom::Point a = pts[c] + Geom::Point(-3*sx, -3*sx);
                Geom::Point b = pts[c] + Geom::Point(3*sx, 3*sx);
                pv.push_back(Geom::Path(Geom::Rect(a, b)));
                ids.push_back({ c, 0 });
            }
        }
    }
    pv *= Geom::Translate(0.5, 0.5);
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
            sd.master = 0;
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

void RectNode::StartTransform()
{
    base_ = data_;
    DrawableNode::StartTransform();
}

void RectNode::EndTransform()
{
    InitData(base_);
    DrawableNode::EndTransform();
}

void RectNode::ResetTransform()
{
    data_ = base_;
    InitData(base_);
    DrawableNode::EndTransform();
}

void RectNode::NodeEdit(const Geom::Point &anchorPt, const Geom::Point &freePt, const Geom::Point &lastPt)
{
    Geom::Coord dx = freePt.x() - lastPt.x();
    Geom::Coord dy = freePt.y() - lastPt.y();
    if (HitState::kHsNode == selData_.hs)
    {
        if (selData_.id>-1 && selData_.id<4)
        {
            if ((1 == selData_.subid || 2 == selData_.subid))
            {
                const int indices[4][2] = { { 3, 1 },{ 0, 2 },{ 1, 3 },{ 2, 0 } };
                Geom::Point spt{ data_.points[selData_.id][0], data_.points[selData_.id][1] };
                Geom::Point ept{ data_.points[indices[selData_.id][selData_.subid - 1]][0], data_.points[indices[selData_.id][selData_.subid - 1]][1] };

                Geom::Line ln{ spt, ept };
                Geom::Coord t = std::max(.0, std::min(0.5, ln.timeAtProjection(freePt)));
                Geom::Point r = Geom::lerp(t, spt, ept);

                const double radi[4][2] = { { 1, 0 },{ 0, 1 },{ 1, 0 },{ 0, 1 } };
                data_.radii[selData_.id][radi[selData_.id][selData_.subid - 1]] = Geom::distance(r, spt);
            }
            else
            {
                Geom::Point pts[] = {
                    Geom::Point(data_.points[0][0], data_.points[0][1]),
                    Geom::Point(data_.points[1][0], data_.points[1][1]),
                    Geom::Point(data_.points[2][0], data_.points[2][1]),
                    Geom::Point(data_.points[3][0], data_.points[3][1])
                };

                const int oindices[4] = { 2, 3, 0, 1 };
                const int sindices[4] = { 3, 0, 1, 2 };
                const int rindices[4] = { 1, 2, 3, 0 };
                const int lindices[4][4] = { {1, 2, 3, 0}, {2, 3, 1, 0}, {0, 3, 1, 2}, {1, 0, 2, 3} };
                Geom::Point mpt{ data_.points[selData_.id][0] + dx,  data_.points[selData_.id][1] + dy };
                Geom::Point opt{ data_.points[oindices[selData_.id]][0],  data_.points[oindices[selData_.id]][1] };
                Geom::Line sln{ pts[lindices[selData_.id][0]], pts[lindices[selData_.id][1]] };
                Geom::Line eln{ pts[lindices[selData_.id][2]], pts[lindices[selData_.id][3]] };
                Geom::Point sp = Geom::projection(mpt, sln);
                Geom::Point ep = Geom::projection(mpt, eln);
                Geom::Line pln{ sp, ep };
                Geom::Coord t = pln.timeAt(mpt);

                Geom::Point spt = Geom::lerp(t, pts[oindices[selData_.id]], pts[sindices[selData_.id]]);
                Geom::Point cpt = Geom::lerp(0.5, mpt, pts[oindices[selData_.id]]);
                Geom::Point rpt = Geom::lerp(2, spt, cpt);

                data_.points[selData_.id][0] = mpt.x();
                data_.points[selData_.id][1] = mpt.y();
                data_.points[sindices[selData_.id]][0] = spt.x();
                data_.points[sindices[selData_.id]][1] = spt.y();
                data_.points[rindices[selData_.id]][0] = rpt.x();
                data_.points[rindices[selData_.id]][1] = rpt.y();
                ConstrainRadii();
            }
        }
    }
    else
    {
        Geom::Affine aff = Geom::Affine::identity();
        aff *= Geom::Translate(dx, dy);
        RectNode::DoTransform(aff, dx, dy);
    }
}

void RectNode::ResetNodeEdit()
{
}

boost::any RectNode::CreateMemento() const
{
    auto mem = std::make_shared<Memento>();
    mem->style   = drawStyle_;
    mem->data    = data_;
    mem->rank    = rank_;
    mem->visible = visible_;
    mem->locked  = locked_;

    return mem;
}

bool RectNode::RestoreFromMemento(const boost::any &memento)
{
    try
    {
        auto mem = boost::any_cast<std::shared_ptr<Memento>>(memento);

        if (mem)
        {
            drawStyle_ = mem->style;
            data_      = mem->data;
            rank_      = mem->rank;
            visible_   = mem->visible;
            locked_    = mem->locked;
        }

        return true;
    }
    catch (const boost::bad_any_cast &)
    {
        return false;
    }
}

void RectNode::InitData(RectData &data)
{
    Geom::Affine ide = Geom::Affine::identity();
    for (int i = 0; i < data.transform.size(); ++i)
    {
        data.transform[i] = ide[i];
    }

    data.points.fill({ 0, 0 });
    data.radii.fill({ 0, 0 });
}

void RectNode::PyDoTransform(const Geom::Affine &aff)
{
    if (!aff.isIdentity())
    {
        for (int i = 0; i < static_cast<int>(data_.points.size()); ++i)
        {
            Geom::Point pt2{ data_.points[i][0], data_.points[i][1] };
            pt2 *= aff;
            data_.points[i][0] = pt2.x();
            data_.points[i][1] = pt2.y();
        }

        ConstrainRadii();
    }
}

Geom::Point RectNode::GetCenter() const
{
    Geom::Point pt0{ data_.points[0][0], data_.points[0][1] };
    Geom::Point pt2{ data_.points[2][0], data_.points[2][1] };

    return Geom::lerp(0.5, pt0, pt2);
}

void RectNode::Save(const H5::Group &g) const
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

void RectNode::DoTransform(const Geom::Affine &aff, const double dx, const double dy)
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

            ConstrainRadii();
        }
    }
}

void RectNode::ConstrainRadii()
{
    Geom::Point p0{ data_.points[0][0], data_.points[0][1] };
    Geom::Point p1{ data_.points[1][0], data_.points[1][1] };
    Geom::Point p2{ data_.points[2][0], data_.points[2][1] };
    Geom::Point p3{ data_.points[3][0], data_.points[3][1] };

    const double w = Geom::distance(p0, p1);
    const double h = Geom::distance(p0, p3);
    const double w2 = w / 2;
    const double h2 = h / 2;
    for (auto &r : data_.radii)
    {
        r[0] = std::min(w2, r[0]);
        r[1] = std::min(h2, r[1]);
    }
}