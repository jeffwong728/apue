#include "drawablenode.h"
#include <ui/evts.h>
#include <ui/spam.h>
#include <helper/h5db.h>
#include <helper/commondef.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/2geom.h>
#include <2geom/path-sink.h>
#include <2geom/cairo-path-sink.h>
#include <2geom/svg-path-parser.h>
#include <2geom/path-intersection.h>
#pragma warning( pop )

DrawableNode::DrawableNode(const SPModelNode &parent, const wxString &title)
    : ModelNode(parent, title)
{
    ClearSelection();
    ClearHighlight();
}

DrawableNode::~DrawableNode()
{
}

void DrawableNode::BuildScaleHandle(const Geom::Point(&corners)[4], const double sx, const double sy, Geom::PathVector &hpv) const
{
    if (selData_.ss == SelectionState::kSelScale)
    {
        const double w = Geom::distance(corners[0], corners[1]);
        const double h = Geom::distance(corners[0], corners[3]);
        const double d0 = Geom::distance(corners[0], corners[2]);
        const double d1 = Geom::distance(corners[1], corners[3]);
        const double w2 = w / 2;
        const double h2 = h / 2;
        const double tol = 1e-9;

        Geom::Path straightArrow = Geom::parse_svg_path("m -7,0 3,-4 v 2 h 8 v -2 l 3,4 -3,4 v -2 h -8 v 2 l -3,-4").front();
        straightArrow *= Geom::Scale(sx*0.8, sy*0.8);
        const double breathDist = 10 * (sx + sy) / 2;

        const int idx[4][2] = { { 0, 1 },{ 1, 2 },{ 2, 3 },{ 3, 0 } };
        const double len[4] = { w, h, w, h };

        for (int i = 0; i<4; ++i)
        {
            if (len[i]>tol)
            {
                const Geom::Point &p0 = corners[idx[i][0]];
                const Geom::Point &p1 = corners[idx[i][1]];

                Geom::Point p01 = p1 - p0;
                Geom::Point p0e = p0 + p01.ccw();
                Geom::Point p0t = Geom::lerp(breathDist / len[i], p0, p0e);

                Geom::Point p10 = p0 - p1;
                Geom::Point p1e = p1 + p10.cw();
                Geom::Point p1t = Geom::lerp(breathDist / len[i], p1, p1e);

                Geom::Point pt = Geom::lerp(0.5, p0t, p1t);
                hpv.push_back(straightArrow*Geom::Rotate(p01.ccw())*Geom::Translate(pt.x(), pt.y()));
            }
        }

        const int didx[4][2] = { { 0, 2 },{ 1, 3 },{ 2, 0 },{ 3, 1 } };
        const double dlen[4] = { d0, d1, d0, d1 };

        for (int i = 0; i<4; ++i)
        {
            if (dlen[i]>tol)
            {
                const Geom::Point &p0 = corners[didx[i][0]];
                const Geom::Point &p1 = corners[didx[i][1]];
                Geom::Point pt = Geom::lerp(1 + breathDist / dlen[i], p1, p0);
                hpv.push_back(straightArrow*Geom::Rotate(p0 - p1)*Geom::Translate(pt.x(), pt.y()));
            }
        }
    }
}

void DrawableNode::BuildSkewHandle(const Geom::Point(&corners)[4], const double sx, const double sy, Geom::PathVector &hpv) const
{
    if (selData_.ss == SelectionState::kSelRotateAndSkew)
    {
        const double w = Geom::distance(corners[0], corners[1]);
        const double h = Geom::distance(corners[0], corners[3]);
        const double d0 = Geom::distance(corners[0], corners[2]);
        const double d1 = Geom::distance(corners[1], corners[3]);
        const double w2 = w / 2;
        const double h2 = h / 2;
        const double tol = 1e-9;

        Geom::Path straightArrow = Geom::parse_svg_path("m -7,0 3,-4 v 2 h 8 v -2 l 3,4 -3,4 v -2 h -8 v 2 l -3,-4").front();
        straightArrow *= Geom::Scale(sx*0.8, sy*0.8);
        const double breathDist = 10 * (sx + sy) / 2;

        const int idx[4][2] = { { 0, 1 },{ 1, 2 },{ 2, 3 },{ 3, 0 } };
        const double len[4] = { w, h, w, h };

        for (int i = 0; i<4; ++i)
        {
            if (len[i]>tol)
            {
                const Geom::Point &p0 = corners[idx[i][0]];
                const Geom::Point &p1 = corners[idx[i][1]];

                Geom::Point p01 = p1 - p0;
                Geom::Point p0e = p0 + p01.ccw();
                Geom::Point p0t = Geom::lerp(breathDist / len[i], p0, p0e);

                Geom::Point p10 = p0 - p1;
                Geom::Point p1e = p1 + p10.cw();
                Geom::Point p1t = Geom::lerp(breathDist / len[i], p1, p1e);

                Geom::Point pt = Geom::lerp(0.5, p0t, p1t);
                hpv.push_back(straightArrow*Geom::Rotate(p01)*Geom::Translate(pt.x(), pt.y()));
            }
        }
    }
}

