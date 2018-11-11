#ifndef SPAM_UI_PROJS_POLYGON_NODE_H
#define SPAM_UI_PROJS_POLYGON_NODE_H
#include "modelfwd.h"
#include "modelnode.h"
#include "geomnode.h"
namespace Geom {
    class PathVector;
}

class PolygonNode : public GeomNode
{
public:
    PolygonNode() : GeomNode() {}
    PolygonNode(const SPModelNode &parent) : GeomNode(parent) {}
    PolygonNode(const SPModelNode &parent, const wxString &title);
    ~PolygonNode();

public:
    bool IsContainer() const override { return false; }
    void BuildPath(Geom::PathVector &pv) const override;
    SelectionData HitTest(const Geom::Point &pt) const override;
    bool IsIntersection(const Geom::Rect &box) const override;
    void Translate(const double dx, const double dy) override;
    int GetNumCorners() const { return static_cast<int>(data_.points.size()); }
    void AddCorner(const Geom::Point &pt);
    void PopCorner();
    void GetCorner(int pos, Geom::Point &corner) const;
    void Clear() { data_.points.clear(); }
    void BuildOpenPath(Geom::PathVector &pv);

public:
    void Save(const H5::Group &g) const override;
    void Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me) override;

    static std::string GetTypeName() { return std::string("PolygonNode"); }
    static SPModelNode Create(const SPModelNode &parent, const wxString &title)
    { 
        return std::make_shared<PolygonNode>(parent, title);
    }

public:
    PolygonData data_;
};

#endif //SPAM_UI_PROJS_POLYGON_NODE_H