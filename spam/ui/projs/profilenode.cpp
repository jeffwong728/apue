#include "profilenode.h"
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

ProfileNode::ProfileNode(const SPModelNode &parent)
    : DrawableNode(parent)
{
}

ProfileNode::~ProfileNode()
{
}

bool ProfileNode::IsTypeOf(const SpamEntityType WXUNUSED(t)) const
{
    return false;
}

bool ProfileNode::IsLegalHit(const SpamEntityOperation WXUNUSED(entityOp)) const
{
    return false;
}

bool ProfileNode::IsIntersection(const Geom::Rect &WXUNUSED(box)) const
{
    return false;
}

SelectionData ProfileNode::HitTest(const Geom::Point &pt) const
{
    SelectionData sd{ SelectionState::kSelNone, HitState::kHsNone, 0, 0, 0 };
    return sd;
}

SelectionData ProfileNode::HitTest(const Geom::Point &pt, const double WXUNUSED(sx), const double WXUNUSED(sy)) const
{
    return HitTest(pt);
}

Geom::OptRect ProfileNode::GetBoundingBox() const
{
    return Geom::OptRect(begPoint_, endPoint_);
}

void ProfileNode::Draw(Cairo::RefPtr<Cairo::Context> &cr, const std::vector<Geom::Rect> &invalidRects) const
{
    Geom::Rect bbox(begPoint_, endPoint_);
    if (IsNeedRefresh(bbox, invalidRects))
    {
        double ux = 1.5, uy = 1.5;
        cr->device_to_user_distance(ux, uy);
        if (ux < uy) ux = uy;

        const Geom::Coord arrowLen = (endPoint_ - begPoint_).length();
        if (arrowLen > 1)
        {
            Geom::Point arrowBase = Geom::lerp(6 * ux / arrowLen, endPoint_, begPoint_);
            Geom::Point tPt = arrowBase + Geom::rot90(endPoint_ - arrowBase);
            Geom::Point rPt = Geom::lerp(0.618, tPt, arrowBase);
            Geom::Point lPt = Geom::lerp(2, rPt, arrowBase);

            cr->move_to(endPoint_.x(), endPoint_.y());
            cr->line_to(rPt.x(), rPt.y());
            cr->line_to(lPt.x(), lPt.y());
            cr->close_path();

            cr->save();
            cr->set_line_width(ux);
            cr->set_source_rgba(1.0, 0.0, 1.0, 1.0);
            cr->move_to(begPoint_.x(), begPoint_.y());
            cr->line_to(endPoint_.x(), endPoint_.y());
            cr->stroke();
            cr->restore();
        }
    }
}