void DrawableNode::BuildRotateHandle(const Geom::Point(&corners)[4], const double sx, const double sy, Geom::PathVector &hpv) const
{
    if (selData_.ss == SelectionState::kSelRotateAndSkew)
    {
        const double w = Geom::distance(corners[0], corners[1]);
        const double h = Geom::distance(corners[0], corners[3]);
        const double d0 = Geom::distance(corners[0], corners[2]);
        const double d1 = Geom::distance(corners[1], corners[3]);
        const double w2 = w / 2;
        const double h2 = h / 2;
        const double tol = 1e-9;

        Geom::Path rotateArrow = Geom::parse_svg_path("m 0,-5 c -5,0 -5,5 -5,5 h 3 l -5,7 -5,-7 h 3 c 0,-9 9,-9 9,-9 v -3 l 7,5 -7,5 v -3").front();
        rotateArrow *= Geom::Scale(sx*0.6, sy*0.6);
        const double breathDist = 10 * (sx + sy) / 2;

        const int didx[4][3] = { { 0, 2, 1 },{ 1, 3, 2 },{ 2, 0, 3 },{ 3, 1, 0 } };
        const double dlen[4] = { d0, d1, d0, d1 };

        for (int i = 0; i<4; ++i)
        {
            if (dlen[i]>tol)
            {
                const Geom::Point &p0 = corners[didx[i][0]];
                const Geom::Point &p1 = corners[didx[i][1]];
                const Geom::Point &p3 = corners[didx[i][2]];
                Geom::Point pt = Geom::lerp(1 + breathDist / dlen[i], p1, p0);
                hpv.push_back(rotateArrow*Geom::Rotate(p3 - p0)*Geom::Translate(pt.x(), pt.y()));
            }
        }
    }
}

void DrawableNode::BuildBox(const Geom::Point(&corners)[4], Geom::PathVector &bpv) const
{
    Geom::PathBuilder pb(bpv);
    pb.moveTo(corners[0]);
    pb.lineTo(corners[1]);
    pb.lineTo(corners[2]);
    pb.lineTo(corners[3]);
    pb.closePath();
}

void DrawableNode::BuildCorners(const Geom::PathVector &pv, Geom::Point(&corners)[4]) const
{
    Geom::OptRect rect = pv.boundsFast();
    if (rect)
    {
        for (int i=0; i<4; ++i)
        {
            corners[i] = rect->corner(i);
        }
    }
    else
    {
        for (int i = 0; i<4; ++i)
        {
            corners[i] = Geom::Point();
        }
    }
}

