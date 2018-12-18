#ifndef SPAM_UI_PROJS_POLYGON_NODE_H
#define SPAM_UI_PROJS_POLYGON_NODE_H
#include "modelfwd.h"
#include "modelnode.h"
#include "geomnode.h"

class PolygonNode : public GeomNode
{
    struct Memento
    {
        DrawStyle    style;
        PolygonData  data;
        long         rank;
        bool         visible;
        bool         locked;
    };

public:
    PolygonNode() : GeomNode() {}
    PolygonNode(const SPModelNode &parent) : GeomNode(parent) {}
    PolygonNode(const SPModelNode &parent, const wxString &title);
    ~PolygonNode();

public:
    bool IsTypeOf(const SpamEntityType t) const override;
    bool IsLegalHit(const SpamEntityOperation entityOp) const override;
    bool IsContainer() const override { return false; }
    void BuildPath(Geom::PathVector &pv) const override;
    void BuildNode(Geom::PathVector &pv, NodeIdVector &ids) const override;
    void BuildEdge(Geom::Path &pth, NodeIdVector &ids) const override;
    SelectionData HitTest(const Geom::Point &pt) const override;
    SelectionData HitTest(const Geom::Point &pt, const double sx, const double sy) const override;
    bool IsIntersection(const Geom::Rect &box) const override;
    void StartTransform() override;
    void EndTransform() override;
    void ResetTransform() override;
    void NodeEdit(const Geom::Point &anchorPt, const Geom::Point &freePt, const Geom::Point &lastPt) override;
    void ResetNodeEdit() override;
    SpamResult Modify(const Geom::Point &pt, const int editMode, const SelectionData &sd) override;
    boost::any CreateMemento() const override;
    bool RestoreFromMemento(const boost::any &memento) override;
    int GetNumCorners() const { return static_cast<int>(data_.points.size()); }
    void AddCorner(const Geom::Point &pt);
    void PopCorner();
    void GetCorner(int pos, Geom::Point &corner) const;
    void Clear() { data_.points.clear(); }
    void BuildOpenPath(Geom::PathVector &pv);
    void SetData(const PolygonData &data) { data_ = data; }
    const PolygonData& GetData() const { return data_; }

public:
    void Save(const H5::Group &g) const override;
    void Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me) override;

    static std::string GetTypeName() { return std::string("PolygonNode"); }
    static SPModelNode Create(const SPModelNode &parent, const wxString &title)
    { 
        return std::make_shared<PolygonNode>(parent, title);
    }

protected:
    void DoTransform(const Geom::Affine &aff, const double dx, const double dy) override;

private:
    PolygonData data_;
    PolygonData base_;
};

#endif //SPAM_UI_PROJS_POLYGON_NODE_H