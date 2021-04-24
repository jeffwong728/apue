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

FixedNode::FixedNode(const SPModelNode &parent)
    : DrawableNode(parent)
{
}

FixedNode::~FixedNode()
{
}

RegionNode::RegionNode(const cv::Ptr<cv::mvlab::Region> &rgn, const SPModelNode &parent)
    : FixedNode(parent), cvRgn_(rgn)
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

        title_ = wxString("Region ") + wxString::Format("%p", rgn.get());
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
        cv::Rect cvRC = BoundingBox();
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

    if (IsSelected())
    {
        r = 218; g = 165; b = 32; a = 0xFF;
    }

    if (HighlightState::kHlFace == hlData_.hls)
    {
        r = 255; g = 215; b = 0; a = 0xFF;
    }

    double lineWidthX = drawStyle_.strokeWidth_ > 0.5 ? drawStyle_.strokeWidth_ : 1.5;
    double lineWidthY = drawStyle_.strokeWidth_ > 0.5 ? drawStyle_.strokeWidth_ : 1.5;
    cr->device_to_user_distance(lineWidthX, lineWidthY);
    if (lineWidthY < lineWidthX) lineWidthY = lineWidthX;

    if (drawMode_)
    {
        cr->set_line_width(0.);
    }
    else
    {
        cr->set_line_width(lineWidthY);
    }

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
        cr->fill();
    }
    else
    {
        cr->stroke();
    }

    if (cvRgn_ && IsNeedRefresh(*GetBoundingBox(), invalidRects))
    {
        cr->set_line_width(lineWidthY);
        cr->set_source_rgba(1., 1., 1., 1.);
        cr->select_font_face("Sans", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_BOLD);
        cr->set_font_size(12.0*lineWidthY);

        if (TestFeature(RegionFeatureFlag::kRFF_AREA) || TestHighlightFeature(RegionFeatureFlag::kRFF_AREA))
        {
            Cairo::TextExtents extents;
            std::string utf8 = std::to_string(cv::saturate_cast<long long>(cvRgn_->Area()));
            cr->get_text_extents(utf8, extents);

            cv::Point2d ct = cvRgn_->Centroid();
            cr->move_to(ct.x - extents.width / 2, ct.y + extents.height / 2);
            cr->show_text(utf8);
        }

        if (TestFeature(RegionFeatureFlag::kRFF_LENGTH) || TestHighlightFeature(RegionFeatureFlag::kRFF_LENGTH))
        {
            Cairo::TextExtents extents;
            std::ostringstream oss; oss << std::setprecision(3) << cvRgn_->Contlength();
            std::string utf8 = oss.str();
            cr->get_text_extents(utf8, extents);

            cv::Point2d ct = cvRgn_->Centroid();
            cr->move_to(ct.x - extents.width / 2, ct.y + extents.height / 2);
            cr->show_text(utf8);
        }

        if (TestFeature(RegionFeatureFlag::kRFF_CIRCULARITY) || TestHighlightFeature(RegionFeatureFlag::kRFF_CIRCULARITY))
        {
            Cairo::TextExtents extents;
            std::ostringstream oss; oss << std::setprecision(3) << cvRgn_->Circularity();
            std::string utf8 = oss.str();
            cr->get_text_extents(utf8, extents);

            cv::Point2d ct = cvRgn_->Centroid();
            cr->move_to(ct.x - extents.width / 2, ct.y + extents.height / 2);
            cr->show_text(utf8);
        }

        if (TestFeature(RegionFeatureFlag::kRFF_CONVEXITY) || TestHighlightFeature(RegionFeatureFlag::kRFF_CONVEXITY))
        {
            Cairo::TextExtents extents;
            std::ostringstream oss; oss << std::setprecision(3) << cvRgn_->Convexity();
            std::string utf8 = oss.str();
            cr->get_text_extents(utf8, extents);

            cv::Point2d ct = cvRgn_->Centroid();
            cr->move_to(ct.x - extents.width / 2, ct.y + extents.height / 2);
            cr->show_text(utf8);
        }

        if (TestFeature(RegionFeatureFlag::kRFF_DIAMETER) || TestHighlightFeature(RegionFeatureFlag::kRFF_DIAMETER))
        {
            cv::Scalar dia = cvRgn_->Diameter();
            cr->move_to(dia[0] + 0.5f, dia[1] + 0.5f);
            cr->line_to(dia[2] + 0.5f, dia[3] + 0.5f);
            cr->stroke();

            Geom::Point sPt(dia[0], dia[1]);
            Geom::Point ePt(dia[2], dia[3]);
            Geom::Point vPt = ePt - sPt;
            Geom::Point ct = Geom::middle_point(sPt, ePt);

            cr->save();
            cr->translate(ct.x(), ct.y());
            cr->rotate(std::atan2(vPt.y(), vPt.x())+ Geom::rad_from_deg(180));
            cr->translate(-ct.x(), -ct.y());
            std::string utf8(wxString::Format(wxT("%.3f"), Geom::distance(sPt, ePt)).ToUTF8().data());
            Cairo::TextExtents extents;
            cr->get_text_extents(utf8, extents);
            cr->move_to(ct.x() - extents.width / 2, ct.y() - lineWidthY * 4);
            cr->show_text(utf8);
            cr->restore();
        }

        if (TestFeature(RegionFeatureFlag::kRFF_SMALLEST_CIRCLE) || TestHighlightFeature(RegionFeatureFlag::kRFF_SMALLEST_CIRCLE))
        {
            cv::Point3d sc = cvRgn_->SmallestCircle();

            cr->arc(sc.x, sc.y, sc.z, 0, 2 * M_PI);
            cr->stroke();
        }

        if (TestFeature(RegionFeatureFlag::kRFF_CONVEX_HULL) || TestHighlightFeature(RegionFeatureFlag::kRFF_CONVEX_HULL))
        {
            if (convexHull_.empty())
            {
                cvRgn_->GetConvex()->SelectPoints(0, convexHull_);
            }

            if (convexHull_.size() > 2)
            {
                cr->move_to(convexHull_.front().x + 0.5f, convexHull_.front().y + 0.5f);
                for (int vv = 1; vv < static_cast<int>(convexHull_.size()); ++vv)
                {
                    cr->line_to(convexHull_[vv].x + 0.5f, convexHull_[vv].y + 0.5f);
                }
                cr->close_path();
                cr->stroke();
            }
        }

        if (TestFeature(RegionFeatureFlag::kRFF_RECT1) || TestHighlightFeature(RegionFeatureFlag::kRFF_RECT1))
        {
            cv::Rect rc1 = cvRgn_->BoundingBox();
            cv::Point2f pts[4] = { cv::Point2f(rc1.x, rc1.y), cv::Point2f(rc1.x + rc1.width, rc1.y), cv::Point2f(rc1.x + rc1.width, rc1.y + rc1.height), cv::Point2f(rc1.x, rc1.y + rc1.height) };
            cr->move_to(pts[0].x + 0.5f, pts[0].y + 0.5f);
            cr->line_to(pts[1].x + 0.5f, pts[1].y + 0.5f);
            cr->line_to(pts[2].x + 0.5f, pts[2].y + 0.5f);
            cr->line_to(pts[3].x + 0.5f, pts[3].y + 0.5f);
            cr->close_path();
            cr->stroke();
        }

        if (TestFeature(RegionFeatureFlag::kRFF_RECT2) || TestHighlightFeature(RegionFeatureFlag::kRFF_RECT2))
        {
            cv::RotatedRect rect2 = cvRgn_->GetContour()->SmallestRectangle();
            cv::Point2f pts[4];
            rect2.points(pts);
            cr->move_to(pts[0].x + 0.5f, pts[0].y + 0.5f);
            cr->line_to(pts[1].x + 0.5f, pts[1].y + 0.5f);
            cr->line_to(pts[2].x + 0.5f, pts[2].y + 0.5f);
            cr->line_to(pts[3].x + 0.5f, pts[3].y + 0.5f);
            cr->close_path();
            cr->stroke();
        }

        if (TestFeature(RegionFeatureFlag::kRFF_ELLIPTIC_AXIS) || TestHighlightFeature(RegionFeatureFlag::kRFF_ELLIPTIC_AXIS))
        {
            cv::Point2d ct = cvRgn_->Centroid();
            cv::Point3d ea = cvRgn_->EllipticAxis();

            Geom::CairoPathSink cairoPathSink(cr->cobj());
            Geom::Ellipse ge(ct.x, ct.y, ea.x, ea.y, Geom::rad_from_deg(ea.z));
            if (ea.x > Geom::EPSILON && ea.y > Geom::EPSILON)
            {
                cairoPathSink.feed(ge);
                cr->stroke();
            }
        }

        if (TestFeature(RegionFeatureFlag::kRFF_ORIENTATION) || TestHighlightFeature(RegionFeatureFlag::kRFF_ORIENTATION))
        {
            cv::Point2d ct = cvRgn_->Centroid();
            cv::Point3d ea = cvRgn_->EllipticAxis();
            const Geom::Coord arrowLen = std::max(12., std::max(ea.x, ea.y)*0.618);
            Geom::Point sPt{ ct.x, ct.y };
            Geom::Point ePt{ ct.x + arrowLen * std::cos(Geom::rad_from_deg(ea.z)), ct.y + arrowLen * std::sin(Geom::rad_from_deg(ea.z)) };
            Geom::Point arrowBase = Geom::lerp(8 / arrowLen, ePt, sPt);
            Geom::Point tPt = arrowBase + Geom::rot90(ePt - arrowBase);
            Geom::Point rPt = Geom::lerp(0.618, tPt, arrowBase);
            Geom::Point lPt = Geom::lerp(2, rPt, arrowBase);

            cr->move_to(ePt.x(), ePt.y());
            cr->line_to(rPt.x(), rPt.y());
            cr->line_to(lPt.x(), lPt.y());
            cr->close_path();
            cr->move_to(sPt.x(), sPt.y());
            cr->line_to(ePt.x(), ePt.y());
            cr->stroke();

            cr->save();
            cr->translate(ct.x, ct.y);
            cr->rotate_degrees(ea.z);
            cr->translate(-ct.x, -ct.y);
            std::string utf8(wxString::Format(wxT("%.3f"), cvRgn_->Orientation()).ToUTF8().data());
            cr->move_to(ct.x, ct.y - lineWidthY * 4);
            cr->show_text(utf8);
            cr->restore();
        }

        if (TestFeature(RegionFeatureFlag::kRFF_CENTROID) || TestHighlightFeature(RegionFeatureFlag::kRFF_CENTROID))
        {
            cv::Point2d ct = cvRgn_->Centroid();
            cr->arc(ct.x, ct.y, 2.5, 0, 2 * M_PI);
            cr->fill();
        }
    }
}

cv::Rect RegionNode::BoundingBox() const
{
    cv::Point3d sc = cvRgn_->SmallestCircle();
    const int x = cvFloor(sc.x - sc.z - drawStyle_.strokeWidth_);
    const int y = cvFloor(sc.y - sc.z - drawStyle_.strokeWidth_);
    const int w = cvCeil((sc.z+ drawStyle_.strokeWidth_ )*2);
    return cv::Rect(x, y, w, w);
}