SelectionData DrawableNode::HitTest(const Geom::Point &pt, const double sx, const double sy) const
{
    SelectionData sd{ selData_.ss, HitState::kHsNone, -1, -1 };

    Geom::PathVector pv;
    BuildPath(pv);

    if (!pv.empty())
    {
        if (selData_.ss == SelectionState::kSelNodeEdit)
        {
            Geom::PathVector npv;
            NodeIdVector nids;
            BuildNode(npv, nids);

            for (int n = 0; n < static_cast<int>(npv.size()); ++n)
            {
                if (Geom::contains(npv[n], pt))
                {
                    sd.hs = HitState::kHsNode;
                    sd.id = nids[n].id;
                    sd.subid = nids[n].subid;
                    sd.master = sd.id;

                    return sd;
                }
            }

            Geom::Path pth;
            NodeIdVector eids;
            BuildEdge(pth, eids);

            for (int c = 0; c < static_cast<int>(pth.size()); ++c)
            {
                const Geom::Curve &curve = pth[c];
                Geom::Coord t = curve.nearestTime(pt);
                Geom::Coord dist = Geom::distanceSq(pt, curve.pointAt(t));
                if (dist < 9)
                {
                    sd.hs = HitState::kHsEdge;
                    sd.id = eids[c].id;
                    sd.subid = eids[c].subid;
                    sd.master = sd.id;

                    return sd;
                }
            }
        }

        if (Geom::contains(pv.front(), pt, false))
        {
            sd.hs = HitState::kHsFace;
            sd.id = 0;
            sd.subid = 0;
            sd.master = 0;

            return sd;
        }

        Geom::Point corners[4];
        BuildCorners(pv, corners);

        if (selData_.ss == SelectionState::kSelScale)
        {
            Geom::PathVector spv;
            BuildScaleHandle(corners, sx, sy, spv);

            for (int s = 0; s < static_cast<int>(spv.size()); ++s)
            {
                if (Geom::contains(spv[s], pt))
                {
                    sd.hs = HitState::kHsScaleHandle;
                    sd.id = s;
                    sd.subid = 0;
                    sd.master = 0;

                    return sd;
                }
            }
        }

        if (selData_.ss == SelectionState::kSelRotateAndSkew)
        {
            Geom::PathVector rpv;
            BuildRotateHandle(corners, sx, sy, rpv);

            for (int r = 0; r < static_cast<int>(rpv.size()); ++r)
            {
                if (Geom::contains(rpv[r], pt))
                {
                    sd.hs = HitState::kHsRotateHandle;
                    sd.id = r;
                    sd.subid = 0;
                    sd.master = 0;

                    return sd;
                }
            }

            Geom::PathVector skpv;
            BuildSkewHandle(corners, sx, sy, skpv);

            for (int sk = 0; sk < static_cast<int>(skpv.size()); ++sk)
            {
                if (Geom::contains(skpv[sk], pt))
                {
                    sd.hs = HitState::kHsSkewHandle;
                    sd.id = sk;
                    sd.subid = 0;
                    sd.master = 0;

                    return sd;
                }
            }
        }
    }

    return sd;
}

Geom::OptRect DrawableNode::GetBoundingBox() const
{
    Geom::PathVector pv;
    BuildPath(pv);
    return pv.boundsFast();
}

Geom::OptRect DrawableNode::GetBoundingBox(const Geom::PathVector &pv) const
{
    return pv.boundsFast();
}

void DrawableNode::StartEdit(const int toolId)
{
    switch (toolId)
    {
    case kSpamID_TOOLBOX_GEOM_TRANSFORM: StartTransform(); break;
    case kSpamID_TOOLBOX_GEOM_EDIT:      StartNodeEdit();  break;
    default: break;
    }
}

void DrawableNode::Edit(const int toolId, const Geom::Point &anchorPt, const Geom::Point &freePt, const Geom::Point &lastPt)
{
    switch (toolId)
    {
    case kSpamID_TOOLBOX_GEOM_TRANSFORM: Transform(anchorPt, freePt, lastPt); break;
    case kSpamID_TOOLBOX_GEOM_EDIT:      NodeEdit(anchorPt, freePt, lastPt);  break;
    default: break;
    }
}

void DrawableNode::EndEdit(const int toolId)
{
    switch (toolId)
    {
    case kSpamID_TOOLBOX_GEOM_TRANSFORM: EndTransform(); break;
    case kSpamID_TOOLBOX_GEOM_EDIT:      EndNodeEdit();  break;
    default: break;
    }
}

void DrawableNode::ResetEdit(const int toolId)
{
    switch (toolId)
    {
    case kSpamID_TOOLBOX_GEOM_TRANSFORM: ResetTransform(); break;
    case kSpamID_TOOLBOX_GEOM_EDIT:      ResetNodeEdit();  break;
    default: break;
    }
}

