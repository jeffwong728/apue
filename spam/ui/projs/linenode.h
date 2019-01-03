#ifndef SPAM_UI_PROJS_LINE_NODE_H
#define SPAM_UI_PROJS_LINE_NODE_H
#include "modelfwd.h"
#include "modelnode.h"
#include "geomnode.h"
namespace Geom {
    class PathVector;
    class Rect;
}

class LineNode : public GeomNode
{
    struct Memento
    {
        DrawStyle style;
        LineData  data;
        long      rank;
        bool      visible;
        bool      locked;
    };

public:
    LineNode() : GeomNode() { InitData(data_); }
    LineNode(const SPModelNode &parent) : GeomNode(parent) { InitData(data_); }
    LineNode(const SPModelNode &parent, const wxString &title);
    ~LineNode();

public:
    bool IsTypeOf(const SpamEntityType t) const override;
    bool IsLegalHit(const SpamEntityOperation entityOp) const override;
    bool IsContainer() const override { return false; }
    void BuildPath(Geom::PathVector &pv) const override;
    void BuildNode(Geom::PathVector &pv, NodeIdVector &ids) const override;
    void BuildEdge(CurveVector &pth, NodeIdVector &ids) const override;
    SelectionData HitTest(const Geom::Point &pt) const override;
    SelectionData HitTest(const Geom::Point &pt, const double sx, const double sy) const override;
    bool IsHitFace(const Geom::Point &pt, const Geom::PathVector &pv) const override;
    bool IsIntersection(const Geom::Rect &box) const override;
    void StartTransform() override;
    void EndTransform() override;
    void ResetTransform() override;
    void NodeEdit(const Geom::Point &anchorPt, const Geom::Point &freePt, const Geom::Point &lastPt) override;
    void ResetNodeEdit() override;
    boost::any CreateMemento() const override;
    bool RestoreFromMemento(const boost::any &memento) override;
    void InitData(LineData &data);
    void SetData(const LineData &data) { data_ = data; }
    const LineData& GetData() const { return data_; }

public:
    void Save(const H5::Group &g) const override;
    void Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me) override;

    static std::string GetTypeName() { return std::string("LineNode"); }
    static SPModelNode Create(const SPModelNode &parent, const wxString &title)
    { 
        return std::make_shared<LineNode>(parent, title);
    }

protected:
    void DoTransform(const Geom::Affine &aff, const double dx, const double dy) override;

private:
    LineData data_;
    LineData base_;
};

#endif //SPAM_UI_PROJS_LINE_NODE_H