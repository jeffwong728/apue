#ifndef SPAM_UI_PROJS_DRAWABLE_NODE_H
#define SPAM_UI_PROJS_DRAWABLE_NODE_H
#include "modelfwd.h"
#include "modelnode.h"
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif
#ifdef WINDING
#undef WINDING
#endif
#include <cairomm/cairomm.h>
namespace Geom {
    class Point;
    class Rect;
    class PathVector;
}

class DrawableNode : public ModelNode
{
public:
    DrawableNode() : ModelNode() {}
    DrawableNode(const SPModelNode &parent) : ModelNode(parent) {}
    DrawableNode(const SPModelNode &parent, const wxString &title);
    ~DrawableNode();

public:
    bool IsContainer() const override { return false; }
    virtual void BuildPath(Geom::PathVector &pv) const = 0;
    virtual void BuildHandle(const Geom::Point(&corners)[4], const double sx, const double sy, Geom::PathVector &hpv) const;
    virtual void BuildScaleHandle(const Geom::Point(&corners)[4], const double sx, const double sy, Geom::PathVector &hpv) const;
    virtual void BuildSkewHandle(const Geom::Point(&corners)[4], const double sx, const double sy, Geom::PathVector &hpv) const;
    virtual void BuildRotateHandle(const Geom::Point(&corners)[4], const double sx, const double sy, Geom::PathVector &hpv) const;
    virtual void BuildBox(const Geom::Point(&corners)[4], Geom::PathVector &bpv) const;
    virtual void BuildCorners(const Geom::PathVector &pv, Geom::Point(&corners)[4]) const;
    virtual SelectionData HitTest(const Geom::Point &pt) const = 0;
    virtual SelectionData HitTest(const Geom::Point &pt, const double sx, const double sy) const;
    virtual bool IsIntersection(const Geom::Rect &box) const = 0;
    virtual void Transform(const Geom::Point &anchorPt, const Geom::Point &freePt, const double dx, const double dy) = 0;
    void Draw(Cairo::RefPtr<Cairo::Context> &cr) const;
    void DrawHighlight(Cairo::RefPtr<Cairo::Context> &cr) const;
    void SetHighlightData(const HighlightData hd) { hlData_ = hd; }
    HighlightData GetHighlightData() const { return hlData_; }
    void ClearSelection();
    void ClearHighlight();
    void HighlightFace();
    void SelectEntity();
    void SwitchSelectionState();

public:
    SelectionData selData_;
    HighlightData hlData_;
};

#endif //SPAM_UI_PROJS_DRAWABLE_NODE_H