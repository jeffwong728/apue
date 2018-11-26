#ifndef SPAM_UI_PROJS_GENERIC_BEZIERGONNODE_NODE_H
#define SPAM_UI_PROJS_GENERIC_BEZIERGONNODE_NODE_H
#include "modelfwd.h"
#include "modelnode.h"
#include "geomnode.h"
namespace Geom {
    class PathVector;
    class Rect;
}

class BeziergonNode : public GeomNode
{
    struct Memento
    {
        DrawStyle style;
        BezierData data;
        long      rank;
        bool      visible;
        bool      locked;
    };

public:
    BeziergonNode() : GeomNode() { InitData(data_); }
    BeziergonNode(const SPModelNode &parent) : GeomNode(parent) { InitData(data_); }
    BeziergonNode(const SPModelNode &parent, const wxString &title);
    ~BeziergonNode();

public:
    bool IsContainer() const override { return false; }
    void BuildPath(Geom::PathVector &pv) const override;
    void BuildNode(Geom::PathVector &pv, NodeIdVector &ids) const override;
    SelectionData HitTest(const Geom::Point &pt) const override;
    SelectionData HitTest(const Geom::Point &pt, const double sx, const double sy) const override;
    bool IsIntersection(const Geom::Rect &box) const override;
    Geom::OptRect GetBoundingBox() const override;
    Geom::OptRect GetBoundingBox(const Geom::PathVector &pv) const override;
    void StartTransform() override;
    void EndTransform() override;
    void ResetTransform() override;
    void NodeEdit(const Geom::Point &anchorPt, const Geom::Point &freePt, const Geom::Point &lastPt) override;
    void ResetNodeEdit() override;
    boost::any CreateMemento() const override;
    bool RestoreFromMemento(const boost::any &memento) override;
    void InitData(BezierData &data);
    void SetData(const BezierData &data) { data_ = data; }
    const BezierData& GetData() const { return data_; }
    void Clear() { data_.points.clear(); data_.ntypes.clear(); }
    void BuildPreviewPath(Geom::PathVector &pv) const;
    int GetNumCorners() const { return static_cast<int>(data_.points.size()); }
    void AddCorner(const Geom::Point &pt, const Geom::Point &c0, const Geom::Point &c1, const BezierNodeType t);
    void ReplaceBackCorner(const Geom::Point &pt, const Geom::Point &c0, const Geom::Point &c1, const BezierNodeType t);
    void PopCorner();
    BezierNodeType GetCorner(int pos, Geom::Point &corner, Geom::Point &c0, Geom::Point &c1) const;

public:
    void Save(const H5::Group &g) const override;
    void Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me) override;

    static std::string GetTypeName() { return std::string("BeziergonNode"); }
    static SPModelNode Create(const SPModelNode &parent, const wxString &title)
    { 
        return std::make_shared<BeziergonNode>(parent, title);
    }

protected:
    void DoTransform(const Geom::Affine &aff, const double dx, const double dy) override;
    std::string GetArcTypeName() const;
    GenericEllipseArcType GetArcTypeValue(const std::string &tName) const;
    void BuildPathImpl(Geom::PathVector &pv, const bool closePath) const;

private:
    BezierData data_;
    BezierData base_;
};

#endif //SPAM_UI_PROJS_GENERIC_BEZIERGONNODE_NODE_H