#include "cairocanvas.h"
#include <ui/cmndef.h>
#include <ui/spam.h>
#include <ui/projs/stationnode.h>
#include <ui/projs/geomnode.h>
#include <ui/projs/drawablenode.h>
#include <ui/projs/projtreemodel.h>
#include <ui/cmds/geomcmd.h>
#include <ui/cmds/transcmd.h>
#include <wx/graphics.h>
#include <algorithm>
#include <cairomm/win32_surface.h>
//#include <tbb/tbb.h>
#include <ui/misc/scopedtimer.h>
#include <ui/evts.h>
#include <2geom/cairo-path-sink.h>

CairoCanvas::CairoCanvas(wxWindow* parent, const std::string &cvWinName, const wxString &uuidStation, const wxSize& size)
: wxScrolledCanvas(parent, wxID_ANY, wxPoint(0, 0), size)
, cvWndName_(cvWinName.c_str())
, stationUUID_(uuidStation)
, anchorX_(0)
, anchorY_(0)
{
    SetScrollRate(6, 6);
    Bind(wxEVT_SIZE,             &CairoCanvas::OnSize,            this, wxID_ANY);
    Bind(wxEVT_CHAR,             &CairoCanvas::OnChar,            this, wxID_ANY);
    Bind(wxEVT_ENTER_WINDOW,     &CairoCanvas::OnEnterWindow,     this, wxID_ANY);
    Bind(wxEVT_LEAVE_WINDOW,     &CairoCanvas::OnLeaveWindow,     this, wxID_ANY);
    Bind(wxEVT_LEFT_DOWN,        &CairoCanvas::OnLeftMouseDown,   this, wxID_ANY);
    Bind(wxEVT_LEFT_UP,          &CairoCanvas::OnLeftMouseUp,     this, wxID_ANY);
    Bind(wxEVT_MOTION,           &CairoCanvas::OnMouseMotion,     this, wxID_ANY);
    Bind(wxEVT_LEFT_DCLICK,      &CairoCanvas::OnLeftDClick,      this, wxID_ANY);
    Bind(wxEVT_MIDDLE_DOWN,      &CairoCanvas::OnMiddleDown,      this, wxID_ANY);
    Bind(wxEVT_PAINT,            &CairoCanvas::OnPaint,           this, wxID_ANY);
    Bind(wxEVT_ERASE_BACKGROUND, &CairoCanvas::OnEraseBackground, this, wxID_ANY);
    Bind(wxEVT_CONTEXT_MENU,     &CairoCanvas::OnContextMenu,     this, wxID_ANY);
    Bind(wxEVT_MENU,             &CairoCanvas::OnDeleteEntities,  this, kSpamID_DELETE_ENTITIES);

    SetDropTarget(new DnDImageFile(parent));
}

CairoCanvas::~CairoCanvas()
{
}

void CairoCanvas::ShowImage(const cv::Mat &img)
{
    int dph = img.depth();
    int cnl = img.channels();

    if (CV_8U == dph && (1==cnl || 3==cnl || 4==cnl))
    {
        if (1 == cnl)
        {
            cv::cvtColor(img, srcMat_, cv::COLOR_GRAY2BGRA);
        }
        else if (3 == cnl)
        {
            cv::cvtColor(img, srcMat_, cv::COLOR_BGR2BGRA);
        }
        else
        {
            srcMat_ = img;
        }

        wxSize sViewPort = GetClientSize();
        wxSize sToSize = GetDispMatSize(sViewPort, wxSize(srcMat_.cols, srcMat_.rows));
        SetVirtualSize(sToSize);
        MoveAnchor(sViewPort, sToSize);
        ScaleShowImage(sToSize);
    }
}

void CairoCanvas::ExtentImage()
{
    if (HasImage())
    {
        wxSize sViewPort = GetSize();
        wxSize sToSize = GetFitSize(sViewPort, wxSize(srcMat_.cols, srcMat_.rows));
        SetVirtualSize(sToSize);
        MoveAnchor(sViewPort, sToSize);
        ScaleShowImage(sToSize);
    }
}

void CairoCanvas::ScaleImage(double scaleVal)
{
    if (HasImage() && scaleVal>0 && scaleVal<10.000001)
    {
        wxSize sToSize(wxRound(srcMat_.cols*scaleVal), wxRound(srcMat_.rows*scaleVal));
        if (sToSize.GetWidth()>3 && sToSize.GetHeight()>3)
        {
            wxSize sViewPort = GetClientSize();
            SetVirtualSize(sToSize);
            MoveAnchor(sViewPort, sToSize);
            ScaleShowImage(sToSize);
        }
    }
}

