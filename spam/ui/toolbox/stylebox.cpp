#include "stylebox.h"
#include <ui/spam.h>

StyleBox::StyleBox(wxWindow* parent)
: wxPanel(parent, kSpamID_TOOLPAGE_STYLE)
{
    auto rootSizer = new wxBoxSizer(wxVERTICAL);
    rootSizer->AddSpacer(5);
    auto styleSizer = new wxFlexGridSizer(2, 2, 2);
    styleSizer->AddGrowableCol(1, 1);
    styleSizer->SetFlexibleDirection(wxHORIZONTAL);
    styleSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Fill:")), wxSizerFlags().Right().Border(wxLEFT));
    styleSizer->Add(new wxColourPickerCtrl(this, kSpamID_TOOLBOX_GEOM_FILL_COLOR), wxSizerFlags(1).Expand().HorzBorder());
    styleSizer->AddSpacer(1);
    styleSizer->Add(new wxSpinCtrl(this, kSpamID_TOOLBOX_GEOM_FILL_ALPHA), wxSizerFlags(1).Expand().HorzBorder());
    styleSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Width:")), wxSizerFlags().Right().Border(wxLEFT));
    styleSizer->Add(new wxSpinCtrl(this, kSpamID_TOOLBOX_GEOM_STROKE_WIDTH), wxSizerFlags(1).Expand().HorzBorder());
    styleSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Stroke:")), wxSizerFlags().Right().Border(wxLEFT));
    styleSizer->Add(new wxColourPickerCtrl(this, kSpamID_TOOLBOX_GEOM_STROKE_COLOR), wxSizerFlags(1).Expand().HorzBorder());
    styleSizer->AddSpacer(1);
    styleSizer->Add(new wxSpinCtrl(this, kSpamID_TOOLBOX_GEOM_STROKE_ALPHA), wxSizerFlags(1).Expand().HorzBorder());
    rootSizer->Add(styleSizer, wxSizerFlags().Expand());

    TransferDataToUI();

    Bind(wxEVT_COLOURPICKER_CHANGED, &StyleBox::OnColorChanged, this, wxID_ANY);
    Bind(wxEVT_SPINCTRL, &StyleBox::OnStyleChanged, this, wxID_ANY);

    SetSizer(rootSizer);
}

StyleBox::~StyleBox()
{
}

void StyleBox::TransferDataToUI()
{
    auto fillColor = wxColour();
    fillColor.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomFillPaint, wxCYAN->GetRGBA()));

    auto fillColorCtrl = dynamic_cast<wxColourPickerCtrl *>(FindWindow(kSpamID_TOOLBOX_GEOM_FILL_COLOR));
    if (fillColorCtrl)
    {
        fillColorCtrl->SetColour(wxColour(fillColor.GetRGB()));
    }

    auto fillColorAlpha = dynamic_cast<wxSpinCtrl *>(FindWindow(kSpamID_TOOLBOX_GEOM_FILL_ALPHA));
    if (fillColorAlpha)
    {
        fillColorAlpha->SetRange(0, 255);
        fillColorAlpha->SetValue(fillColor.Alpha());
    }

    auto strokeWidthCtrl = dynamic_cast<wxSpinCtrl *>(FindWindow(kSpamID_TOOLBOX_GEOM_STROKE_WIDTH));
    if (strokeWidthCtrl)
    {
        strokeWidthCtrl->SetRange(1, 10);
        strokeWidthCtrl->SetValue(SpamConfig::Get<int>(cp_ToolGeomStrokeWidth, 1));
    }

    auto strokeColor = wxColour();
    strokeColor.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomStrokePaint, wxCYAN->GetRGBA()));

    auto strokeColorCtrl = dynamic_cast<wxColourPickerCtrl *>(FindWindow(kSpamID_TOOLBOX_GEOM_STROKE_COLOR));
    if (strokeColorCtrl)
    {
        strokeColorCtrl->SetColour(wxColour(strokeColor.GetRGB()));
    }

    auto strokeColorAlpha = dynamic_cast<wxSpinCtrl *>(FindWindow(kSpamID_TOOLBOX_GEOM_STROKE_ALPHA));
    if (strokeColorAlpha)
    {
        strokeColorAlpha->SetRange(0, 255);
        strokeColorAlpha->SetValue(strokeColor.Alpha());
    }
}

void StyleBox::TransferDataFromUI()
{
    wxColour fillColor;
    auto fillColorCtrl = dynamic_cast<wxColourPickerCtrl *>(FindWindow(kSpamID_TOOLBOX_GEOM_FILL_COLOR));
    if (fillColorCtrl)
    {
        fillColor = fillColorCtrl->GetColour();
    }

    unsigned char a = wxALPHA_OPAQUE;
    auto fillColorAlpha = dynamic_cast<wxSpinCtrl *>(FindWindow(kSpamID_TOOLBOX_GEOM_FILL_ALPHA));
    if (fillColorAlpha)
    {
        a = static_cast<unsigned char>(fillColorAlpha->GetValue());
    }

    if (fillColor.IsOk())
    {
        fillColor.Set(fillColor.Red(), fillColor.Green(), fillColor.Blue(), a);
        SpamConfig::Set(cp_ToolGeomFillPaint, fillColor.GetRGBA());
    }

    auto strokeWidthCtrl = dynamic_cast<wxSpinCtrl *>(FindWindow(kSpamID_TOOLBOX_GEOM_STROKE_WIDTH));
    if (strokeWidthCtrl)
    {
        SpamConfig::Set(cp_ToolGeomStrokeWidth, strokeWidthCtrl->GetValue());
    }

    wxColour strokeColor;
    auto strokeColorCtrl = dynamic_cast<wxColourPickerCtrl *>(FindWindow(kSpamID_TOOLBOX_GEOM_STROKE_COLOR));
    if (strokeColorCtrl)
    {
        strokeColor = strokeColorCtrl->GetColour();
    }

    a = wxALPHA_OPAQUE;
    auto strokeColorAlpha = dynamic_cast<wxSpinCtrl *>(FindWindow(kSpamID_TOOLBOX_GEOM_STROKE_ALPHA));
    if (strokeColorAlpha)
    {
        a = static_cast<unsigned char>(strokeColorAlpha->GetValue());
    }

    if (strokeColor.IsOk())
    {
        strokeColor.Set(strokeColor.Red(), strokeColor.Green(), strokeColor.Blue(), a);
        SpamConfig::Set(cp_ToolGeomStrokePaint, strokeColor.GetRGBA());
    }
}

void StyleBox::OnColorChanged(wxColourPickerEvent &e)
{
    TransferDataFromUI();
}

void StyleBox::OnStyleChanged(wxSpinEvent& e)
{
    TransferDataFromUI();
}