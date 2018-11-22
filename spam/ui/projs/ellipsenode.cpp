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
#include <boost/algorithm/string.hpp>
#include <cmath>

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
    Geom::Point  p0{ data_.points[0][0], data_.points[0][1] };
    Geom::Point  p1{ data_.points[1][0], data_.points[1][1] };
    Geom::Point  p2{ data_.points[2][0], data_.points[2][1] };
    Geom::Point  p3{ data_.points[3][0], data_.points[3][1] };
    Geom::Point  cp  = Geom::lerp(0.5, p0, p2);

    Geom::Point a = p0 - p3;
    Geom::Point b = p2 - p3;
    double vc = Geom::cross(a, b);
    double vd = Geom::dot(a, b);

    if (std::abs(vc)>Geom::EPSILON)
    {
        Geom::Affine aff = Geom::Affine::identity();
        aff *= Geom::Translate(-cp);
        aff *= (Geom::Rotate(p1 - p0).inverse());
        aff *= Geom::HShear(vd / vc);
        Geom::Affine raff = aff.inverse();

        Geom::Point rp0 = p0 * aff;
        Geom::Point rp1 = p1 * aff;
        Geom::Point rp2 = p2 * aff;
        Geom::Point rp3 = p3 * aff;
        double rx = (rp1.x() - rp0.x())/2;
        double ry = (rp3.y() - rp0.y())/2;
        Geom::Ellipse e(0, 0, rx, ry, 0);

        double dAngle = std::remainder(data_.angles[1] - data_.angles[0], 360.0);
        if (dAngle<0) dAngle += 360;
        if (dAngle<Geom::EPSILON)
        {
            Geom::Path ePath(e);
            ePath *= raff;
            pv.push_back(ePath);
        }
        else
        {
            Geom::PathBuilder pb(pv);
            Geom::Point saPt = e.pointAt(Geom::rad_from_deg(data_.angles[0]));
            Geom::Point eaPt = e.pointAt(Geom::rad_from_deg(data_.angles[1]));

            if (GenericEllipseArcType::kAtSlice == data_.type)
            {
                pb.moveTo(Geom::Point(0, 0));
                pb.lineTo(saPt);
            }
            else
            {
                pb.moveTo(saPt);
            }

            if (data_.angles[1]>data_.angles[0])
            {
                double da = data_.angles[1] - data_.angles[0];
                pb.arcTo(rx, ry, 0, da>180, true, eaPt);
            }
            else
            {
                double da = data_.angles[0] - data_.angles[1];
                pb.arcTo(rx, ry, 0, da<180, true, eaPt);
            }
            pb.closePath();
            pv *= raff;
        }
    }
}

