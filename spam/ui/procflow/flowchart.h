#ifndef SPAM_UI_PROCFLOW_FLOWCHART_WIDGET_H
#define SPAM_UI_PROCFLOW_FLOWCHART_WIDGET_H
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/dcgraph.h>
#include <boost/signals2.hpp>
#include "stepfwd.h"

class StepBase;

class FlowChart : public wxScrolledCanvas
{
public:
    typedef boost::signals2::keywords::mutex_type<boost::signals2::dummy_mutex> bs2_dummy_mutex;
    boost::signals2::signal_type<void(FlowChart *), bs2_dummy_mutex>::type sig_ThumbsMoved;

public:
    FlowChart(wxWindow* parent);

public:

protected:
    void OnEnterWindow(wxMouseEvent &e);
    void OnLeaveWindow(wxMouseEvent &e);
    void OnLeftMouseDown(wxMouseEvent &e);
    void OnLeftMouseUp(wxMouseEvent &e);
    void OnMouseMotion(wxMouseEvent &e);
    void OnPaint(wxPaintEvent &e);

protected:
    wxSize DoGetBestSize() const wxOVERRIDE { return wxSize(120, 300); }
    bool InheritsBackgroundColour() const { return true; }

private:
    void DrawBackground() const;

private:
    int gapX_{5};
    int gapY_{5};
    std::vector<SPStepBase> steps_;

    wxDECLARE_NO_COPY_CLASS(FlowChart);
};
#endif //SPAM_UI_PROCFLOW_FLOWCHART_WIDGET_H