void DrawableNode::StartTransform()
{
    baseRect_ = Geom::Rect();

    Geom::PathVector pv;
    BuildPath(pv);
    auto rect = pv.boundsFast();
    if (rect)
    {
        baseRect_ = *rect;
    }
}

void DrawableNode::Transform(const Geom::Point &anchorPt, const Geom::Point &freePt, const Geom::Point &lastPt)
{
    Geom::Coord dx = freePt.x() - lastPt.x();
    Geom::Coord dy = freePt.y() - lastPt.y();
    Geom::Affine aff = Geom::Affine::identity();
    if (HitState::kHsRotateHandle == selData_.hs)
    {
        BuildRotateMat(anchorPt, freePt, aff);
    }
    else if (HitState::kHsScaleHandle == selData_.hs)
    {
        BuildScaleMat(anchorPt, freePt, aff);
    }
    else if (HitState::kHsSkewHandle == selData_.hs)
    {
        BuildSkewMat(anchorPt, freePt, aff);
    }
    else
    {
        aff *= Geom::Translate(dx, dy);
    }

    DoTransform(aff, dx, dy);
}

void DrawableNode::EndTransform()
{
    baseRect_ = Geom::Rect();
}

void DrawableNode::StartNodeEdit()
{
}

void DrawableNode::NodeEdit(const Geom::Point &anchorPt, const Geom::Point &freePt, const Geom::Point &lastPt)
{
    Geom::Coord dx = freePt.x() - lastPt.x();
    Geom::Coord dy = freePt.y() - lastPt.y();
    Geom::Affine aff = Geom::Affine::identity();
    if (HitState::kHsFace == selData_.hs)
    {
        aff *= Geom::Translate(dx, dy);
        DoTransform(aff, dx, dy);
    }
}

void DrawableNode::EndNodeEdit()
{
}

void DrawableNode::Draw(Cairo::RefPtr<Cairo::Context> &cr) const
{
    DrawHighlight(cr);
}

void DrawableNode::DrawHighlight(Cairo::RefPtr<Cairo::Context> &cr) const
{
    Geom::PathVector pv;
    BuildPath(pv);
    pv *= Geom::Translate(0.5, 0.5);

    double ux = drawStyle_.strokeWidth_;
    double uy = drawStyle_.strokeWidth_;
    cr->device_to_user_distance(ux, uy);
    if (ux < uy) ux = uy;

    double ax = drawStyle_.strokeWidth_ + 3;
    double ay = drawStyle_.strokeWidth_ + 3;
    cr->device_to_user_distance(ax, ay);
    if (ax < ay) ax = ay;

    cr->save();
    DrawHighlightFace(cr, pv, ux, ax);

    if (selData_.ss != SelectionState::kSelNone)
    {
        Geom::Point corners[4];
        Geom::PathVector spv;
        Geom::PathVector skpv;
        Geom::PathVector rpv;
        Geom::PathVector npv;
        Geom::PathVector hpv;
        Geom::Path       epth;
        NodeIdVector     nids;
        NodeIdVector     eids;

        double sx = 1;
        double sy = 1;
        cr->device_to_user_distance(sx, sy);

        double hx = 3;
        double hy = 3;
        cr->device_to_user_distance(hx, hy);

        BuildCorners(pv, corners);
        BuildScaleHandle(corners, sx, sy, spv);
        BuildSkewHandle(corners, sx, sy, skpv);
        BuildRotateHandle(corners, sx, sy, rpv);
        DrawHighlightScaleHandle(cr, spv, sx, hx);
        DrawHighlightSkewHandle(cr, skpv, sx, hx);
        DrawHighlightRotateHandle(cr, rpv, sx, hx);

        BuildEdge(epth, eids);
        DrawHighlightEdge(cr, epth, sx, hx);

        BuildHandle(hpv);
        DrawHighlightHandle(cr, hpv, HighlightState::kHlNone, sx, hx);

        BuildNode(npv, nids);
        DrawHighlightNode(cr, npv, nids, sx, hx);

        Geom::PathVector bpv;
        BuildBox(corners, bpv);

        const wxColour sc(0xF9, 0xA6, 0x02);
        Geom::CairoPathSink cairoPathSink(cr->cobj());
        cairoPathSink.feed(bpv);
        cr->set_dash(std::vector<double>(1, 2 * sx), 0);
        cr->set_line_width(sx);
        cr->set_source_rgba(sc.Red() / 255.0, sc.Green() / 255.0, sc.Blue() / 255.0, sc.Alpha() / 255.0);
        cr->stroke();
    }

    cr->restore();
}

