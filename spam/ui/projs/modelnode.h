#ifndef SPAM_UI_PROJS_MODEL_NODE_H
#define SPAM_UI_PROJS_MODEL_NODE_H
#include <memory>
#include <vector>
#include <boost/core/noncopyable.hpp>
#include <boost/uuid/uuid.hpp>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <ui/cmds/cmdsfwd.h>
#include "drawstyle.h"
#include "modelfwd.h"

class NodeFactory;
namespace H5 
{
    class Group;
}

class ModelNode : private boost::noncopyable
{
public:
    ModelNode();
    ModelNode(const SPModelNode &parent);
    ModelNode(const SPModelNode &parent, const wxString &title);
    virtual ~ModelNode();

public:
    virtual bool IsContainer() const = 0;
    virtual bool IsCurrentStation() const { return false; };
    virtual EntitySigType GetCreateSigType() const;
    virtual EntitySigType GetAddSigType() const;
    virtual EntitySigType GetDeleteSigType() const;

public:
    virtual void Save(const H5::Group &g) const;
    virtual void Load(const H5::Group &g, const NodeFactory &nf, const SPModelNode &me);

public:
    bool isVisible() const { return visible_; }
    const wxString &GetTitle() const { return title_; }
    long GetRank() const { return rank_; }
    void SetRank(const long rank) { rank_ = rank; }
    SPModelNode GetParent() { return parent_.lock(); }
    SPModelNode GetParent() const { return parent_.lock(); }
    SPModelNodeVector &GetChildren() { return children_; }
    const SPModelNodeVector &GetChildren() const { return children_; }
    SPModelNode GetNthChild(int n) { return children_.at(n); }
    SPCModelNode GetNthChild(int n) const { return children_.at(n); }
    SPModelNode  FindChild(const ModelNode *const child);
    void RemoveChild(const ModelNode *const child);
    void RemoveAllChildren() { children_.clear(); }
    void GetAllAncestors(SPModelNodeVector &ances) const;

    void Insert(const SPModelNode &child, int n) { children_.insert(std::next(children_.begin(), n), child); }
    void Append(const SPModelNode &child) { children_.push_back(child); }
    std::size_t GetChildCount() const { return children_.size(); }
    std::string GetUUIDTag() const;

public:
    wxString  title_;
    DrawStyle drawStyle_;
    bool      visible_;
    bool      locked_;
    boost::uuids::uuid tag_;
    long      rank_;

private:
    WPModelNode parent_;
    SPModelNodeVector children_;
};

#endif //SPAM_UI_PROJS_MODEL_NODE_H