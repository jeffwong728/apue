#include "linenode.h"
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

LineNode::LineNode(const SPModelNode &parent, const wxString &title)
    : GeomNode(parent, title)
{
    InitData(data_);
}

LineNode::~LineNode()
{
}

bool LineNode::IsTypeOf(const SpamEntityType t) const
{
    switch (t)
    {
    case SpamEntityType::kET_GEOM:
    case SpamEntityType::kET_GEOM_REGION:
    case SpamEntityType::kET_GEOM_LINE:
        return true;

    default: return false;
    }
}

bool LineNode::IsLegalHit(const SpamEntityOperation entityOp) const
{
    switch (entityOp)
    {
    case SpamEntityOperation::kEO_GENERAL:
    case SpamEntityOperation::kEO_GEOM_TRANSFORM:
    case SpamEntityOperation::kEO_VERTEX_MOVE:
        return true;

    default:
        return false;
    }
}

void LineNode::BuildPath(Geom::PathVector &pv) const
{
    Geom::Point p0{ data_.points[0][0], data_.points[0][1] };
    Geom::Point p1{ data_.points[1][0], data_.points[1][1] };

    Geom::PathBuilder pb(pv);
    pb.moveTo(p0);
    pb.lineTo(p1);
    pb.flush();
}

void LineNode::BuildNode(Geom::PathVector &pv, NodeIdVector &ids) const
{
    if (selData_.ss == SelectionState::kSelNodeEdit)
    {
        Geom::Point pts[] = {
            Geom::Point(data_.points[0][0], data_.points[0][1]),
            Geom::Point(data_.points[1][0], data_.points[1][1])
        };

        pv.push_back(Geom::Path(Geom::Circle(pts[0], 3)));
        pv.push_back(Geom::Path(Geom::Circle(pts[1], 3)));
        ids.push_back({ 0, 0 });
        ids.push_back({ 1, 0 });
    }
}

void LineNode::BuildEdge(CurveVector &pth, NodeIdVector &ids) const
{
    if (selData_.ss == SelectionState::kSelNodeEdit)
    {
        Geom::Point pts[] = {
            Geom::Point(data_.points[0][0], data_.points[0][1]),
            Geom::Point(data_.points[1][0], data_.points[1][1])
        };

        pth.push_back(std::make_unique<Geom::LineSegment>(pts[0], pts[1]));
        ids.push_back({ 0, 0 });
    }
}

SelectionData LineNode::HitTest(const Geom::Point &pt) const
{
    SelectionData sd{ selData_.ss, HitState::kHsNone, -1, -1};
    Geom::PathVector pv;
    BuildPath(pv);

    if (!pv.empty())
    {
        const Geom::Curve &curve = pv.front().front();
        Geom::Coord t = curve.nearestTime(pt);
        Geom::Coord dist = Geom::distanceSq(pt, curve.pointAt(t));
        if (dist < 9)
        {
            sd.hs = HitState::kHsFace;
            sd.id = 0;
            sd.subid = 0;
            sd.master = 0;
        }
    }

    return sd;
}

SelectionData LineNode::HitTest(const Geom::Point &pt, const double sx, const double sy) const
{
    return DrawableNode::HitTest(pt, sx, sy);
}

bool LineNode::IsHitFace(const Geom::Point &pt, const Geom::PathVector &pv) const
{
    if (!pv.empty())
    {
        const Geom::Curve &curve = pv.front().front();
        Geom::Coord t = curve.nearestTime(pt);
        Geom::Coord dist = Geom::distanceSq(pt, curve.pointAt(t));
        if (dist < 9)
        {
            return true;
        }
    }

    return false;
}

bool LineNode::IsIntersection(const Geom::Rect &box) const
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

void LineNode::StartTransform()
{
    base_ = data_;
    DrawableNode::StartTransform();
}

void LineNode::EndTransform()
{
    InitData(base_);
    DrawableNode::EndTransform();
}

void LineNode::ResetTransform()
{
    data_ = base_;
    InitData(base_);
    DrawableNode::EndTransform();
}

void LineNode::NodeEdit(const Geom::Point &anchorPt, const Geom::Point &freePt, const Geom::Point &lastPt)
{
    Geom::Coord dx = freePt.x() - lastPt.x();
    Geom::Coord dy = freePt.y() - lastPt.y();
    if (HitState::kHsNode == selData_.hs)
    {
        if (selData_.id>-1 && selData_.id<2)
        {
            Geom::Point pts[] = {
                Geom::Point(data_.points[0][0], data_.points[0][1]),
                Geom::Point(data_.points[1][0], data_.points[1][1])
            };

            data_.points[selData_.id][0] += dx;
            data_.points[selData_.id][1] += dy;
        }
    }
    else if (HitState::kHsEdge == selData_.hs)
    {
        data_.points[0][0] += dx;
        data_.points[0][1] += dy;
        data_.points[1][0] += dx;
        data_.points[1][1] += dy;
    }
    else
    {
        Geom::Affine aff = Geom::Affine::identity();
        aff *= Geom::Translate(dx, dy);
        LineNode::DoTransform(aff, dx, dy);
    }
}

void LineNode::ResetNodeEdit()
{
}

boost::any LineNode::CreateMemento() const
{
    auto mem = std::make_shared<Memento>();
    mem->style   = drawStyle_;
    mem->data    = data_;
    mem->rank    = rank_;
    mem->visible = visible_;
    mem->locked  = locked_;

    return mem;
}

bool LineNode::RestoreFromMemento(const boost::any &memento)
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

void LineNode::InitData(LineData &data)
{
    Geom::Affine ide = Geom::Affine::identity();
    for (int i = 0; i < data.transform.size(); ++i)
    {
        data.transform[i] = ide[i];
    }

    data.points.fill({ 0, 0 });
}

void LineNode::Save(const H5::Group &g) const
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
        ModelNode::Save(cwg);
    }
}

void LineNode::Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me)
{
    H5DB::Load(g, std::string("Transform"), data_.transform);
    std::vector<std::array<double, 2>> points;
    H5DB::Load(g, std::string("Points"), points);

    if (2 == points.size())
    {
        std::copy(points.cbegin(), points.cend(), data_.points.begin());
    }

    ModelNode::Load(g, nf, me);
}

void LineNode::DoTransform(const Geom::Affine &aff, const double dx, const double dy)
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