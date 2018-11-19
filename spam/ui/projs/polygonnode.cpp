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
#include <algorithm>

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

void PolygonNode::BuildNode(Geom::PathVector &pv, NodeIdVector &ids) const
{
    if (selData_.ss == SelectionState::kSelNodeEdit)
    {
        for (int c = 0; c<GetNumCorners(); ++c)
        {
            Geom::Point pt{ data_.points[c][0], data_.points[c][1] };
            Geom::Point a = pt + Geom::Point(-3, -3);
            Geom::Point b = pt + Geom::Point(3, 3);
            pv.push_back(Geom::Path(Geom::Rect(a, b)));
            ids.push_back({ c, 0 });
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

void PolygonNode::StartTransform()
{
    base_ = data_;
    DrawableNode::StartTransform();
}

void PolygonNode::EndTransform()
{
    base_.points.clear();
    Geom::Affine ide = Geom::Affine::identity();
    for (int i = 0; i < data_.transform.size(); ++i)
    {
        base_.transform[i] = ide[i];
    }

    DrawableNode::EndTransform();
}

void PolygonNode::ResetTransform()
{
    data_ = base_;
    PolygonNode::EndTransform();
}

void PolygonNode::NodeEdit(const Geom::Point &anchorPt, const Geom::Point &freePt, const double dx, const double dy)
{
    if (HitState::kHsNode == selData_.hs)
    {
        if (selData_.id>-1 && selData_.id<GetNumCorners())
        {
            data_.points[selData_.id][0] += dx;
            data_.points[selData_.id][1] += dy;
        }
    }
    else
    {
        Geom::Affine aff = Geom::Affine::identity();
        aff *= Geom::Translate(dx, dy);
        PolygonNode::DoTransform(aff, dx, dy);
    }
}

void PolygonNode::ResetNodeEdit()
{
}

boost::any PolygonNode::CreateMemento() const
{
    auto mem = std::make_shared<Memento>();
    mem->style   = drawStyle_;
    mem->data    = data_;
    mem->rank    = rank_;
    mem->visible = visible_;
    mem->locked  = locked_;

    return mem;
}

bool PolygonNode::RestoreFromMemento(const boost::any &memento)
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

void PolygonNode::DoTransform(const Geom::Affine &aff, const double dx, const double dy)
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