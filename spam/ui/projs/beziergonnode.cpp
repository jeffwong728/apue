#include "beziergonnode.h"
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
#include <set>

BeziergonNode::BeziergonNode(const SPModelNode &parent, const wxString &title)
    : GeomNode(parent, title)
{
    InitData(data_);
}

BeziergonNode::~BeziergonNode()
{
}

void BeziergonNode::BuildPath(Geom::PathVector &pv) const
{
    BuildPathImpl(pv, true);
}

void BeziergonNode::BuildNode(Geom::PathVector &pv, NodeIdVector &ids) const
{
    int numCurves = static_cast<int>(data_.points.size());
    if (selData_.ss == SelectionState::kSelNodeEdit && numCurves>1 && data_.points.size() == data_.ntypes.size())
    {
        std::set<int> showCtrls;
        showCtrls.insert((selData_.master - 1 + numCurves) % numCurves);
        showCtrls.insert(selData_.master % numCurves);
        showCtrls.insert((selData_.master + 1) % numCurves);

        for (const int n : showCtrls)
        {
            switch (static_cast<BezierNodeType>(data_.ntypes[n]))
            {
            case BezierNodeType::kBezierPrevCtrl:
                pv.push_back(Geom::Path(Geom::Circle(Geom::Point(data_.points[n][2], data_.points[n][3]), 3)));
                ids.push_back({ n, 1 });
                break;

            case BezierNodeType::kBezierNextCtrl:
                pv.push_back(Geom::Path(Geom::Circle(Geom::Point(data_.points[n][4], data_.points[n][5]), 3)));
                ids.push_back({ n, 2 });
                break;

            case BezierNodeType::kBezierBothCtrl:
                pv.push_back(Geom::Path(Geom::Circle(Geom::Point(data_.points[n][2], data_.points[n][3]), 3)));
                pv.push_back(Geom::Path(Geom::Circle(Geom::Point(data_.points[n][4], data_.points[n][5]), 3)));
                ids.push_back({ n, 1 });
                ids.push_back({ n, 2 });
                break;

            default:
                break;
            }
        }

        int nNext = 0;
        for (const auto &pt : data_.points)
        {
            Geom::Point nPt{ pt[0], pt[1] };
            pv.push_back(Geom::Path(Geom::Rect(nPt + Geom::Point(-3, -3), nPt + Geom::Point(3, 3))));
            ids.push_back({ nNext++, 0 });
        }
    }
}

SelectionData BeziergonNode::HitTest(const Geom::Point &pt) const
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

SelectionData BeziergonNode::HitTest(const Geom::Point &pt, const double sx, const double sy) const
{
    return DrawableNode::HitTest(pt, sx, sy);
}

bool BeziergonNode::IsIntersection(const Geom::Rect &box) const
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

Geom::OptRect BeziergonNode::GetBoundingBox() const
{
    Geom::PathVector pv;
    BuildPath(pv);

    return pv.boundsFast();
}

Geom::OptRect BeziergonNode::GetBoundingBox(const Geom::PathVector &pv) const
{
    return pv.boundsFast();
}

void BeziergonNode::StartTransform()
{
    base_ = data_;
    DrawableNode::StartTransform();
}

void BeziergonNode::EndTransform()
{
    InitData(base_);
    DrawableNode::EndTransform();
}

void BeziergonNode::ResetTransform()
{
    data_ = base_;
    InitData(base_);
    DrawableNode::EndTransform();
}

void BeziergonNode::NodeEdit(const Geom::Point &anchorPt, const Geom::Point &freePt, const Geom::Point &lastPt)
{
}

void BeziergonNode::ResetNodeEdit()
{
}

boost::any BeziergonNode::CreateMemento() const
{
    auto mem = std::make_shared<Memento>();
    mem->style   = drawStyle_;
    mem->data    = data_;
    mem->rank    = rank_;
    mem->visible = visible_;
    mem->locked  = locked_;

    return mem;
}

bool BeziergonNode::RestoreFromMemento(const boost::any &memento)
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

