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
    void UpdateHistogram(const std::string &uuidTag, const cv::Mat &srcImg, const boost::any &roi);

protected:
    wxPanel * GetOptionPanel(const int toolIndex, wxWindow *parent) override;
    ToolOptions GetToolOptions() const override;

private:
    void OnToolEnter(const ToolOptions &toolOpts);
    void OnChannelChanged(wxCommandEvent& e);
    void OnFilterTypeChanged(wxCommandEvent& e);
    void OnThreshold(HistogramWidget *hist);
    void OnPyramidEnter(wxCommandEvent &e);
    void OnThresholdEnter(wxCommandEvent &e);

private:
    wxPanel *CreateFilterOption(wxWindow *parent);
    wxPanel *CreateThresholdOption(wxWindow *parent);
    wxPanel *CreatePyramidOption(wxWindow *parent);
    void UpdateSelectionFilter(void);
    void RePopulateChannelChoice(const int numChannels);
    void RePopulateHistogramProfiles(const std::vector<cv::Mat> &imags, const cv::Mat &mask);
    void ReThreshold();

private:
    wxChoice        *channelChoice_;
    HistogramWidget *hist_;
    cv::Mat img_;
    std::vector<cv::Mat> imgs_;
    std::string uuidStation_;
    int pyraLevel_ = 5;
    int minGray_ = 50;
    int maxGray_ = 200;

};
#endif //SPAM_UI_TOOLBOX_PROC_BOX_H