void DrawableNode::ClearSelection()
{
    selData_.ss    = SelectionState::kSelNone;
    selData_.hs    = HitState::kHsNone;
    selData_.id    = -1;
    selData_.subid = -1;
    selData_.master = 0;
}

void DrawableNode::ClearHighlight()
{
    hlData_.hls   = HighlightState::kHlNone;
    hlData_.id    = -1;
    hlData_.subid = -1;
}

void DrawableNode::HighlightFace()
{
    hlData_.hls   = HighlightState::kHlFace;
    hlData_.id    = 0;
    hlData_.subid = 0;
}

void DrawableNode::Select(int toolId)
{
    switch (toolId)
    {
    case kSpamID_TOOLBOX_GEOM_TRANSFORM:
        selData_.ss = SelectionState::kSelScale;
        break;

    case kSpamID_TOOLBOX_GEOM_EDIT:
        selData_.ss = SelectionState::kSelNodeEdit;
        break;

    default:
        selData_.ss = SelectionState::kSelState;
        break;
    }

    selData_.hs = HitState::kHsNone;
    selData_.id = -1;
    selData_.subid = -1;
    selData_.master = 0;
}

void DrawableNode::SwitchSelectionState(int toolId)
{
    switch (toolId)
    {
    case kSpamID_TOOLBOX_GEOM_TRANSFORM: SwitchTransformState(); break;
    case kSpamID_TOOLBOX_GEOM_EDIT:      SwitchNodeEditState();  break;
    default: break;
    }
}

void DrawableNode::SetSelectionData(const SelectionData &selData)
{
    selData_ = selData;
}

HighlightData DrawableNode::MapSelectionToHighlight(const SelectionData &sd)
{
    HighlightState hss[] = 
    { 
        HighlightState::kHlNone,
        HighlightState::kHlNode,
        HighlightState::kHlEdge,
        HighlightState::kHlFace,
        HighlightState::kHlScaleHandle,
        HighlightState::kHlRotateHandle,
        HighlightState::kHlSkewHandle
    };

    return { hss[static_cast<int>(sd.hs)], sd.id, sd.subid };
}

bool DrawableNode::IsHighlightChanged(const HighlightData &hdl, const HighlightData &hdr)
{
    return hdl.hls != hdr.hls || hdl.id != hdr.id || hdl.subid != hdr.subid;
}

SelectionState DrawableNode::GetInitialSelectState(const int toolId)
{
    switch (toolId)
    {
    case kSpamID_TOOLBOX_GEOM_TRANSFORM: return SelectionState::kSelScale;
    case kSpamID_TOOLBOX_GEOM_EDIT:      return SelectionState::kSelNodeEdit;
    default: return SelectionState::kSelState;
    }
}

void DrawableNode::DrawHighlightFace(Cairo::RefPtr<Cairo::Context> &cr, const Geom::PathVector &fpv, const double ux, const double ax) const
{
    Geom::CairoPathSink cairoPathSink(cr->cobj());
    if (HighlightState::kHlFace == hlData_.hls)
    {
        wxColour::ChannelType r = 0xF9, g = 0xA6, b = 0x02;
        if (!IsLegalHit(Spam::GetSelectionFilter()->GetEntityOperation()))
        {
            r = 0xFF; g = 0; b = 0;
        }

        const wxColour sc(r, g, b);
        const wxColour &fc = sc;

        cairoPathSink.feed(fpv);
        cr->set_source_rgba(fc.Red() / 255.0, fc.Green() / 255.0, fc.Blue() / 255.0, fc.Alpha() / 255.0 / 4);
        cr->fill_preserve();
        cr->set_line_width(ax);
        cr->set_source_rgba(sc.Red() / 255.0, sc.Green() / 255.0, sc.Blue() / 255.0, sc.Alpha() / 255.0 / 4);
        cr->stroke_preserve();
        cr->set_line_width(ux);
        cr->set_source_rgba(sc.Red() / 255.0, sc.Green() / 255.0, sc.Blue() / 255.0, sc.Alpha() / 255.0);
        cr->stroke();
    }
    else
    {
        const wxColour &sc = drawStyle_.strokeColor_;
        const wxColour &fc = drawStyle_.fillColor_;

        cairoPathSink.feed(fpv);
        cr->set_source_rgba(fc.Red() / 255.0, fc.Green() / 255.0, fc.Blue() / 255.0, fc.Alpha() / 255.0);
        cr->fill_preserve();
        cr->set_line_width(ux);
        cr->set_source_rgba(sc.Red() / 255.0, sc.Green() / 255.0, sc.Blue() / 255.0, sc.Alpha() / 255.0);
        cr->stroke();
    }
}