void CairoCanvas::ScaleUp(double val)
{
    if (HasImage() && val > 0)
    {
        double fval = GetMatScale();
        if (fval > 0)
        {
            double newScale = std::min(10.0, fval * (1 + val));
            ScaleImage(newScale);
        }
    }
}

void CairoCanvas::ScaleDown(double val)
{
    if (HasImage() && val > 0 && val < 1)
    {
        double fval = GetMatScale();
        if (fval > 0)
        {
            double newScale = std::max(0.01, fval * (1 - val));
            ScaleImage(newScale);
        }
    }
}

double CairoCanvas::GetMatScale() const
{ 
    if (srcMat_.empty() || disMat_.empty())
    {
        return -1;
    }
    else
    {
        return (disMat_.rows + 0.0) / srcMat_.rows;
    } 
}

wxRealPoint CairoCanvas::ScreenToDispImage(const wxPoint &pt)
{
    auto uspt = CalcUnscrolledPosition(pt) - wxSize(anchorX_, anchorY_);

    if (srcMat_.empty() || disMat_.empty())
    {
        return wxRealPoint();
    }
    else
    {
        return wxRealPoint(uspt.x, uspt.y);
    }
}

wxRealPoint CairoCanvas::ScreenToImage(const wxPoint &pt)
{
    auto uspt = CalcUnscrolledPosition(pt) - wxSize(anchorX_, anchorY_);

    if (srcMat_.empty() || disMat_.empty())
    {
        return wxRealPoint();
    }
    else
    {
        auto sX = srcMat_.cols / (disMat_.cols + 0.0);
        auto sY = srcMat_.rows / (disMat_.rows + 0.0);

        return wxRealPoint((uspt.x + 0.5)*sX - 0.5, (uspt.y + 0.5)*sY - 0.5);
    }
}

void CairoCanvas::DrawDrawables(const SPDrawableNodeVector &des)
{
    wxClientDC dc(this);
    PrepareDC(dc);
    auto wxhdc = dc.GetHDC();

    if (wxhdc)
    {
        auto cairoDC = Cairo::Win32Surface::create(wxhdc);
        auto cr = Cairo::Context::create(cairoDC);
        cr->translate(anchorX_, anchorY_);
        cr->scale(GetMatScale(), GetMatScale());

        for (const auto &de : des)
        {
            if (de)
            {
                de->Draw(cr);
            }
        }
    }
}

void CairoCanvas::EraseDrawables(const SPDrawableNodeVector &des)
{
    wxClientDC dc(this);
    PrepareDC(dc);
    auto wxhdc = dc.GetHDC();

    if (wxhdc)
    {
        Geom::OptRect boundRect;
        for (const auto &de : des)
        {
            if (de)
            {
                Geom::PathVector pv;
                de->BuildPath(pv);
                boundRect.unionWith(pv.boundsFast());
            }
        }

        if (!boundRect.empty())
        {
            int x = wxRound(boundRect.get().left()*GetMatScale());
            int y = wxRound(boundRect.get().top()*GetMatScale());
            int w = wxRound(boundRect.get().width()*GetMatScale());
            int h = wxRound(boundRect.get().height()*GetMatScale());
            wxRect invalidRect(x, y, w, h);
            ConpensateHandle(invalidRect);
            invalidRect.Intersect(wxRect(0, 0, scrMat_.cols, scrMat_.rows));

            auto dstPtr = scrMat_.ptr(invalidRect.GetY(), invalidRect.GetX());
            auto srcPtr = disMat_.ptr(invalidRect.GetY(), invalidRect.GetX());
            for (int r = invalidRect.GetTop(); r<=invalidRect.GetBottom(); ++r)
            {
                ::memcpy(dstPtr, srcPtr, scrMat_.elemSize()*invalidRect.GetWidth());
                dstPtr += scrMat_.step1();
                srcPtr += disMat_.step1();
            }

            auto data = scrMat_.ptr(invalidRect.GetY(), invalidRect.GetX());
            auto imgSurf = Cairo::ImageSurface::create(data, Cairo::Surface::Format::RGB24, invalidRect.GetWidth(), invalidRect.GetHeight(), scrMat_.step1());
            auto cr = Cairo::Context::create(imgSurf);

            cr->translate(-invalidRect.GetX(), -invalidRect.GetY());
            cr->scale(GetMatScale(), GetMatScale());
            DrawEntities(cr);

            auto cairoDC = Cairo::Win32Surface::create(wxhdc);
            auto crScr = Cairo::Context::create(cairoDC);
            crScr->set_source(imgSurf, anchorX_ + invalidRect.GetX(), anchorY_ + invalidRect.GetY());
            crScr->paint();
        }
    }
}

