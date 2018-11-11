#ifndef SPAM_UI_PROJS_GEOM_NODE_H
#define SPAM_UI_PROJS_GEOM_NODE_H
#include "modelfwd.h"
#include "modelnode.h"
#include "drawablenode.h"
namespace Geom {
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
    virtual void Translate(const double dx, const double dy) = 0;

public:
    bool IsContainer() const wxOVERRIDE { return false; }

public:
    EntitySigType GetCreateSigType() const wxOVERRIDE;
    EntitySigType GetAddSigType() const wxOVERRIDE;
    EntitySigType GetDeleteSigType() const wxOVERRIDE;
};

#endif //SPAM_UI_PROJS_GEOM_NODE_H