void BeziergonNode::InitData(BezierData &data)
{
    Geom::Affine ide = Geom::Affine::identity();
    for (int i = 0; i < data.transform.size(); ++i)
    {
        data.transform[i] = ide[i];
    }

    data.ntypes.clear();
    data.points.clear();
    data.type = GenericEllipseArcType::kAtChord;
}

void BeziergonNode::BuildPreviewPath(Geom::PathVector &pv) const
{
    BuildPathImpl(pv, false);
    if (!data_.points.empty() && !data_.ntypes.empty())
    {
        Geom::Point selfPt{ data_.points.back()[0], data_.points.back()[1] };
        Geom::Point prevPt{ data_.points.back()[2], data_.points.back()[3] };
        Geom::Point nextPt{ data_.points.back()[4], data_.points.back()[5] };

        Geom::Path pth;
        switch (static_cast<BezierNodeType>(data_.ntypes.back()))
        {
        case BezierNodeType::kBezierPrevCtrl:
            pth.append(Geom::LineSegment(selfPt, prevPt));
            pv.push_back(Geom::Path(Geom::Circle(prevPt, 3)));
            pv.push_back(pth);
            break;

        case BezierNodeType::kBezierNextCtrl:
            pth.append(Geom::LineSegment(selfPt, nextPt));
            pv.push_back(Geom::Path(Geom::Circle(nextPt, 3)));
            pv.push_back(pth);
            break;

        case BezierNodeType::kBezierBothCtrl:
            pth.append(Geom::LineSegment(prevPt, nextPt));
            pv.push_back(Geom::Path(Geom::Circle(prevPt, 3)));
            pv.push_back(Geom::Path(Geom::Circle(nextPt, 3)));
            pv.push_back(pth);
            break;

        default:
            break;
        }
    }
}

void BeziergonNode::AddCorner(const Geom::Point &pt, const Geom::Point &c0, const Geom::Point &c1, const BezierNodeType t)
{
    data_.points.push_back({ pt.x(), pt.y(), c0.x(), c0.y(), c1.x(), c1.y() });
    data_.ntypes.push_back(static_cast<int>(t));
}

void BeziergonNode::ReplaceBackCorner(const Geom::Point &pt, const Geom::Point &c0, const Geom::Point &c1, const BezierNodeType t)
{
    data_.points.back()[0] = pt.x();
    data_.points.back()[1] = pt.y();
    data_.points.back()[2] = c0.x();
    data_.points.back()[3] = c0.y();
    data_.points.back()[4] = c1.x();
    data_.points.back()[5] = c1.y();
    data_.ntypes.back() = static_cast<int>(t);
}

void BeziergonNode::PopCorner()
{
    data_.points.pop_back();
    data_.ntypes.pop_back();
}

BezierNodeType BeziergonNode::GetCorner(int pos, Geom::Point &corner, Geom::Point &c0, Geom::Point &c1) const
{
    corner.x() = data_.points[pos][0];
    corner.y() = data_.points[pos][1];
    c0.x()     = data_.points[pos][2];
    c0.y()     = data_.points[pos][3];
    c1.x()     = data_.points[pos][4];
    c1.y()     = data_.points[pos][5];

    return static_cast<BezierNodeType>(data_.ntypes[pos]);
}

void BeziergonNode::Save(const H5::Group &g) const
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
        H5DB::SetAttribute(cwg, std::string("Type"), GetArcTypeName());
        H5DB::Save(cwg, std::string("Transform"), data_.transform);
        H5DB::Save(cwg, std::string("PointTypes"), data_.ntypes);
        H5DB::Save(cwg, std::string("Points"), data_.points);
        ModelNode::Save(cwg);
    }
}

void BeziergonNode::Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me)
{
    H5DB::Load(g, std::string("Transform"), data_.transform);
    H5DB::Load(g, std::string("PointTypes"), data_.ntypes);
    H5DB::Load(g, std::string("Points"), data_.points);

    std::string tName = H5DB::GetAttribute<std::string>(g, std::string("Type"));
    data_.type = GetArcTypeValue(tName);

    ModelNode::Load(g, nf, me);
}