void CairoCanvas::HighlightDrawable(const SPDrawableNode &de)
{
    if (de)
    {
        wxClientDC dc(this);
        PrepareDC(dc);

        auto wxhdc = dc.GetHDC();
        if (wxhdc)
        {
            Geom::PathVector pv;
            de->BuildPath(pv);

            Geom::OptRect boundRect = pv.boundsFast();
            if (!boundRect.empty())
            {
                int x = wxRound(boundRect.get().left()*GetMatScale());
                int y = wxRound(boundRect.get().top()*GetMatScale());
                int w = wxRound(boundRect.get().width()*GetMatScale());
                int h = wxRound(boundRect.get().height()*GetMatScale());
                wxRect invalidRect(x, y, w, h);
                ConpensateHandle(invalidRect);
                invalidRect.Intersect(wxRect(0, 0, scrMat_.cols, scrMat_.rows));

                auto dstPtr = scrMat_.ptr(invalidRect.GetY(), invalidRect.GetX());
                auto srcPtr = disMat_.ptr(invalidRect.GetY(), invalidRect.GetX());
                for (int r = invalidRect.GetTop(); r<invalidRect.GetBottom(); ++r)
                {
                    ::memcpy(dstPtr, srcPtr, scrMat_.elemSize()*invalidRect.GetWidth());
                    dstPtr += scrMat_.step1();
                    srcPtr += disMat_.step1();
                }

                auto data = scrMat_.ptr(invalidRect.GetY(), invalidRect.GetX());
                auto imgSurf = Cairo::ImageSurface::create(data, Cairo::Surface::Format::RGB24, invalidRect.GetWidth(), invalidRect.GetHeight(), scrMat_.step1());
                auto cr = Cairo::Context::create(imgSurf);

                cr->translate(-invalidRect.GetX(), -invalidRect.GetY());
                cr->scale(GetMatScale(), GetMatScale());
                DrawEntities(cr, de);

                auto cairoDC = Cairo::Win32Surface::create(wxhdc);
                auto crScr = Cairo::Context::create(cairoDC);
                crScr->set_source(imgSurf, anchorX_ + invalidRect.GetX(), anchorY_ + invalidRect.GetY());
                crScr->paint();
            }
        }
    }
}

void CairoCanvas::DimDrawable(const SPDrawableNode &de)
{
    EraseDrawables(SPDrawableNodeVector(1, de));
}

void CairoCanvas::DrawPathVector(const Geom::PathVector &pth, const Geom::OptRect &rect)
{
    if (!rect.empty())
    {
        wxClientDC dc(this);
        PrepareDC(dc);

        auto wxhdc = dc.GetHDC();
        if (wxhdc)
        {
            int x = wxRound(rect.get().left()*GetMatScale());
            int y = wxRound(rect.get().top()*GetMatScale());
            int w = wxRound(rect.get().width()*GetMatScale());
            int h = wxRound(rect.get().height()*GetMatScale());
            wxRect invalidRect(x, y, w, h);
            ConpensateHandle(invalidRect);
            invalidRect.Intersect(wxRect(0, 0, scrMat_.cols, scrMat_.rows));

            auto dstPtr = scrMat_.ptr(invalidRect.GetY(), invalidRect.GetX());
            auto srcPtr = disMat_.ptr(invalidRect.GetY(), invalidRect.GetX());
            for (int r = invalidRect.GetTop(); r <= invalidRect.GetBottom(); ++r)
            {
                ::memcpy(dstPtr, srcPtr, scrMat_.elemSize()*invalidRect.GetWidth());
                dstPtr += scrMat_.step1();
                srcPtr += disMat_.step1();
            }

            auto data = scrMat_.ptr(invalidRect.GetY(), invalidRect.GetX());
            auto imgSurf = Cairo::ImageSurface::create(data, Cairo::Surface::Format::RGB24, invalidRect.GetWidth(), invalidRect.GetHeight(), scrMat_.step1());
            auto cr = Cairo::Context::create(imgSurf);

            cr->translate(-invalidRect.GetX(), -invalidRect.GetY());
            cr->scale(GetMatScale(), GetMatScale());
            DrawEntities(cr);

            if (!pth.empty())
            {
                wxColour strokeColor;
                strokeColor.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomStrokePaint, wxBLUE->GetRGBA()));

                wxColour fillColor;
                fillColor.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomFillPaint, wxBLUE->GetRGBA()));

                double ux = SpamConfig::Get<int>(cp_ToolGeomStrokeWidth, 1);
                double uy = ux;
                cr->device_to_user_distance(ux, uy);
                if (ux < uy) ux = uy;

                Geom::Translate aff = Geom::Translate(0.5, 0.5);
                Geom::PathVector paths = pth * aff;

                cr->save();
                Geom::CairoPathSink cairoPathSink(cr->cobj());
                cairoPathSink.feed(paths);
                cr->set_source_rgba(fillColor.Red() / 255.0, fillColor.Green() / 255.0, fillColor.Blue() / 255.0, fillColor.Alpha() / 255.0);
                cr->fill_preserve();
                cr->set_line_width(ux);
                cr->set_source_rgba(strokeColor.Red() / 255.0, strokeColor.Green() / 255.0, strokeColor.Blue() / 255.0, strokeColor.Alpha() / 255.0);
                cr->stroke();
                cr->restore();
            }

            auto cairoDC = Cairo::Win32Surface::create(wxhdc);
            auto crScr = Cairo::Context::create(cairoDC);
            crScr->set_source(imgSurf, anchorX_ + invalidRect.GetX(), anchorY_ + invalidRect.GetY());
            crScr->paint();
        }
    }
}