void GenericEllipseArcNode::BuildNode(Geom::PathVector &pv, NodeIdVector &ids) const
{
    if (selData_.ss == SelectionState::kSelNodeEdit)
    {
        Geom::Point  p0{ data_.points[0][0], data_.points[0][1] };
        Geom::Point  p1{ data_.points[1][0], data_.points[1][1] };
        Geom::Point  p2{ data_.points[2][0], data_.points[2][1] };
        Geom::Point  p3{ data_.points[3][0], data_.points[3][1] };
        Geom::Point  cp = Geom::lerp(0.5, p0, p2);

        Geom::Point a = p0 - p3;
        Geom::Point b = p2 - p3;
        double vc = Geom::cross(a, b);
        double vd = Geom::dot(a, b);

        if (std::abs(vc)>Geom::EPSILON)
        {
            Geom::Affine aff = Geom::Affine::identity();
            aff *= Geom::Translate(-cp);
            aff *= (Geom::Rotate(p1 - p0).inverse());
            aff *= Geom::HShear(vd / vc);

            Geom::Point rp0 = p0 * aff;
            Geom::Point rp1 = p1 * aff;
            Geom::Point rp2 = p2 * aff;
            Geom::Point rp3 = p3 * aff;
            Geom::Affine raff = aff.inverse();

            Geom::Point tPt = Geom::lerp(0.5, rp0, rp1)*raff;
            Geom::Point lPt = Geom::lerp(0.5, rp0, rp3)*raff;

            double rw = rp1.x() - rp0.x();
            double rh = rp3.y() - rp0.y();
            Geom::Ellipse e(0, 0, rw / 2, rh / 2, 0);
            Geom::Point saPt = e.pointAt(Geom::rad_from_deg(data_.angles[0]))*raff;
            Geom::Point eaPt = e.pointAt(Geom::rad_from_deg(data_.angles[1]))*raff;

            pv.push_back(Geom::Path(Geom::Rect(tPt + Geom::Point(-3, -3), tPt + Geom::Point(3, 3))));
            pv.push_back(Geom::Path(Geom::Rect(lPt + Geom::Point(-3, -3), lPt + Geom::Point(3, 3))));
            pv.push_back(Geom::Path(Geom::Circle(saPt, 3)));
            pv.push_back(Geom::Path(Geom::Circle(eaPt, 3)));

            ids.push_back({ 0, 0 });
            ids.push_back({ 0, 1 });
            ids.push_back({ 1, 0 });
            ids.push_back({ 1, 1 });
        }
    }
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

void GenericEllipseArcNode::NodeEdit(const Geom::Point &anchorPt, const Geom::Point &freePt, const Geom::Point &lastPt)
{
    if (HitState::kHsNode == selData_.hs)
    {
        Geom::Point  p0{ data_.points[0][0], data_.points[0][1] };
        Geom::Point  p1{ data_.points[1][0], data_.points[1][1] };
        Geom::Point  p2{ data_.points[2][0], data_.points[2][1] };
        Geom::Point  p3{ data_.points[3][0], data_.points[3][1] };
        Geom::Point  cp = Geom::lerp(0.5, p0, p2);

        Geom::Point a = p0 - p3;
        Geom::Point b = p2 - p3;
        double vc = Geom::cross(a, b);
        double vd = Geom::dot(a, b);

        if (std::abs(vc) > Geom::EPSILON)
        {
            Geom::Affine aff = Geom::Affine::identity();
            aff *= Geom::Translate(-cp);
            aff *= (Geom::Rotate(p1 - p0).inverse());
            aff *= Geom::HShear(vd / vc);

            Geom::Point rp0 = p0 * aff;
            Geom::Point rp1 = p1 * aff;
            Geom::Point rp2 = p2 * aff;
            Geom::Point rp3 = p3 * aff;
            Geom::Point rFreePt = freePt * aff;
            Geom::Point rLastPt = lastPt * aff;

            if (selData_.id == 1)
            {
                if (selData_.subid == 0)
                {
                    data_.angles[0] = Geom::deg_from_rad(Geom::atan2(rFreePt));
                    if (data_.angles[0]<0)
                    {
                        data_.angles[0] += 360;
                    }
                }
                else
                {
                    data_.angles[1] = Geom::deg_from_rad(Geom::atan2(rFreePt));
                    if (data_.angles[1]<0)
                    {
                        data_.angles[1] += 360;
                    }
                }

                double rx = (rp1.x() - rp0.x()) / 2;
                double ry = (rp3.y() - rp0.y()) / 2;
                Geom::Ellipse e(0, 0, rx, ry, 0);
                if (e.contains(rFreePt))
                {
                    data_.type = GenericEllipseArcType::kAtChord;
                }
                else
                {
                    data_.type = GenericEllipseArcType::kAtSlice;
                }
            }
            else
            {
                Geom::Point deltaPt = rFreePt - rLastPt;
                if (selData_.subid == 0)
                {
                    rp0.y() += deltaPt.y();
                    rp1.y() += deltaPt.y();
                    rp2.y() -= deltaPt.y();
                    rp3.y() -= deltaPt.y();
                }
                else
                {
                    rp0.x() += deltaPt.x();
                    rp3.x() += deltaPt.x();
                    rp1.x() -= deltaPt.x();
                    rp2.x() -= deltaPt.x();
                }

                Geom::Affine raff = aff.inverse();
                p0 = rp0 * raff;
                p1 = rp1 * raff;
                p2 = rp2 * raff;
                p3 = rp3 * raff;

                data_.points[0][0] = p0.x();
                data_.points[0][1] = p0.y();
                data_.points[1][0] = p1.x();
                data_.points[1][1] = p1.y();
                data_.points[2][0] = p2.x();
                data_.points[2][1] = p2.y();
                data_.points[3][0] = p3.x();
                data_.points[3][1] = p3.y();
            }
        }
    }
    else
    {
        Geom::Coord dx = freePt.x() - lastPt.x();
        Geom::Coord dy = freePt.y() - lastPt.y();
        Geom::Affine aff = Geom::Affine::identity();
        aff *= Geom::Translate(dx, dy);
        GenericEllipseArcNode::DoTransform(aff, dx, dy);
    }
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
    try
    {
        auto mem = boost::any_cast<std::shared_ptr<Memento>>(memento);

        if (mem)
        {
            drawStyle_ = mem->style;
            data_ = mem->data;
            rank_ = mem->rank;
            visible_ = mem->visible;
            locked_ = mem->locked;
        }

        return true;
    }
    catch (const boost::bad_any_cast &)
    {
        return false;
    }
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
        H5DB::SetAttribute(cwg, std::string("ArcType"), GetArcTypeName());
        H5DB::Save(cwg, std::string("Transform"), data_.transform);
        H5DB::Save(cwg, std::string("Angles"), data_.angles);
        H5DB::Save(cwg, std::string("Points"),  std::vector<std::array<double, 2>>(data_.points.cbegin(), data_.points.cend()));
        ModelNode::Save(cwg);
    }
}

void GenericEllipseArcNode::Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me)
{
    H5DB::Load(g, std::string("Transform"), data_.transform);
    H5DB::Load(g, std::string("Angles"), data_.angles);

    std::vector<std::array<double, 2>> points;
    H5DB::Load(g, std::string("Points"), points);
    if (4 == points.size())
    {
        std::copy(points.cbegin(), points.cend(), data_.points.begin());
    }

    std::string tName = H5DB::GetAttribute<std::string>(g, std::string("ArcType"));
    data_.type = GetArcTypeValue(tName);

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

std::string GenericEllipseArcNode::GetArcTypeName() const
{
    switch (data_.type)
    {
    case GenericEllipseArcType::kAtSlice: return std::string("slice");
    case GenericEllipseArcType::kAtChord: return std::string("chord");
    case GenericEllipseArcType::kAtArc: return std::string("arc");
    default: return std::string("slice");
    }
}

GenericEllipseArcType GenericEllipseArcNode::GetArcTypeValue(const std::string &tName) const
{
    const std::string tLower = boost::algorithm::to_lower_copy(tName);
    if (std::string("slice") == tLower)
    {
        return GenericEllipseArcType::kAtSlice;
    }
    else if (std::string("chord") == tLower)
    {
        return GenericEllipseArcType::kAtChord;
    }
    else if (std::string("arc") == tLower)
    {
        return GenericEllipseArcType::kAtArc;
    }
    else
    {
        return GenericEllipseArcType::kAtSlice;
    }
}