#ifndef SPAM_UI_SPAM_H
#define SPAM_UI_SPAM_H
#include "cmndef.h"
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <string>
#include <boost/core/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <memory>
#include <ui/cmds/spamcmd.h>
#include <ui/projs/modelfwd.h>

class SpamConfig : private boost::noncopyable
{
public:
    SpamConfig() = delete;

public:
    static void Save();
    static std::unique_ptr<boost::property_tree::ptree> &GetPropertyTree();
    static void Set(const std::string &p, const wxString &v);
    template<typename T> static void Set(const std::string &p, const T &v);
    template<typename T> static T Get(const std::string &p, const T &v=T());
};

template<typename T>
void SpamConfig::Set(const std::string &p, const T &v)
{
    const auto &tree = GetPropertyTree();
    tree->put(p, v);
}

template<>
wxString SpamConfig::Get<wxString>(const std::string &p, const wxString &v);

template<typename T>
T SpamConfig::Get(const std::string &p, const T &v)
{
    const auto &tree = GetPropertyTree();
    return tree->get(p, v);
}

class SelectionFilter
{
public:
    SelectionFilter() : entityOp_(SpamEntityOperation::kEO_NONE) { AddAllPassType(); }

public:
    bool IsPass(const SPDrawableNode &dn) const;

public:
    void AddPassType(const SpamEntityType et);
    void AddPassType(const std::vector<SpamEntityType> &ets);
    void ReplacePassType(const SpamEntityType et);
    void ReplacePassType(const std::vector<SpamEntityType> &ets);
    void Clear() { passTypes_.clear(); }
    void AddAllPassType();
    void SetEntityOperation(const SpamEntityOperation entityOp) { entityOp_ = entityOp; }
    SpamEntityOperation GetEntityOperation() const { return entityOp_; }
    void SetEntitySelectionMode(const SpamEntitySelectionMode selMode) { entitySelMode_ = selMode; }
    SpamEntitySelectionMode GetEntitySelectionMode() const { return entitySelMode_; }

private:
    SpamEntitySelectionMode entitySelMode_;
    SpamEntityOperation entityOp_;
    std::vector<SpamEntityType> passTypes_;
};

class Spam : private boost::noncopyable
{
public:
    Spam() = delete;

public:
    static ProjTreeModel *GetModel(void);
    static SelectionFilter *GetSelectionFilter(void);
    static void PopupPyError();
    static void ClearPyOutput();
    static void LogPyOutput();
    static SPDrawableNodeVector Difference(const SPDrawableNodeVector& lseq, const SPDrawableNodeVector& rseq);
    static SPDrawableNodeVector Intersection(const SPDrawableNodeVector& lseq, const SPDrawableNodeVector& rseq);
    static wxBitmap GetBitmap(const SpamIconPurpose ip, const std::string &bmName);
    static void SetStatus(const StatusIconType iconType, const wxString &text);
};

class SpamUndoRedo : private boost::noncopyable
{
public:
    SpamUndoRedo() = delete;

public:
    static void AddCommand(const std::shared_ptr<SpamCmd> &cmd);
    static void Undo(void);
    static void Redo(void);

    static bool IsUndoable(void);
    static bool IsRedoable(void);
};

#endif //SPAM_UI_SPAM_H