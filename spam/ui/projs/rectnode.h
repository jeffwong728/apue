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
    struct Memento
    {
        DrawStyle style;
        RectData  data;
        long      rank;
        bool      visible;
        bool      locked;
    };

public:
    RectNode() : GeomNode() { InitData(data_); }
    RectNode(const SPModelNode &parent) : GeomNode(parent) { InitData(data_); }
    RectNode(const SPModelNode &parent, const wxString &title);
    ~RectNode();

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
    void ResetNodeEdit() override;
    boost::any CreateMemento() const override;
    bool RestoreFromMemento(const boost::any &memento) override;
    void InitData(RectData &data);

public:
    void Save(const H5::Group &g) const override;
    void Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me) override;

    static std::string GetTypeName() { return std::string("RectNode"); }
    static SPModelNode Create(const SPModelNode &parent, const wxString &title)
    { 
        return std::make_shared<RectNode>(parent, title);
    }

protected:
    void DoTransform(const Geom::Affine &aff, const double dx, const double dy) override;

public:
    RectData data_;
    RectData base_;
};

#endif //SPAM_UI_PROJS_STATION_NODE_H