void DrawableNode::DrawHighlightScaleHandle(Cairo::RefPtr<Cairo::Context> &cr, const Geom::PathVector &spv, const double ux, const double ax) const
{
    DrawHighlightHandle(cr, spv, HighlightState::kHlScaleHandle, ux, ax);
}

void DrawableNode::DrawHighlightSkewHandle(Cairo::RefPtr<Cairo::Context> &cr, const Geom::PathVector &spv, const double ux, const double ax) const
{
    DrawHighlightHandle(cr, spv, HighlightState::kHlSkewHandle, ux, ax);
}

void DrawableNode::DrawHighlightRotateHandle(Cairo::RefPtr<Cairo::Context> &cr, const Geom::PathVector &rpv, const double ux, const double ax) const
{
    DrawHighlightHandle(cr, rpv, HighlightState::kHlRotateHandle, ux, ax);
}

void DrawableNode::DrawHighlightNode(Cairo::RefPtr<Cairo::Context> &cr, const Geom::PathVector &npv, const NodeIdVector &ids, const double ux, const double ax) const
{
    Geom::CairoPathSink cairoPathSink(cr->cobj());
    wxColour::ChannelType r = 0xF9, g = 0xA6, b = 0x02;
    if (!IsLegalHit(Spam::GetSelectionFilter()->GetEntityOperation()))
    {
        r = 0xFF; g = 0; b = 0;
    }

    for (int p = 0; p<static_cast<int>(npv.size()); ++p)
    {
        if (npv[p].empty())
        {
            continue;
        }

        if (HighlightState::kHlNode == hlData_.hls &&
            hlData_.id == ids[p].id &&
            hlData_.subid == ids[p].subid)
        {
            const wxColour sc(r, g, b);
            const wxColour &fc = sc;

            cairoPathSink.feed(npv[p]);
            cr->set_source_rgba(fc.Red() / 255.0, fc.Green() / 255.0, fc.Blue() / 255.0, fc.Alpha() / 255.0 / 4);
            cr->fill_preserve();
            cr->set_line_width(ax);
            cr->set_source_rgba(sc.Red() / 255.0, sc.Green() / 255.0, sc.Blue() / 255.0, sc.Alpha() / 255.0 / 4);
            cr->stroke_preserve();
            cr->set_line_width(ux);
            cr->set_source_rgba(sc.Red() / 255.0, sc.Green() / 255.0, sc.Blue() / 255.0, sc.Alpha() / 255.0);
            cr->stroke();
        }
        else
        {
            const wxColour &sc = drawStyle_.strokeColor_;
            const wxColour &fc = drawStyle_.fillColor_;

            cairoPathSink.feed(npv[p]);
            cr->set_source_rgba(1.0, 0.0, 0.0, 0.2);
            cr->fill_preserve();
            cr->set_line_width(ux);
            cr->set_source_rgba(1.0, 0.0, 0.0, sc.Alpha() / 255.0);
            cr->stroke();
        }
    }
}

