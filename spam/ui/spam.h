#ifndef SPAM_UI_SPAM_H
#define SPAM_UI_SPAM_H
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

class Spam : private boost::noncopyable
{
public:
    Spam() = delete;

public:
    static ProjTreeModel *GetModel(void);
    static void PopupPyError();
    static void ClearPyOutput();
    static void LogPyOutput();
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