#ifndef SPAM_UI_PROJS_RECT_NODE_H
#define SPAM_UI_PROJS_RECT_NODE_H
#include "modelfwd.h"
#include "modelnode.h"
#include "geomnode.h"
namespace Geom {
    class PathVector;
    class Rect;
}

class RectNode : public GeomNode
{
public:
    RectNode() : GeomNode() { InitData(); }
    RectNode(const SPModelNode &parent) : GeomNode(parent) { InitData(); }
    RectNode(const SPModelNode &parent, const wxString &title);
    ~RectNode();

public:
    bool IsContainer() const override { return false; }
    void BuildPath(Geom::PathVector &pv) const override;
    void BuildCorners(const Geom::PathVector &pv, Geom::Point(&corners)[4]) const override;
    SelectionData HitTest(const Geom::Point &pt) const override;
    SelectionData HitTest(const Geom::Point &pt, const double sx, const double sy) const override;
    bool IsIntersection(const Geom::Rect &box) const override;
    void Translate(const double dx, const double dy) override;
    void Transform(const Geom::Point &anchorPt, const Geom::Point &freePt, const double dx, const double dy) override;
    void InitData();

public:
    void Save(const H5::Group &g) const override;
    void Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me) override;

    static std::string GetTypeName() { return std::string("RectNode"); }
    static SPModelNode Create(const SPModelNode &parent, const wxString &title)
    { 
        return std::make_shared<RectNode>(parent, title);
    }

public:
    RectData data_;
};

#endif //SPAM_UI_PROJS_STATION_NODE_H