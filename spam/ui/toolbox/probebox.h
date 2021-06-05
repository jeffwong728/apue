#ifndef SPAM_UI_TOOLBOX_PROBE_BOX_H
#define SPAM_UI_TOOLBOX_PROBE_BOX_H
#include "toolbox.h"
#include <ui/cmndef.h>
#include <ui/misc/histwidget.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include "wx/choice.h"
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#include <boost/any.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/2geom.h>
#pragma warning( pop )

class ProbeBox : public ToolBox
{
public:
    ProbeBox(wxWindow* parent);
    ~ProbeBox();

public:
    void UpdateProfile(const cv::Mat &srcImg, const Geom::Point &sPt, const Geom::Point &ePt);
    void UpdateHistogram(const cv::Mat &srcImg, const boost::any &roi);

protected:
    wxPanel * GetOptionPanel(const int toolIndex, wxWindow *parent) override;
    ToolOptions GetToolOptions() const override;

private:
    void OnProbeEntity(wxCommandEvent &cmd);
    void OnProbeRegion(wxCommandEvent &cmd);
    void OnToolEnter(const ToolOptions &toolOpts);
    void OnProfileChanged(wxCommandEvent& e);

private:
    wxPanel *CreateSelectOption(wxWindow *parent);
    wxPanel *CreateRegionOption(wxWindow *parent);
    wxPanel *CreateHistOption(wxWindow *parent);
    wxPanel *CreateProfileOption(wxWindow *parent);
    void UpdateSelectionFilter(void);
    void SetFeature(const RegionFeatureFlag ff) { regionProbeMask_ |= static_cast<uint64_t>(ff); }
    void ClearFeature(const RegionFeatureFlag ff) { regionProbeMask_ &= ~static_cast<uint64_t>(ff); }
    void RePopulateProfileChoice(const int numChannels);

private:
    int probeMode_{ kSpamID_TOOLBOX_PROBE_PIXEL };
    uint64_t regionProbeMask_{ 0 };
    HistogramWidget *hist_;
    HistogramWidget *profile_;
    wxChoice *profileChoice_;
    std::vector<cv::Mat> imags_;
    Geom::Point begPoint_;
    Geom::Point endPoint_;
};
#endif //SPAM_UI_TOOLBOX_PROBE_BOX_H