void DrawableNode::DrawHighlightHandle(Cairo::RefPtr<Cairo::Context> &cr, const Geom::PathVector &pv, const HighlightState hs, const double ux, const double ax) const
{
    Geom::CairoPathSink cairoPathSink(cr->cobj());
    wxColour::ChannelType r = 0xF9, g = 0xA6, b = 0x02;
    if (!IsLegalHit(Spam::GetSelectionFilter()->GetEntityOperation()))
    {
        r = 0xFF; g = 0; b = 0;
    }

    for (int p = 0; p<static_cast<int>(pv.size()); ++p)
    {
        if (pv[p].empty())
        {
            continue;
        }

        if (hs == hlData_.hls && hlData_.id == p)
        {
            const wxColour sc(r, g, b);
            const wxColour &fc = sc;

            cairoPathSink.feed(pv[p]);
            cr->set_source_rgba(fc.Red() / 255.0, fc.Green() / 255.0, fc.Blue() / 255.0, fc.Alpha() / 255.0 / 4);
            cr->fill_preserve();
            cr->set_line_width(ax);
            cr->set_source_rgba(sc.Red() / 255.0, sc.Green() / 255.0, sc.Blue() / 255.0, sc.Alpha() / 255.0 / 4);
            cr->stroke_preserve();
            cr->set_line_width(ux);
            cr->set_source_rgba(sc.Red() / 255.0, sc.Green() / 255.0, sc.Blue() / 255.0, sc.Alpha() / 255.0);
            cr->stroke();
        }
        else
        {
            const wxColour &sc = drawStyle_.strokeColor_;
            const wxColour &fc = drawStyle_.fillColor_;

            cairoPathSink.feed(pv[p]);
            cr->set_source_rgba(1.0, 0.0, 0.0, 0.2);
            cr->fill_preserve();
            cr->set_line_width(ux);
            cr->set_source_rgba(1.0, 0.0, 0.0, sc.Alpha() / 255.0);
            cr->stroke();
        }
    }
}

void DrawableNode::DrawHighlightEdge(Cairo::RefPtr<Cairo::Context> &cr, const Geom::Path &pth, const double ux, const double ax) const
{
    Geom::CairoPathSink cairoPathSink(cr->cobj());
    wxColour::ChannelType r = 0xF9, g = 0xA6, b = 0x02;
    if (!IsLegalHit(Spam::GetSelectionFilter()->GetEntityOperation()))
    {
        r = 0xFF; g = 0; b = 0;
    }

    for (int p = 0; p<static_cast<int>(pth.size()); ++p)
    {
        if (HighlightState::kHlEdge == hlData_.hls && hlData_.id == p)
        {
            const wxColour sc(r, g, b);
            cairoPathSink.feed(pth[p]);
            cr->set_line_width(ax);
            cr->set_source_rgba(sc.Red() / 255.0, sc.Green() / 255.0, sc.Blue() / 255.0, sc.Alpha() / 255.0 / 4);
            cr->stroke_preserve();
            cr->set_line_width(ux);
            cr->set_source_rgba(sc.Red() / 255.0, sc.Green() / 255.0, sc.Blue() / 255.0, sc.Alpha() / 255.0);
            cr->stroke();
        }
    }
}

void DrawableNode::BuildRotateMat(const Geom::Point &anchorPt, const Geom::Point &freePt, Geom::Affine &aff)
{
    Geom::Point cp = baseRect_.midpoint();
    Geom::Point ap = anchorPt - cp;
    Geom::Point fp = freePt - cp;

    aff *= Geom::Translate(-cp);
    aff *= Geom::Rotate(Geom::angle_between(ap, fp));
    aff *= Geom::Translate(cp);
}