void CairoCanvas::DrawBox(const Geom::Path &pth)
{
    wxClientDC dc(this);
    PrepareDC(dc);
    DrawBox(dc, pth);
}

void CairoCanvas::DrawBox(const Geom::OptRect &oldRect, const Geom::OptRect &newRect)
{
    wxClientDC dc(this);
    PrepareDC(dc);
    DrawBox(dc, oldRect, newRect);
}

void CairoCanvas::AddRect(const RectData &rd)
{
    auto model = Spam::GetModel();
    if (model)
    {
        auto station = model->FindStationByUUID(stationUUID_);
        if (station)
        {
            wxString title = wxString::Format(wxT("rect%d"), cRect_++);
            auto cmd = std::make_shared<CreateRectCmd>(model, station, title, rd);
            cmd->Do();
            SpamUndoRedo::AddCommand(cmd);
            wxLogStatus(cmd->GetDescription());
        }
    }
}

void CairoCanvas::DoEdit(const int toolId, const SPDrawableNodeVector &selEnts, const SpamMany &mementos)
{
    switch (toolId)
    {
    case kSpamID_TOOLBOX_GEOM_TRANSFORM: DoTransform(selEnts, mementos); break;
    case kSpamID_TOOLBOX_GEOM_EDIT:      DoNodeEdit(selEnts, mementos);  break;
    default: break;
    }
}

void CairoCanvas::AddPolygon(const PolygonData &pd)
{
    auto model = Spam::GetModel();
    if (model)
    {
        auto station = model->FindStationByUUID(stationUUID_);
        if (station)
        {
            wxString title = wxString::Format(wxT("polygon%d"), cPolygon_++);
            auto cmd = std::make_shared<CreatePolygonCmd>(model, station, title, pd);
            cmd->Do();
            SpamUndoRedo::AddCommand(cmd);
            wxLogStatus(cmd->GetDescription());
        }
    }
}

void CairoCanvas::DoTransform(const SPDrawableNodeVector &selEnts, const SpamMany &mementos)
{
    auto model = Spam::GetModel();
    if (model)
    {
        auto station = model->FindStationByUUID(stationUUID_);
        if (station)
        {
            auto cmd = std::make_shared<TransformCmd>(model, station, selEnts, mementos);
            cmd->Do();
            SpamUndoRedo::AddCommand(cmd);
            wxLogStatus(cmd->GetDescription());
        }
    }
}

void CairoCanvas::DoNodeEdit(const SPDrawableNodeVector &selEnts, const SpamMany &mementos)
{
    auto model = Spam::GetModel();
    if (model)
    {
        auto station = model->FindStationByUUID(stationUUID_);
        if (station)
        {
            auto cmd = std::make_shared<NodeEditCmd>(model, station, selEnts, mementos);
            cmd->Do();
            SpamUndoRedo::AddCommand(cmd);
            wxLogStatus(cmd->GetDescription());
        }
    }
}

SPStationNode CairoCanvas::GetStation()
{
    auto model = Spam::GetModel();
    if (model)
    {
        return model->FindStationByUUID(stationUUID_);
    }

    return SPStationNode();
}

SPDrawableNode CairoCanvas::FindDrawable(const Geom::Point &pt)
{
    SPStationNode sn = GetStation();
    if (sn)
    {
        return sn->FindDrawable(pt);
    }

    return SPDrawableNode();
}

