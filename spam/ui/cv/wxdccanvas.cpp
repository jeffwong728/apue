#include "wxdccanvas.h"
#include <wx/graphics.h>
#include <algorithm>
//#include <tbb/tbb.h>

WXDCCanvas::WXDCCanvas(wxWindow* parent, const std::string &cvWinName)
: wxScrolledCanvas(parent, wxID_ANY, wxPoint(0, 0), wxDefaultSize)
, cvWndName_(cvWinName.c_str())
{
    SetScrollRate(6, 6);
    Connect(wxEVT_SIZE, wxSizeEventHandler(WXDCCanvas::OnSize));
    Bind(wxEVT_CHAR, &WXDCCanvas::OnChar, this, wxID_ANY);
    Bind(wxEVT_LEFT_DOWN, &WXDCCanvas::OnLeftMouseDown, this, wxID_ANY);
    Bind(wxEVT_PAINT, &WXDCCanvas::OnPaint, this, wxID_ANY);
}

WXDCCanvas::~WXDCCanvas()
{
}

void WXDCCanvas::ShowImage(const cv::Mat &img)
{
    wxClientDC dc(this);
    Draw(dc);
}

void WXDCCanvas::SetImageWndSize(const wxSize &s)
{
    SetVirtualSize(s);
}

void WXDCCanvas::OnSize(wxSizeEvent& event)
{
    auto cSize = GetClientSize();
}

void WXDCCanvas::OnLeftMouseDown(wxMouseEvent &e)
{
    if (!HasFocus())
    {
        SetFocus();
    }
 
    auto pt = e.GetPosition();
    wxLogMessage(wxT("WXDCCanvas::OnLeftMouseDown (x=%d, y=%d)"), pt.x, pt.y);

    wxGraphicsRenderer *renders[] =
        {
            wxGraphicsRenderer::GetDefaultRenderer(),
            wxGraphicsRenderer::GetCairoRenderer()
#ifdef __WXMSW__
            ,wxGraphicsRenderer::GetGDIPlusRenderer(),
            wxGraphicsRenderer::GetDirect2DRenderer()
#endif
            };

    wxString::const_pointer strs[] =
        {
            wxT("DefaultRenderer"),
            wxT("CairoRenderer")
#ifdef __WXMSW__
            ,wxT("GDIPlusRenderer"),
            wxT("Direct2DRenderer")
#endif
            };

#ifdef __WXMSW__
    std::array<int, 4> indices = { 0,1,2,3 };
#else
    std::array<int, 2> indices = {0, 1};
#endif

    for (auto i : indices)
    {
        if (renders[i])
        {
            wxLogMessage(wxT("%s : %s"), strs[i], renders[i]->GetName().wx_str());
        }
        else
        {
            wxLogMessage(wxT("%s : not existed"), strs[i]);
        }
    }
}

void WXDCCanvas::OnChar(wxKeyEvent &e)
{
    wxString strC(static_cast<char>(e.GetKeyCode()), 1);
    strC.UpperCase();
    wxLogMessage(wxT("WXDCCanvas::OnChar - ")+strC);
}

void WXDCCanvas::OnPaint(wxPaintEvent& e)
{
    wxPaintDC dc(this);
    Draw(dc);
}

void WXDCCanvas::Draw(wxDC &dc)
{
    //tbb::tick_count t0 = tbb::tick_count::now();
#ifdef __WXMSW__
    auto wxhdc = dc.GetHDC();
    if (wxhdc)
    {
    }
#endif
    //tbb::tick_count t1 = tbb::tick_count::now();
    //wxLogMessage(wxT("Draw cost %fms"), (t1 - t0).seconds()*1000);
}