void BeziergonNode::DoTransform(const Geom::Affine &aff, const double dx, const double dy)
{
    if (HitState::kHsFace == selData_.hs)
    {
        for (auto &pt : data_.points)
        {
            pt[0] += dx;
            pt[1] += dy;
            pt[2] += dx;
            pt[3] += dy;
            pt[4] += dx;
            pt[5] += dy;
        }
    }
    else
    {
        if (!aff.isIdentity())
        {
            for (int i = 0; i<static_cast<int>(base_.points.size()); ++i)
            {
                Geom::Point pt0{ base_.points[i][0], base_.points[i][1] };
                pt0 *= aff;
                data_.points[i][0] = pt0.x();
                data_.points[i][1] = pt0.y();

                Geom::Point pt1{ base_.points[i][2], base_.points[i][3] };
                pt1 *= aff;
                data_.points[i][2] = pt1.x();
                data_.points[i][3] = pt1.y();

                Geom::Point pt2{ base_.points[i][4], base_.points[i][5] };
                pt2 *= aff;
                data_.points[i][4] = pt2.x();
                data_.points[i][5] = pt2.y();
            }
        }
    }
}

std::string BeziergonNode::GetArcTypeName() const
{
    switch (data_.type)
    {
    case GenericEllipseArcType::kAtSlice: return std::string("slice");
    case GenericEllipseArcType::kAtChord: return std::string("chord");
    case GenericEllipseArcType::kAtArc: return std::string("arc");
    default: return std::string("slice");
    }
}

GenericEllipseArcType BeziergonNode::GetArcTypeValue(const std::string &tName) const
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
        return GenericEllipseArcType::kAtChord;
    }
}

void BeziergonNode::BuildPathImpl(Geom::PathVector &pv, const bool closePath) const
{
    int numCurves = static_cast<int>(data_.points.size());
    if (data_.points.size() == data_.ntypes.size() && numCurves>1)
    {
        int curveDegs[4][4] = { { 1, 2, 1, 2 },{ 1, 2, 1, 2 },{ 2, 3, 2, 3 },{ 2, 3, 2, 3 } };
        int ctrlIndexs[4][4] = { { 0, 1, 0, 1 },{ 0, 1, 0, 1 },{ 0, 0, 0, 0 },{ 0, 0, 0, 0 } };

        Geom::PathBuilder pb(pv);
        pb.moveTo(Geom::Point(data_.points[0][0], data_.points[0][1]));

        for (int c = 0; c<numCurves - !closePath; ++c)
        {
            int sIndex = c;
            int eIndex = (c + 1) % numCurves;
            Geom::Point ePt{ data_.points[eIndex][0], data_.points[eIndex][1] };

            int curveDeg = curveDegs[data_.ntypes[sIndex]][data_.ntypes[eIndex]];
            if (1 == curveDeg)
            {
                pb.lineTo(ePt);
            }
            else if (2 == curveDeg)
            {
                if (static_cast<int>(BezierNodeType::kBezierNoneCtrl) == data_.ntypes[sIndex] ||
                    static_cast<int>(BezierNodeType::kBezierPrevCtrl) == data_.ntypes[sIndex])
                {
                    pb.quadTo(Geom::Point(data_.points[eIndex][2], data_.points[eIndex][3]), ePt);
                }
                else
                {
                    pb.quadTo(Geom::Point(data_.points[sIndex][4], data_.points[sIndex][5]), ePt);
                }
            }
            else
            {
                Geom::Point c0{ data_.points[sIndex][4], data_.points[sIndex][5] };
                Geom::Point c1{ data_.points[eIndex][2], data_.points[eIndex][3] };
                pb.curveTo(c0, c1, ePt);
            }
        }

        if (closePath)
        {
            pb.closePath();
        }
        else
        {
            pb.flush();
        }
    }
}