SPDrawableNode CairoCanvas::FindDrawable(const Geom::Point &pt, const double sx, const double sy, SelectionData &sd)
{
    sd.ss = SelectionState::kSelNone;
    sd.hs = HitState::kHsNone;
    sd.id = -1;
    sd.subid = -1;
    sd.master = 0;

    SPStationNode sn = GetStation();
    if (sn)
    {
        return sn->FindDrawable(pt, sx, sy, sd);
    }

    return SPDrawableNode();
}

void CairoCanvas::SelectDrawable(const Geom::Rect &box, SPDrawableNodeVector &ents)
{
    SPStationNode sn = GetStation();
    if (sn)
    {
        return sn->SelectDrawable(box, ents);
    }
}

void CairoCanvas::OnSize(wxSizeEvent& e)
{
    if (!disMat_.empty())
    { 
        MoveAnchor(e.GetSize(), wxSize(disMat_.cols, disMat_.rows));
        Refresh(false);
    }
}

void CairoCanvas::OnEnterWindow(wxMouseEvent &e)
{
    sig_EnterWindow(e);
}

void CairoCanvas::OnLeaveWindow(wxMouseEvent &e)
{
    sig_LeaveWindow(e);
}

void CairoCanvas::OnLeftMouseDown(wxMouseEvent &e)
{
    if (!HasFocus())
    {
        SetFocus();
    }
    sig_LeftMouseDown(e);
}

void CairoCanvas::OnLeftMouseUp(wxMouseEvent &e)
{
    sig_LeftMouseUp(e);
}

void CairoCanvas::OnMouseMotion(wxMouseEvent &e)
{
    sig_MouseMotion(e);
}

void CairoCanvas::OnMiddleDown(wxMouseEvent &e)
{
    sig_MiddleDown(e);
}

void CairoCanvas::OnLeftDClick(wxMouseEvent &e)
{
    sig_LeftDClick(e);
}

void CairoCanvas::OnKeyDown(wxKeyEvent &e)
{
    sig_KeyDown(e);
}

void CairoCanvas::OnKeyUp(wxKeyEvent &e)
{
    sig_KeyUp(e);
}

void CairoCanvas::OnChar(wxKeyEvent &e)
{
    sig_Char(e);
}

void CairoCanvas::OnPaint(wxPaintEvent& e)
{
    wxPaintDC dc(this);
    PrepareDC(dc);

    dc.SetBrush(*wxBLACK_BRUSH);
    dc.SetPen(wxNullPen);

    auto cRect = GetClientRect();
    wxRect tRect{0, 0, cRect.GetWidth(), anchorY_};
    dc.DrawRectangle(tRect);

    wxRect lRect{ 0, 0, anchorX_, cRect.GetHeight() };
    dc.DrawRectangle(lRect);

    int x = std::min(anchorX_ + disMat_.cols, cRect.GetRight());
    int w = std::max(cRect.GetRight() - (anchorX_ + disMat_.cols) + 1, 0);
    wxRect rRect{ x, 0, w, cRect.GetHeight() };
    dc.DrawRectangle(rRect);

    int y = std::min(anchorY_ + disMat_.rows, cRect.GetBottom());
    int h = std::max(cRect.GetBottom() - (anchorY_ + disMat_.rows) + 1, 0);
    wxRect bRect{ 0, y, cRect.GetWidth(), h };
    dc.DrawRectangle(bRect);

    wxRegionIterator upd(GetUpdateRegion());
    while (upd)
    {
        int vX = upd.GetX(), vY = upd.GetY(), vW = upd.GetW(), vH = upd.GetH();
        auto tl = ScreenToImage(wxPoint(vX, vY));
        auto br = ScreenToImage(wxPoint(vX + vW -1, vY + vH -1));
        Geom::OptRect invalidRect(Geom::Point(tl.x, tl.y), Geom::Point(br.x, br.y));
        Draw(dc, invalidRect);

        upd++;
    }
}

void CairoCanvas::OnEraseBackground(wxEraseEvent &e)
{
}

void CairoCanvas::OnContextMenu(wxContextMenuEvent& e)
{
    auto model = Spam::GetModel();
    if (model)
    {
        auto station = model->FindStationByUUID(stationUUID_);
        if (station)
        {
            int numSel = station->GetNumSelected();
            int numDra = station->GetNumDrawable();

            wxMenu menu;
            menu.Append(kSpamID_DELETE_ENTITIES, wxT("Delete"))->Enable(numSel);
            menu.AppendSeparator();
            menu.Append(kSpamID_HIDE_ENTITIES, wxT("Hide"))->Enable(numSel);
            menu.Append(kSpamID_SHOW_ONLY_ENTITIES, wxT("Show Only"))->Enable(numSel);
            menu.Append(kSpamID_SHOW_REVERSE_ENTITIES, wxT("Show Reverse"))->Enable(numDra);
            menu.Append(kSpamID_SHOW_ALL_ENTITIES, wxT("Show All"))->Enable(numDra);
            menu.Append(kSpamID_HIDE_ALL_ENTITIES, wxT("Hide All"))->Enable(numDra);

            PopupMenu(&menu);
        }
    }
}

