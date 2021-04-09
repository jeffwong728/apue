#ifndef SPAM_UI_PROJS_FIXED_NODE_H
#define SPAM_UI_PROJS_FIXED_NODE_H
#include "modelfwd.h"
#include "drawablenode.h"
#include <opencv2/mvlab.hpp>
#include <cairomm/cairomm.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )

class FixedNode : public DrawableNode
{
public:
    FixedNode();
    ~FixedNode();

public:
    void BuildPath(Geom::PathVector &WXUNUSED(pv)) const wxOVERRIDE {}
    void BuildNode(Geom::PathVector &WXUNUSED(pv), NodeIdVector &WXUNUSED(ids), const double WXUNUSED(sx), const double WXUNUSED(sy)) const wxOVERRIDE {}
    void BuildEdge(CurveVector &WXUNUSED(pth), NodeIdVector &WXUNUSED(ids)) const wxOVERRIDE {}
    void BuildHandle(Geom::PathVector &WXUNUSED(hpv)) const wxOVERRIDE {}
    void BuildScaleHandle(const Geom::Point(&WXUNUSED(corners))[4], const double WXUNUSED(sx), const double WXUNUSED(sy), Geom::PathVector &WXUNUSED(hpv)) const wxOVERRIDE {}
    void BuildSkewHandle(const Geom::Point(&WXUNUSED(corners))[4], const double WXUNUSED(sx), const double WXUNUSED(sy), Geom::PathVector &WXUNUSED(hpv)) const wxOVERRIDE {}
    void BuildRotateHandle(const Geom::Point(&WXUNUSED(corners))[4], const double WXUNUSED(sx), const double WXUNUSED(sy), Geom::PathVector &WXUNUSED(hpv)) const wxOVERRIDE {}

    // Transform
    void StartTransform() wxOVERRIDE {}
    void Transform(const Geom::Point &WXUNUSED(anchorPt), const Geom::Point &WXUNUSED(freePt), const Geom::Point &WXUNUSED(lastPt)) wxOVERRIDE {}
    void EndTransform() wxOVERRIDE {}
    void ResetTransform() wxOVERRIDE {}

    // Edit node
    void StartNodeEdit() wxOVERRIDE {}
    void NodeEdit(const Geom::Point &WXUNUSED(anchorPt), const Geom::Point &WXUNUSED(freePt), const Geom::Point &WXUNUSED(lastPt)) wxOVERRIDE {}
    void EndNodeEdit() wxOVERRIDE {}
    void ResetNodeEdit() wxOVERRIDE {}

    boost::any CreateMemento() const wxOVERRIDE { return boost::any(); }
    bool RestoreFromMemento(const boost::any &WXUNUSED(memento)) wxOVERRIDE { return false; }

protected:
    void DoTransform(const Geom::Affine &WXUNUSED(aff), const double WXUNUSED(dx), const double WXUNUSED(dy)) wxOVERRIDE {}
};

class RegionNode : public FixedNode
{
public:
    RegionNode(const cv::Ptr<cv::mvlab::Region> &rgn);

public:
    bool IsTypeOf(const SpamEntityType t) const wxOVERRIDE;
    bool IsLegalHit(const SpamEntityOperation entityOp) const wxOVERRIDE;
    bool IsIntersection(const Geom::Rect &box) const wxOVERRIDE;
    SelectionData HitTest(const Geom::Point &pt) const wxOVERRIDE;
    SelectionData HitTest(const Geom::Point &pt, const double WXUNUSED(sx), const double WXUNUSED(sy)) const wxOVERRIDE;
    Geom::OptRect GetBoundingBox() const wxOVERRIDE;

public:
    void Draw(Cairo::RefPtr<Cairo::Context> &cr, const std::vector<Geom::Rect> &invalidRects) const;
    void SetDraw(const int drawMode) { drawMode_ = drawMode; }
    cv::Rect BoundingBox() const { return cvRgn_->BoundingBox(); }
    cv::Ptr<cv::mvlab::Region> GetRegion() const { return cvRgn_; }

private:
    cv::Ptr<cv::mvlab::Region> cvRgn_;
    std::vector<std::vector<cv::Point2f>> outerCurves_;
    std::vector<std::vector<cv::Point2f>> innerCurves_;
    std::vector<cv::Rect2f> outerBBoxs_;
    std::vector<cv::Rect2f> innerBBoxs_;
    int drawMode_;
};

#endif //SPAM_UI_PROJS_FIXED_NODE_H
