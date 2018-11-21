#ifndef SPAM_UI_PROJS_GENERIC_ELLIPSE_ARC_NODE_H
#define SPAM_UI_PROJS_GENERIC_ELLIPSE_ARC_NODE_H
#include "modelfwd.h"
#include "modelnode.h"
#include "geomnode.h"
namespace Geom {
    class PathVector;
    class Rect;
}

class GenericEllipseArcNode : public GeomNode
{
    struct Memento
    {
        DrawStyle style;
        GenericEllipseArcData  data;
        long      rank;
        bool      visible;
        bool      locked;
    };

public:
    GenericEllipseArcNode() : GeomNode() { InitData(data_); }
    GenericEllipseArcNode(const SPModelNode &parent) : GeomNode(parent) { InitData(data_); }
    GenericEllipseArcNode(const SPModelNode &parent, const wxString &title);
    ~GenericEllipseArcNode();

public:
    bool IsContainer() const override { return false; }
    void BuildPath(Geom::PathVector &pv) const override;
    void BuildNode(Geom::PathVector &pv, NodeIdVector &ids) const override;
    SelectionData HitTest(const Geom::Point &pt) const override;
    SelectionData HitTest(const Geom::Point &pt, const double sx, const double sy) const override;
    bool IsIntersection(const Geom::Rect &box) const override;
    void StartTransform() override;
    void EndTransform() override;
    void ResetTransform() override;
    void NodeEdit(const Geom::Point &anchorPt, const Geom::Point &freePt, const double dx, const double dy) override;
    void ResetNodeEdit() override;
    boost::any CreateMemento() const override;
    bool RestoreFromMemento(const boost::any &memento) override;
    void InitData(GenericEllipseArcData &data);
    void SetData(const GenericEllipseArcData &data) { data_ = data; }
    const GenericEllipseArcData& GetData() const { return data_; }

public:
    void Save(const H5::Group &g) const override;
    void Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me) override;

    static std::string GetTypeName() { return std::string("GenericEllipseArcNode"); }
    static SPModelNode Create(const SPModelNode &parent, const wxString &title)
    { 
        return std::make_shared<GenericEllipseArcNode>(parent, title);
    }

protected:
    void DoTransform(const Geom::Affine &aff, const double dx, const double dy) override;

private:
    GenericEllipseArcData data_;
    GenericEllipseArcData base_;
};

#endif //SPAM_UI_PROJS_GENERIC_ELLIPSE_ARC_NODE_H