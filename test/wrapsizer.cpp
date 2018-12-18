#include <iostream>
#include <string>
#include <vector>
// wxWidgets "Hello World" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <opencv2/highgui.hpp>
#include <SkData.h>
#include <SkImage.h>
#include <SkStream.h>
#include <SkSurface.h>
#include <SkCanvas.h>
#include <SkPath.h>
#include <SkMaskFilter.h>
#include <SkBlurMaskFilter.h>
#include <wxSVG/SVGDocument.h>

void raster(int width, int height, void(*draw)(SkCanvas*), const char* path) {
    sk_sp<SkSurface> rasterSurface = SkSurface::MakeRasterN32Premul(width, height);
    SkCanvas* rasterCanvas = rasterSurface->getCanvas();
    draw(rasterCanvas);
    sk_sp<SkImage> img(rasterSurface->makeImageSnapshot());
    if (!img) { return; }
    sk_sp<SkData> png(img->encodeToData());
    if (!png) { return; }
    SkFILEWStream out(path);
    (void)out.write(png->data(), png->size());
}

void example(SkCanvas* canvas) {
    const SkScalar scale = 256.0f;
    const SkScalar R = 0.45f * scale;
    const SkScalar TAU = 6.2831853f;
    SkPath path;
    path.moveTo(24, 108);
    for (int i = 0; i < 16; i++) {
        SkScalar sx, sy;
        sx = SkScalarSinCos(i * SK_ScalarPI / 8, &sy);
        path.rCubicTo(40 * sx, 4 * sy, 4 * sx, 40 * sy, 40 * sx, 40 * sy);
    }
    path.close();
    SkPaint p;
    p.setAntiAlias(true);
    p.setColor(SkColorSetARGB(255, 0xF9, 0xA6, 0x02));
    p.setStyle(SkPaint::kFill_Style);
    p.setStrokeWidth(1);
    p.setMaskFilter(SkMaskFilter::MakeBlur(kOuter_SkBlurStyle, 5));
    //p.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 5.0f));
    canvas->clear(SK_ColorTRANSPARENT);
    canvas->translate(0.5f * scale, 0.5f * scale);
    canvas->drawPath(path, p);
}

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};
class MyFrame : public wxFrame
{
public:
    MyFrame();
private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnPaint(wxPaintEvent& e);
};
enum
{
    ID_Hello = 1
};

wxIMPLEMENT_APP(MyApp);
bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame();
    wxInitAllImageHandlers();
    auto svgDoc = std::make_unique<wxSVGDocument>();
    svgDoc->Load(wxT("D:\\Data\\inkscape\\share\\icons\\hicolor\\scalable\\actions\\node-add.svg"));
    wxImage img = svgDoc->Render(128, 128, 0, true, true);
    img.SaveFile(wxT("D:\\apue.png"), wxBITMAP_TYPE_PNG);

    frame->Show(true);
    return true;
}
MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY, "Hello World")
{
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
        "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);
    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");
    Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_PAINT, &MyFrame::OnPaint, this, wxID_ANY);
}
void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}
void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets Hello World example",
        "About Hello World", wxOK | wxICON_INFORMATION);
}
void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}

void MyFrame::OnPaint(wxPaintEvent& e)
{
    wxPaintDC dc(this);
    PrepareDC(dc);
}