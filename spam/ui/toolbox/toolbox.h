#ifndef SPAM_UI_TOOLBOX_TOOL_BOX_H
#define SPAM_UI_TOOLBOX_TOOL_BOX_H
#include <ui/cmndef.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <tuple>
#include <vector>
#include <boost/any.hpp>
#include <boost/signals2.hpp>
class wxCollapsiblePaneEvent;
class wxToggleButton;
class wxCollapsiblePane;
namespace bs2 = boost::signals2;

class ToolBox : public wxPanel
{
public:
    ToolBox(wxWindow* parent, const wxWindowID winid, const wxString &toolboxName, const std::vector<wxString> &toolGroupNames, const int numTools, const int startToolId);
    ~ToolBox();

protected:
    void Init(const wxWindowID toolIds[], const wxString toolTips[], const wxBitmap toolIcons[], const int numTools, const int startIndex, const int toolGroup);
    virtual wxPanel *GetOptionPanel(const int toolIndex, wxWindow *parent) = 0;
    virtual ToolOptions GetToolOptions() const = 0;

public:
    virtual void OpenToolbox();
    virtual void QuitToolbox();
    virtual void UpdateUI(const int toolId, const std::string &uuidTag, const boost::any &params) {}

public:
    bs2::signal_type<void(const ToolOptions &), bs2::keywords::mutex_type<bs2::dummy_mutex>>::type sig_ToolEnter;
    bs2::signal_type<void(int), bs2::keywords::mutex_type<bs2::dummy_mutex>>::type sig_ToolQuit;
    bs2::signal_type<void(const ToolOptions &), bs2::keywords::mutex_type<bs2::dummy_mutex>>::type sig_OptionsChanged;

protected:
    void OnToolCollapse(wxCollapsiblePaneEvent &e);
    void OnHelpCollapse(wxCollapsiblePaneEvent &e);
    void OnTool(wxCommandEvent &e);

private:
    bool CreateOption(const int toolIndex);
    void OnToolQuit(int toolId);

protected:
    const int startToolId_;
    std::string uuidStation_;
    wxCollapsiblePane *collToolPane_;
    wxCollapsiblePane *collOptPane_;
    std::vector<std::tuple<wxToggleButton*, wxPanel*, wxSizerItem*>> tools_;
    std::vector<wxSizer *> toolGroupSizers_;
};
#endif //SPAM_UI_TOOLBOX_TOOL_BOX_H