void CairoCanvas::OnDeleteEntities(wxCommandEvent &cmd)
{
    auto model = Spam::GetModel();
    if (model)
    {
        auto station = model->FindStationByUUID(stationUUID_);
        if (station)
        {
            SPDrawableNodeVector drawables = station->GeSelected();
            SPGeomNodeVector delGeoms;
            for (const auto &drawable : drawables)
            {
                auto geom = std::dynamic_pointer_cast<GeomNode>(drawable);
                if (geom)
                {
                    delGeoms.push_back(geom);
                }
            }

            if (!delGeoms.empty())
            {
                auto cmd = std::make_shared<DeleteGeomsCmd>(Spam::GetModel(), delGeoms);
                cmd->Do();
                SpamUndoRedo::AddCommand(cmd);
                wxLogStatus(cmd->GetDescription());
            }
        }
    }
}

void CairoCanvas::Draw(wxDC &dc, const Geom::OptRect &rect)
{
    auto wxhdc = dc.GetHDC();
    if (wxhdc)
    {
        if (!rect.empty())
        {
            int x = wxRound(rect.get().left()*GetMatScale());
            int y = wxRound(rect.get().top()*GetMatScale());
            int w = wxRound(rect.get().width()*GetMatScale());
            int h = wxRound(rect.get().height()*GetMatScale());
            wxRect invalidRect(x, y, w, h);
            ConpensateHandle(invalidRect);
            invalidRect.Intersect(wxRect(0, 0, scrMat_.cols, scrMat_.rows));

            auto dstPtr = scrMat_.ptr(invalidRect.GetY(), invalidRect.GetX());
            auto srcPtr = disMat_.ptr(invalidRect.GetY(), invalidRect.GetX());
            for (int r = invalidRect.GetTop(); r <= invalidRect.GetBottom(); ++r)
            {
                ::memcpy(dstPtr, srcPtr, scrMat_.elemSize()*invalidRect.GetWidth());
                dstPtr += scrMat_.step1();
                srcPtr += disMat_.step1();
            }

            auto data = scrMat_.ptr(invalidRect.GetY(), invalidRect.GetX());
            auto imgSurf = Cairo::ImageSurface::create(data, Cairo::Surface::Format::RGB24, invalidRect.GetWidth(), invalidRect.GetHeight(), scrMat_.step1());
            auto cr = Cairo::Context::create(imgSurf);

            cr->translate(-invalidRect.GetX(), -invalidRect.GetY());
            cr->scale(GetMatScale(), GetMatScale());
            DrawEntities(cr);

            auto cairoDC = Cairo::Win32Surface::create(wxhdc);
            auto crScr = Cairo::Context::create(cairoDC);
            crScr->set_source(imgSurf, anchorX_ + invalidRect.GetX(), anchorY_ + invalidRect.GetY());
            crScr->paint();
        }
    }
}

void CairoCanvas::Draw(wxDC &dc, const Geom::Path &pth)
{
    auto wxhdc = dc.GetHDC();
    if (wxhdc)
    {
        ::memcpy(scrMat_.data, disMat_.data, scrMat_.step1()*scrMat_.rows);
        auto imgSurf = Cairo::ImageSurface::create(scrMat_.data, Cairo::Surface::Format::RGB24, scrMat_.cols, scrMat_.rows, scrMat_.step1());
        auto cr = Cairo::Context::create(imgSurf);

        cr->scale(GetMatScale(), GetMatScale());
        DrawEntities(cr);

        wxColour strokeColor;
        strokeColor.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomStrokePaint, wxBLUE->GetRGBA()));

        wxColour fillColor;
        fillColor.SetRGBA(SpamConfig::Get<wxUint32>(cp_ToolGeomFillPaint, wxBLUE->GetRGBA()));

        double ux = SpamConfig::Get<int>(cp_ToolGeomStrokeWidth, 1);
        double uy = ux;
        cr->device_to_user_distance(ux, uy);
        if (ux < uy) ux = uy;

        cr->save();
        Geom::CairoPathSink cairoPathSink(cr->cobj());
        cairoPathSink.feed(pth);
        cr->set_source_rgba(fillColor.Red() / 255.0, fillColor.Green() / 255.0, fillColor.Blue() / 255.0, fillColor.Alpha() / 255.0);
        cr->fill_preserve();
        cr->set_line_width(ux);
        cr->set_source_rgba(strokeColor.Red() / 255.0, strokeColor.Green() / 255.0, strokeColor.Blue() / 255.0, strokeColor.Alpha() / 255.0);
        cr->stroke();
        cr->restore();

        auto cairoDC = Cairo::Win32Surface::create(wxhdc);
        auto crScr = Cairo::Context::create(cairoDC);
        crScr->set_source(imgSurf, anchorX_, anchorY_);
        crScr->paint();
    }
}

