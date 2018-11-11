#ifndef SPAM_UI_PROJS_PROJ_NODE_H
#define SPAM_UI_PROJS_PROJ_NODE_H
#include "modelnode.h"
#include "modelfwd.h"

class ProjNode : public ModelNode
{
public:
    ProjNode() : ModelNode() {}
    ProjNode(const SPModelNode &parent) : ModelNode(parent) {}
    ProjNode(const SPModelNode &parent, const wxString &title);
    ~ProjNode();

public:
    bool IsContainer() const override { return true; }
    bool IsCurrentStation() const override { return false; }
    void SetPerspective(const std::string &perspective) { perspective_ = perspective; }
    const std::string &GetPerspective() const { return perspective_; }
    std::string &GetPerspective() { return perspective_; }

public:
    void Save(const H5::Group &g) const override;
    void Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me) override;

public:
    static std::string GetTypeName() { return std::string("ProjNode"); }
    static SPModelNode Create(const SPModelNode &parent, const wxString &title)
    {
        return std::make_shared<ProjNode>(parent, title);
    }

private:
    std::string perspective_;
};

#endif //SPAM_UI_PROJS_PROJ_NODE_H