void DrawableNode::BuildScaleMat(const Geom::Point &anchorPt, const Geom::Point &freePt, Geom::Affine &aff)
{
    double tx = 0;
    double ty = 0;
    double axDist = 0;
    double ayDist = 0;
    double fxDist = 0;
    double fyDist = 0;

    switch (selData_.id)
    {
    case 0:
        ty = baseRect_.bottom();
        ayDist = baseRect_.height();
        fyDist = baseRect_.height() - (freePt.y() - anchorPt.y());
        break;

    case 1:
        tx = baseRect_.left();
        axDist = baseRect_.width();
        fxDist = baseRect_.width() + (freePt.x() - anchorPt.x());
        break;

    case 2:
        ty = baseRect_.top();
        ayDist = baseRect_.height();
        fyDist = baseRect_.height() + (freePt.y() - anchorPt.y());
        break;

    case 3:
        tx = baseRect_.right();
        axDist = baseRect_.width();
        fxDist = baseRect_.width() - (freePt.x() - anchorPt.x());
        break;

    case 4:
        tx = baseRect_.right();
        ty = baseRect_.bottom();
        axDist = baseRect_.width();
        ayDist = baseRect_.height();
        fxDist = baseRect_.width() - (freePt.x() - anchorPt.x());
        fyDist = baseRect_.height() - (freePt.y() - anchorPt.y());
        break;

    case 5:
        tx = baseRect_.left();
        ty = baseRect_.bottom();
        axDist = baseRect_.width();
        ayDist = baseRect_.height();
        fxDist = baseRect_.width() + (freePt.x() - anchorPt.x());
        fyDist = baseRect_.height() - (freePt.y() - anchorPt.y());
        break;

    case 6:
        tx = baseRect_.left();
        ty = baseRect_.top();
        axDist = baseRect_.width();
        ayDist = baseRect_.height();
        fxDist = baseRect_.width() + (freePt.x() - anchorPt.x());
        fyDist = baseRect_.height() + (freePt.y() - anchorPt.y());
        break;

    case 7:
        tx = baseRect_.right();
        ty = baseRect_.top();
        axDist = baseRect_.width();
        ayDist = baseRect_.height();
        fxDist = baseRect_.width() - (freePt.x() - anchorPt.x());
        fyDist = baseRect_.height() + (freePt.y() - anchorPt.y());
        break;

    default:
        break;
    }

    axDist = std::max(0.0, axDist);
    ayDist = std::max(0.0, ayDist);
    fxDist = std::max(0.0, fxDist);
    fyDist = std::max(0.0, fyDist);

    double sx = 1.0;
    double sy = 1.0;

    if (fxDist > Geom::EPSILON && axDist > Geom::EPSILON)
    {
        sx = fxDist / axDist;
    }

    if (fyDist > Geom::EPSILON && ayDist > Geom::EPSILON)
    {
        sy = fyDist / ayDist;
    }

    aff *= Geom::Translate(-tx, -ty);
    aff *= Geom::Scale(sx, sy);
    aff *= Geom::Translate(tx, ty);
}

void DrawableNode::BuildSkewMat(const Geom::Point &anchorPt, const Geom::Point &freePt, Geom::Affine &aff)
{
    double tx = 0;
    double ty = 0;
    double axDist = 0;
    double ayDist = 0;
    double fxDist = 0;
    double fyDist = 0;

    switch (selData_.id)
    {
    case 0:
        tx = baseRect_.left();
        ty = baseRect_.bottom();
        fxDist = anchorPt.x() - freePt.x();
        break;

    case 1:
        tx = baseRect_.left();
        ty = baseRect_.top();
        fyDist = freePt.y() - anchorPt.y();
        break;

    case 2:
        tx = baseRect_.left();
        ty = baseRect_.top();
        fxDist = freePt.x() - anchorPt.x();
        break;

    case 3:
        tx = baseRect_.right();
        ty = baseRect_.top();
        fyDist = anchorPt.y() - freePt.y();
        break;

    default:
        break;
    }

    double mx = 0;
    double my = 0;

    if (baseRect_.height() > Geom::EPSILON)
    {
        mx = fxDist / baseRect_.height();
    }

    if (baseRect_.width() > Geom::EPSILON)
    {
        my = fyDist / baseRect_.width();
    }
 
    aff *= Geom::Translate(-tx, -ty);
    aff *= Geom::HShear(mx);
    aff *= Geom::VShear(my);
    aff *= Geom::Translate(tx, ty);
}

void DrawableNode::SwitchTransformState()
{
    switch (selData_.ss)
    {
    case SelectionState::kSelNone:
        selData_.ss = SelectionState::kSelScale;
        break;

    case SelectionState::kSelScale:
        selData_.ss = SelectionState::kSelRotateAndSkew;
        break;

    case SelectionState::kSelRotateAndSkew:
        selData_.ss = SelectionState::kSelScale;
        break;

    default: break;
    }
}

void DrawableNode::SwitchNodeEditState()
{
    switch (selData_.ss)
    {
    case SelectionState::kSelNone:
        selData_.ss = SelectionState::kSelNodeEdit;
        break;

    default: break;
    }

    selData_.master = std::max(0, selData_.id);
}