void CairoCanvas::DrawBox(wxDC &dc, const Geom::Path &pth)
{
    auto wxhdc = dc.GetHDC();
    if (wxhdc)
    {
        ::memcpy(scrMat_.data, disMat_.data, scrMat_.step1()*scrMat_.rows);
        auto imgSurf = Cairo::ImageSurface::create(scrMat_.data, Cairo::Surface::Format::RGB24, scrMat_.cols, scrMat_.rows, scrMat_.step1());
        auto cr = Cairo::Context::create(imgSurf);

        cr->scale(GetMatScale(), GetMatScale());
        DrawEntities(cr);

        double ux = 1, uy = 1;
        cr->device_to_user_distance(ux, uy);
        if (ux < uy) ux = uy;

        cr->save();
        Geom::CairoPathSink cairoPathSink(cr->cobj());
        Geom::Translate aff = Geom::Translate(0.5, 0.5);
        Geom::Path path = pth*aff;
        cairoPathSink.feed(path);
        cr->set_line_width(ux);
        cr->set_line_cap(Cairo::Context::LineCap::SQUARE);
        cr->set_source_rgba(1.0, 0.0, 1.0, 1.0);
        cr->stroke();
        cr->restore();

        auto cairoDC = Cairo::Win32Surface::create(wxhdc);
        auto crScr = Cairo::Context::create(cairoDC);
        crScr->set_source(imgSurf, anchorX_, anchorY_);
        crScr->paint();
    }
}

void CairoCanvas::DrawBox(wxDC &dc, const Geom::OptRect &oldRect, const Geom::OptRect &newRect)
{
    auto wxhdc = dc.GetHDC();
    if (!wxhdc) {
        return;
    }

    Geom::OptRect rect = oldRect;
    rect.unionWith(newRect);

    if (rect.empty()) {
        return;
    }

    int x = wxRound(rect.get().left()*GetMatScale());
    int y = wxRound(rect.get().top()*GetMatScale());
    int w = wxRound(rect.get().width()*GetMatScale());
    int h = wxRound(rect.get().height()*GetMatScale());
    wxRect invalidRect(x, y, w, h);
    ConpensateHandle(invalidRect);
    invalidRect.Intersect(wxRect(0, 0, scrMat_.cols, scrMat_.rows));

    auto dstPtr = scrMat_.ptr(invalidRect.GetY(), invalidRect.GetX());
    auto srcPtr = disMat_.ptr(invalidRect.GetY(), invalidRect.GetX());
    for (int r = invalidRect.GetTop(); r<=invalidRect.GetBottom(); ++r)
    {
        ::memcpy(dstPtr, srcPtr, scrMat_.elemSize()*invalidRect.GetWidth());
        dstPtr += scrMat_.step1();
        srcPtr += disMat_.step1();
    }

    auto data = scrMat_.ptr(invalidRect.GetY(), invalidRect.GetX());
    auto imgSurf = Cairo::ImageSurface::create(data, Cairo::Surface::Format::RGB24, invalidRect.GetWidth(), invalidRect.GetHeight(), scrMat_.step1());
    auto cr = Cairo::Context::create(imgSurf);

    cr->translate(-invalidRect.GetX(), -invalidRect.GetY());
    cr->scale(GetMatScale(), GetMatScale());
    DrawEntities(cr);

    if (!newRect.empty())
    {
        Geom::Path pth(newRect.get());
        pth *= Geom::Translate(0.5, 0.5);
        double ux = 1, uy = 1;
        cr->device_to_user_distance(ux, uy);
        if (ux < uy) ux = uy;

        cr->save();
        Geom::CairoPathSink cairoPathSink(cr->cobj());
        cairoPathSink.feed(pth);
        cr->set_line_width(ux);
        cr->set_line_cap(Cairo::Context::LineCap::SQUARE);
        cr->set_source_rgba(1.0, 0.0, 1.0, 1.0);
        cr->stroke();
        cr->restore();
    }

    auto cairoDC = Cairo::Win32Surface::create(wxhdc);
    auto crScr = Cairo::Context::create(cairoDC);
    crScr->set_source(imgSurf, anchorX_ + invalidRect.GetX(), anchorY_ + invalidRect.GetY());
    crScr->paint();
}

