#ifndef SPAM_UI_PROJS_GEOM_NODE_H
#define SPAM_UI_PROJS_GEOM_NODE_H
#include "modelfwd.h"
#include "modelnode.h"
#include "drawablenode.h"
#include <ui/proc/rgn.h>
namespace Geom {
    class Point;
    class PathVector;
}

class GeomNode : public DrawableNode
{
public:
    GeomNode() : DrawableNode() {}
    GeomNode(const SPModelNode &parent) : DrawableNode(parent) {}
    GeomNode(const SPModelNode &parent, const wxString &title);
    ~GeomNode();

public:
    bool IsContainer() const wxOVERRIDE { return false; }
    SPSpamRgn ToRegion() const;

public:
    EntitySigType GetCreateSigType() const wxOVERRIDE;
    EntitySigType GetAddSigType() const wxOVERRIDE;
    EntitySigType GetDeleteSigType() const wxOVERRIDE;

private:
    static bool IsPointInside(Geom::PathVector &pv, const Geom::Point &pt);
};

#endif //SPAM_UI_PROJS_GEOM_NODE_H