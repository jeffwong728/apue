#ifndef SPAM_UI_TOOLBOX_IMG_FLOW_BOX_H
#define SPAM_UI_TOOLBOX_IMG_FLOW_BOX_H
#include <ui/cmndef.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/clrpicker.h>
#include <wx/spinctrl.h>
#include <tuple>
#include <vector>
#include <boost/signals2.hpp>

class FlowChart;
namespace bs2 = boost::signals2;

class ImgFlowBox : public wxPanel
{
public:
    ImgFlowBox(wxWindow* parent);
    ~ImgFlowBox();

public:
    void TransferDataToUI();
    void TransferDataFromUI();

private:
    void OnColorChanged(wxColourPickerEvent &e);
    void OnStyleChanged(wxSpinEvent& e);

private:
    FlowChart *imgProcFlowChart_;
};
#endif //SPAM_UI_TOOLBOX_IMG_FLOW_BOX_H