#ifndef SPAM_UI_PROJS_PROFILE_NODE_H
#define SPAM_UI_PROJS_PROFILE_NODE_H
#include "modelfwd.h"
#include "drawablenode.h"
#include <opencv2/mvlab.hpp>
#include <cairomm/cairomm.h>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/path.h>
#pragma warning( pop )

class ProfileNode : public DrawableNode
{
public:
    ProfileNode(const SPModelNode &parent);
    ~ProfileNode();

public:
    bool IsTypeOf(const SpamEntityType t) const wxOVERRIDE;
    bool IsLegalHit(const SpamEntityOperation entityOp) const wxOVERRIDE;
    bool IsIntersection(const Geom::Rect &box) const wxOVERRIDE;
    SelectionData HitTest(const Geom::Point &pt) const wxOVERRIDE;
    SelectionData HitTest(const Geom::Point &pt, const double WXUNUSED(sx), const double WXUNUSED(sy)) const wxOVERRIDE;
    Geom::OptRect GetBoundingBox() const wxOVERRIDE;

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

public:
    void Draw(Cairo::RefPtr<Cairo::Context> &cr, const std::vector<Geom::Rect> &invalidRects) const;
    void SetData(const Geom::Point &begPoint, const Geom::Point &endPoint) { begPoint_ = begPoint; endPoint_ = endPoint; }

protected:
    void DoTransform(const Geom::Affine &WXUNUSED(aff), const double WXUNUSED(dx), const double WXUNUSED(dy)) wxOVERRIDE {}

protected:
    Geom::Point begPoint_;
    Geom::Point endPoint_;
};

#endif //SPAM_UI_PROJS_PROFILE_NODE_H
