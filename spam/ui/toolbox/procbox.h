#ifndef SPAM_UI_TOOLBOX_PROC_BOX_H
#define SPAM_UI_TOOLBOX_PROC_BOX_H
#include "toolbox.h"
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include "wx/choice.h"
#include "wx/textctrl.h"
#include <ui/cmndef.h>
#include <ui/misc/histwidget.h>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/mvlab.hpp>
#include <boost/any.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/2geom.h>
#pragma warning( pop )

class ProcBox : public ToolBox
{
public:
    ProcBox(wxWindow* parent);
    ~ProcBox();

public:
    void UpdateUI(const int toolId, const std::string &uuidTag, const boost::any &params) override;

protected:
    wxPanel * GetOptionPanel(const int toolIndex, wxWindow *parent) override;
    ToolOptions GetToolOptions() const override;

private:
    void OnToolEnter(const ToolOptions &toolOpts);
    void OnChannelChanged(wxCommandEvent& e);
    void OnFilterTypeChanged(wxCommandEvent& e);
    void OnFilterBorderTypeChanged(wxCommandEvent& e);
    void OnFilterEnter(wxCommandEvent &e);
    void OnPyramidEnter(wxCommandEvent &e);
    void OnThresholdEnter(wxCommandEvent &e);
    void OnEdgeEnter(wxCommandEvent &e);
    void OnEdgeTypeChanged(wxCommandEvent &e);
    void OnEdgeChannelChanged(wxCommandEvent &e);
    void OnConvertChannelChanged(wxCommandEvent &e);

private:
    wxPanel *CreateFilterOption(wxWindow *parent);
    wxPanel *CreateThresholdOption(wxWindow *parent);
    wxPanel *CreatePyramidOption(wxWindow *parent);
    wxPanel *CreateEdgeOption(wxWindow *parent);
    wxPanel *CreateConvertOption(wxWindow *parent);
    void UpdateSelectionFilter(void);
    void UpdateEdgeUI(const std::string &uuidTag, const boost::any &roi);
    void UpdateThresholdUI(const std::string &uuidTag, const boost::any &roi);
    void UpdateConvertUI(const std::string &uuidTag, const boost::any &roi);
    void PopulateChannelChoice(wxChoice *choiceCtrl, const int numChannels);
    void PopulateHistogramProfiles(const std::vector<cv::Mat> &imags, const cv::Mat &mask);

private:
    HistogramWidget *hist_;
    wxChoice *edgeChannelChoice_;
    wxChoice *threshChannelChoice_;
    wxChoice *convertChannelChoice_;
    std::map<std::string, int> iParams_;
    std::map<std::string, double> fParams_;
};
#endif //SPAM_UI_TOOLBOX_PROC_BOX_H