#ifndef SPAM_UI_PROJS_NODE_FACTORY_H
#define SPAM_UI_PROJS_NODE_FACTORY_H
#include "modelnode.h"
#include "modelfwd.h"
#include <unordered_map>

class NodeFactory
{
public:
    NodeFactory();
    ~NodeFactory() {}

public:
    SPModelNode Create(const std::string &typeName, const SPModelNode &parent, const wxString &title) const;

private:
    std::unordered_map<std::string, SPModelNode(*)(const SPModelNode&, const wxString&)> nodeMakers_;
};

#endif //SPAM_UI_PROJS_NODE_FACTORY_H