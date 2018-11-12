#include "drawablenode.h"
#include <ui/evts.h>
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

void DrawableNode::BuildHandle(const Geom::Point(&corners)[4], const double sx, const double sy, Geom::PathVector &hpv) const
{
    BuildScaleHandle(corners, sx, sy, hpv);
    BuildSkewHandle(corners, sx, sy, hpv);
    BuildRotateHandle(corners, sx, sy, hpv);
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
        straightArrow *= Geom::Scale(sx*0.6, sy*0.6);
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
        straightArrow *= Geom::Scale(sx*0.6, sy*0.6);
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
    Geom::Rect boxRect(corners[0], corners[2]);
    bpv.push_back(Geom::Path(boxRect));
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
        if (Geom::contains(pv.front(), pt))
        {
            sd.hs = HitState::kHsFace;
            sd.id = 0;
            sd.subid = 0;

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

                    return sd;
                }
            }
        }
    }

    return sd;
}

void DrawableNode::Draw(Cairo::RefPtr<Cairo::Context> &cr) const
{
    if (HighlightState::kHlFace == hlData_.hls)
    {
        DrawHighlight(cr);
    }
    else
    {
        Geom::PathVector pv;
        BuildPath(pv);
        pv *= Geom::Translate(0.5, 0.5);

        const wxColour &strokeColor = drawStyle_.strokeColor_;
        const wxColour &fillColor = drawStyle_.fillColor_;

        double ux = drawStyle_.strokeWidth_;
        double uy = drawStyle_.strokeWidth_;
        cr->device_to_user_distance(ux, uy);
        if (ux < uy) ux = uy;

        cr->save();
        Geom::CairoPathSink cairoPathSink(cr->cobj());
        cairoPathSink.feed(pv);
        cr->set_source_rgba(fillColor.Red() / 255.0, fillColor.Green() / 255.0, fillColor.Blue() / 255.0, fillColor.Alpha() / 255.0);
        cr->fill_preserve();
        cr->set_line_width(ux);
        cr->set_source_rgba(strokeColor.Red() / 255.0, strokeColor.Green() / 255.0, strokeColor.Blue() / 255.0, strokeColor.Alpha() / 255.0);
        cr->stroke();

        if (selData_.ss != SelectionState::kSelNone)
        {
            Geom::Point corners[4];
            Geom::PathVector hpv;

            double sx = 1;
            double sy = 1;
            cr->device_to_user_distance(sx, sy);

            BuildCorners(pv, corners);
            BuildHandle(corners, sx, sy, hpv);

            cairoPathSink.feed(hpv);
            cr->set_source_rgba(1.0, 0.0, 0.0, 0.2);
            cr->fill_preserve();
            cr->set_source_rgba(1.0, 0.0, 0.0, strokeColor.Alpha() / 255.0);
            cr->stroke();

            Geom::PathVector bpv;
            BuildBox(corners, bpv);
            cairoPathSink.feed(bpv);
            cr->set_dash(std::vector<double>(1, 2*sx), 0);
            cr->set_line_width(sx);
            cr->set_source_rgba(strokeColor.Red() / 255.0, strokeColor.Green() / 255.0, strokeColor.Blue() / 255.0, strokeColor.Alpha() / 255.0);
            cr->stroke();
        }

        cr->restore();
    }
}

void DrawableNode::DrawHighlight(Cairo::RefPtr<Cairo::Context> &cr) const
{
    Geom::PathVector pv;
    BuildPath(pv);
    pv *= Geom::Translate(0.5, 0.5);

    const wxColour strokeColor(0xF9, 0xA6, 0x02);
    const wxColour &fillColor = strokeColor;

    double ux = drawStyle_.strokeWidth_;
    double uy = drawStyle_.strokeWidth_;
    cr->device_to_user_distance(ux, uy);
    if (ux < uy) ux = uy;

    double ax = drawStyle_.strokeWidth_ + 3;
    double ay = drawStyle_.strokeWidth_ + 3;
    cr->device_to_user_distance(ax, ay);
    if (ax < ay) ax = ay;

    cr->save();
    Geom::CairoPathSink cairoPathSink(cr->cobj());
    cairoPathSink.feed(pv);
    cr->set_source_rgba(fillColor.Red() / 255.0, fillColor.Green() / 255.0, fillColor.Blue() / 255.0, fillColor.Alpha() / 255.0 / 4);
    cr->fill_preserve();
    cr->set_line_width(ax);
    cr->set_source_rgba(strokeColor.Red() / 255.0, strokeColor.Green() / 255.0, strokeColor.Blue() / 255.0, strokeColor.Alpha() / 255.0 / 4);
    cr->stroke_preserve();
    cr->set_line_width(ux);
    cr->set_source_rgba(strokeColor.Red() / 255.0, strokeColor.Green() / 255.0, strokeColor.Blue() / 255.0, strokeColor.Alpha() / 255.0);
    cr->stroke();

    if (selData_.ss != SelectionState::kSelNone)
    {
        Geom::Point corners[4];
        Geom::PathVector hpv;

        double sx = 1;
        double sy = 1;
        cr->device_to_user_distance(sx, sy);

        BuildCorners(pv, corners);
        BuildHandle(corners, sx, sy, hpv);

        cairoPathSink.feed(hpv);
        cr->set_source_rgba(1.0, 0.0, 0.0, 0.2);
        cr->fill_preserve();
        cr->set_source_rgba(1.0, 0.0, 0.0, strokeColor.Alpha() / 255.0);
        cr->stroke();

        Geom::PathVector bpv;
        BuildBox(corners, bpv);
        cairoPathSink.feed(bpv);
        cr->set_dash(std::vector<double>(1, 2 * sx), 0);
        cr->set_line_width(sx);
        cr->set_source_rgba(strokeColor.Red() / 255.0, strokeColor.Green() / 255.0, strokeColor.Blue() / 255.0, strokeColor.Alpha() / 255.0);
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

void DrawableNode::SelectEntity()
{
    selData_.ss = SelectionState::kSelScale;
    selData_.hs = HitState::kHsNone;
    selData_.id = -1;
    selData_.subid = -1;
}

void DrawableNode::SwitchSelectionState()
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