void CairoCanvas::DrawEntities(Cairo::RefPtr<Cairo::Context> &cr)
{
    auto station = GetStation();
    if (station)
    {
        for (const auto &c : station->GetChildren())
        {
            auto drawable = std::dynamic_pointer_cast<DrawableNode>(c);
            if (drawable)
            {
                drawable->Draw(cr);
            }
        }
    }
}

void CairoCanvas::DrawEntities(Cairo::RefPtr<Cairo::Context> &cr, const SPDrawableNode &highlight)
{
    auto station = GetStation();
    if (station)
    {
        for (const auto &c : station->GetChildren())
        {
            auto drawable = std::dynamic_pointer_cast<DrawableNode>(c);
            if (drawable != highlight)
            {
                drawable->Draw(cr);
            }
        }

        highlight->DrawHighlight(cr);
    }
}

void CairoCanvas::ConpensateHandle(wxRect &invalidRect) const
{
    invalidRect.Inflate(20, 20);
}

void CairoCanvas::MoveAnchor(const wxSize &sViewport, const wxSize &disMatSize)
{
    anchorX_ = std::max(0, (sViewport.GetWidth() - disMatSize.GetWidth()) / 2);
    anchorY_ = std::max(0, (sViewport.GetHeight() - disMatSize.GetHeight()) / 2);
}

void CairoCanvas::ScaleShowImage(const wxSize &sToSize)
{
    if (sToSize.GetWidth()>2 && sToSize.GetHeight()>2)
    {
        if (sToSize.GetWidth() == srcMat_.cols && sToSize.GetHeight() == srcMat_.rows)
        {
            disMat_ = srcMat_;
        }
        else
        {
            cv::Size newSize(sToSize.GetWidth(), sToSize.GetHeight());
            bool shrink = newSize.width<srcMat_.cols || newSize.height<srcMat_.rows;
            ScopedTimer st(wxT("cv::resize"));
            cv::resize(srcMat_, disMat_, newSize, 0.0, 0.0, shrink ? cv::INTER_AREA : cv::INTER_LINEAR);
        }
        scrMat_ = disMat_.clone();
        Refresh(false);
    }
}

wxSize CairoCanvas::GetDispMatSize(const wxSize &sViewport, const wxSize &srcMatSize)
{
    if (srcMatSize.GetWidth() <= sViewport.GetWidth() &&
        srcMatSize.GetHeight() <= sViewport.GetHeight())
    {
        return srcMatSize;
    }
    else
    {
        return GetFitSize(sViewport, srcMatSize);
    }
}

wxSize CairoCanvas::GetFitSize(const wxSize &sViewport, const wxSize &srcMatSize)
{
    double fw = (srcMatSize.GetWidth()+0.0) / sViewport.GetWidth();
    double fh = (srcMatSize.GetHeight() + 0.0) / sViewport.GetHeight();

    if (fw > fh)
    {
        return wxSize(sViewport.GetWidth(), srcMatSize.GetHeight()/fw);
    }
    else
    {
        return wxSize(srcMatSize.GetWidth()/fh, sViewport.GetHeight());
    }
}

wxDragResult DnDImageFile::OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult)
{
    if (GetData())
    {
        auto fdo = dynamic_cast<wxFileDataObject*>(GetDataObject());
        if (fdo)
        {
            auto cFiles = fdo->GetFilenames().GetCount();
            if (1 == cFiles)
            {
                wxString fileName = fdo->GetFilenames()[0];
                if (wxImage::CanRead(fileName))
                {
                    return wxFileDropTarget::OnDragOver(x, y, defResult);
                }
            }
        }
    }

    return wxDragNone;
}

wxDragResult DnDImageFile::OnEnter(wxCoord x, wxCoord y, wxDragResult defResult)
{
    return wxFileDropTarget::OnEnter(x, y, defResult);
}

bool DnDImageFile::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
{
    auto cFiles = filenames.GetCount();
    if (1==cFiles)
    {
        wxString fileName = filenames[0];
        if (wxImage::CanRead(fileName))
        {
            DropImageEvent e(spamEVT_DROP_IMAGE, ownerPanel_->GetId(), ownerPanel_, fileName);
            wxTheApp->AddPendingEvent(e);
            return true;
        }
    }

    return false;
}