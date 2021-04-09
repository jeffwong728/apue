#include "fixednode.h"
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

FixedNode::FixedNode()
    : DrawableNode()
{
}

FixedNode::~FixedNode()
{
}

RegionNode::RegionNode(const cv::Ptr<cv::mvlab::Region> &rgn)
    : FixedNode(), cvRgn_(rgn)
{
    if (cvRgn_)
    {
        const cv::Ptr<cv::mvlab::Contour> outer = cvRgn_->GetContour();
        outerCurves_.reserve(outer->CountCurves());
        outerBBoxs_.reserve(outer->CountCurves());

        for (int c = 0; c < outer->CountCurves(); ++c)
        {
            outerCurves_.emplace_back();
            outerBBoxs_.emplace_back();
            outer->SelectPoints(c, outerCurves_.back());
            outerBBoxs_.back() = cv::mvlab::BoundingBox(outerCurves_.back());
        }

        const cv::Ptr<cv::mvlab::Contour> inner = cvRgn_->GetHole();
        innerCurves_.reserve(inner->CountCurves());
        innerBBoxs_.reserve(inner->CountCurves());
        for (int c = 0; c < inner->CountCurves(); ++c)
        {
            innerCurves_.emplace_back();
            innerBBoxs_.emplace_back();
            inner->SelectPoints(c, innerCurves_.back());
            innerBBoxs_.back() = cv::mvlab::BoundingBox(innerCurves_.back());
        }
    }
}

bool RegionNode::IsTypeOf(const SpamEntityType t) const
{
    switch (t)
    {
    case SpamEntityType::kET_REGION:
        return true;

    default: return false;
    }
}

bool RegionNode::IsLegalHit(const SpamEntityOperation entityOp) const
{
    switch (entityOp)
    {
    case SpamEntityOperation::kEO_REGION_PROBE:
        return true;

    default:
        return false;
    }
}

bool RegionNode::IsIntersection(const Geom::Rect &box) const
{
    for (const cv::Rect &oRect : outerBBoxs_)
    {
        Geom::Point pt1(oRect.tl().x, oRect.tl().y);
        Geom::Point pt2(oRect.br().x, oRect.br().y);
        if (!box.contains(pt1) || !box.contains(pt2))
        {
            return false;
        }
    }

    return true;
}

SelectionData RegionNode::HitTest(const Geom::Point &pt) const
{
    SelectionData sd{ selData_.ss, HitState::kHsNone, 0, 0, 0 };
    if (cvRgn_)
    {
        cv::Point cvpt{cvRound(pt.x()), cvRound(pt.y())};
        if (cvRgn_->TestPoint(cvpt))
        {
            sd.hs = HitState::kHsFace;
        }
    }
    return sd;
}

SelectionData RegionNode::HitTest(const Geom::Point &pt, const double WXUNUSED(sx), const double WXUNUSED(sy)) const
{
    return HitTest(pt);
}

Geom::OptRect RegionNode::GetBoundingBox() const
{
    if (cvRgn_)
    {
        cv::Rect cvRC = cvRgn_->BoundingBox();
        return Geom::OptRect(Geom::IntRect(cvRC.tl().x, cvRC.tl().y, cvRC.br().x, cvRC.br().y));
    }

    return Geom::OptRect();
}

void RegionNode::Draw(Cairo::RefPtr<Cairo::Context> &cr, const std::vector<Geom::Rect> &invalidRects) const
{
    wxColour::ChannelType r = drawStyle_.strokeColor_.Red();
    wxColour::ChannelType g = drawStyle_.strokeColor_.Green();
    wxColour::ChannelType b = drawStyle_.strokeColor_.Blue();
    wxColour::ChannelType a = drawStyle_.strokeColor_.Alpha();
    if (HighlightState::kHlFace == hlData_.hls)
    {
        r = 0xF9; g = 0xA6; b = 0x02; a = 0xFF;
    }

    cr->set_line_width(drawStyle_.strokeWidth_);
    cr->set_source_rgba(r / 255., g / 255., b / 255., a / 255.);

    Geom::Rect bbox;
    for (int cc = 0; cc < static_cast<int>(outerCurves_.size()); ++cc)
    {
        bbox.setLeft(outerBBoxs_[cc].x);
        bbox.setRight(outerBBoxs_[cc].x + outerBBoxs_[cc].width);
        bbox.setTop(outerBBoxs_[cc].y);
        bbox.setBottom(outerBBoxs_[cc].y + outerBBoxs_[cc].height);
        if (IsNeedRefresh(bbox, invalidRects))
        {
            const auto &curve = outerCurves_[cc];
            if (curve.size() > 2)
            {
                cr->move_to(curve.front().x + 0.5f, curve.front().y + 0.5f);
                for (int vv = 1; vv < static_cast<int>(curve.size()); ++vv)
                {
                    cr->line_to(curve[vv].x + 0.5f, curve[vv].y + 0.5f);
                }
                cr->close_path();
            }
        }
    }

    for (int cc = 0; cc < static_cast<int>(innerCurves_.size()); ++cc)
    {
        bbox.setLeft(innerBBoxs_[cc].x);
        bbox.setRight(innerBBoxs_[cc].x + innerBBoxs_[cc].width);
        bbox.setTop(innerBBoxs_[cc].y);
        bbox.setBottom(innerBBoxs_[cc].y + innerBBoxs_[cc].height);
        if (IsNeedRefresh(bbox, invalidRects))
        {
            const auto &curve = innerCurves_[cc];
            if (curve.size() > 2)
            {
                cr->move_to(curve.front().x + 0.5f, curve.front().y + 0.5f);
                for (int vv = 1; vv < static_cast<int>(curve.size()); ++vv)
                {
                    cr->line_to(curve[vv].x + 0.5f, curve[vv].y + 0.5f);
                }
                cr->close_path();
            }
        }
    }

    if (drawMode_)
    {
        cr->fill_preserve();
        if (IsSelected())
        {
            double offset = 0.;
            std::vector<double> dashes;
            cr->get_dash(dashes, offset);
            cr->set_dash(std::vector<double>(2, 5), 0);
            cr->stroke();
            cr->set_dash(dashes, offset);
        }
    }
    else
    {
        if (IsSelected())
        {
            double offset = 0.;
            std::vector<double> dashes;
            cr->get_dash(dashes, offset);
            cr->set_dash(std::vector<double>(2, 5), 0);
            cr->stroke();
            cr->set_dash(dashes, offset);
        }
        else
        {
            cr->stroke();
        }
    }
}
