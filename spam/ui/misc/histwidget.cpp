#include "histwidget.h"
#include <wx/artprov.h>
#include <wx/display.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <algorithm>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/2geom.h>
#pragma warning( pop )

HistogramWidget::HistogramWidget(wxWindow* parent)
    : rangeX_(std::make_pair<int, int>(0, 0))
{
    wxControl::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    InheritAttributes();

    Bind(wxEVT_ENTER_WINDOW, &HistogramWidget::OnEnterWindow,   this, wxID_ANY);
    Bind(wxEVT_LEAVE_WINDOW, &HistogramWidget::OnLeaveWindow,   this, wxID_ANY);
    Bind(wxEVT_LEFT_DOWN,    &HistogramWidget::OnLeftMouseDown, this, wxID_ANY);
    Bind(wxEVT_LEFT_UP,      &HistogramWidget::OnLeftMouseUp,   this, wxID_ANY);
    Bind(wxEVT_MOTION,       &HistogramWidget::OnMouseMotion,   this, wxID_ANY);
    Bind(wxEVT_PAINT,        &HistogramWidget::OnPaint,         this, wxID_ANY);

    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create());
    if (gc)
    {
        wxDouble width{0};
        wxDouble height{0};
        gc->SetFont(*wxNORMAL_FONT, wxColor());
        gc->GetTextExtent(wxT("123"), &width, &height);
        gapX_ = wxRound(height);
        gapY_ = wxRound(height);
    }
}

void HistogramWidget::ClearProfiles()
{ 
    profiles_.clear();
}

void HistogramWidget::SetRangeX(const std::pair<int, int> &r) 
{ 
    rangeX_ = r;
}

void HistogramWidget::OnEnterWindow(wxMouseEvent &e)
{
    cursorVisible_ = true;
}

void HistogramWidget::OnLeaveWindow(wxMouseEvent &e)
{
    cursorVisible_ = false;
    Refresh();
}

void HistogramWidget::OnLeftMouseDown(wxMouseEvent &e)
{
    dragingThumb_ = -1;
    wxRect cRect  = GetClientRect();
    int width     = cRect.GetWidth() - 2 * gapX_;
    int height    = cRect.GetHeight() - 2 * gapY_;

    for (int t=0; t<static_cast<int>(thumbs_.size()); ++t)
    {
        double x = (thumbs_[t] + 0.0)*width / (rangeX_.second- rangeX_.first) + gapX_;
        wxRect2DDouble rect(x-2, gapY_, 5, height);
        if (rect.Contains(wxPoint2DDouble(e.GetPosition())))
        {
            dragingThumb_ = t;
            break;
        }
    }

    if (dragingThumb_ >= 0)
    {
        if (!HasCapture())
        {
            CaptureMouse();
        }
    }
}

void HistogramWidget::OnLeftMouseUp(wxMouseEvent &e)
{
    if (dragingThumb_ >= 0)
    {
        if (HasCapture())
        {
            ReleaseMouse();
        }
    }

    dragingThumb_ = -1;
    Refresh();
}

void HistogramWidget::OnMouseMotion(wxMouseEvent &e)
{
    wxPoint mPos = e.GetPosition();
    wxRect cRect = GetClientRect();
    int width = cRect.GetWidth() - 2 * gapX_;
    int height = cRect.GetHeight() - 2 * gapY_;

    highlightThumb_ = -1;
    for (int t = 0; t<static_cast<int>(thumbs_.size()); ++t)
    {
        double x = (thumbs_[t] + 0.0)*width / (rangeX_.second - rangeX_.first) + gapX_;
        wxRect2DDouble rect(x - 2, gapY_, 5, height);
        if (rect.Contains(wxPoint2DDouble(e.GetPosition())))
        {
            highlightThumb_ = t;
            break;
        }
    }

    int numThumbs = static_cast<int>(thumbs_.size());
    if (dragingThumb_ >= 0 && dragingThumb_<numThumbs)
    {
        int t = std::min(wxRound((mPos.x - gapX_ + 0.0)*(rangeX_.second - rangeX_.first) / width), rangeX_.second);
        t = std::max(t, rangeX_.first);
        if (numThumbs>1)
        {
            if (dragingThumb_ < numThumbs - 1)
            {
                t = std::min(t, thumbs_[dragingThumb_ + 1]);
            }
            
            if (dragingThumb_>0)
            {
                t = std::max(t, thumbs_[dragingThumb_ - 1]);
            }
        }

        thumbs_[dragingThumb_] = t;
        sig_ThumbsMoved(this);
    }

    wxClientDC cdc(this);
    wxBufferedDC bdc(&cdc);

    wxGCDC gcdc(bdc);
    DrawHistogram(gcdc, &mPos);
}

void HistogramWidget::OnPaint(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(this);
    PrepareDC(dc);
    wxGCDC gcdc(dc);
    DrawHistogram(gcdc);
}

void HistogramWidget::SmoothProfile(std::vector<wxPoint> &pts) const
{
    if (!pts.empty())
    {
        int x = pts.front().x;
        int y = 0;

        std::vector<wxPoint> npts;
        for (const wxPoint &pt : pts)
        {
            if (pt.x != x)
            {
                npts.push_back(wxPoint(x, y));
                x = pt.x;
                y = pt.y;
            }
            else
            {
                y += pt.y;
            }
        }
        npts.push_back(wxPoint(x, y));
        pts.swap(npts);
    }
}

void HistogramWidget::DrawBackground(wxGCDC &dc) const
{
    wxColour backgroundColour = GetBackgroundColour();
    dc.SetBrush(wxBrush(backgroundColour));
    dc.SetPen(wxNullPen);
    wxRect cRect = GetClientRect();
    dc.DrawRectangle(cRect);
}

void HistogramWidget::DrawHistogram(wxGCDC &dc, const wxPoint *point) const
{
    DrawBackground(dc);

    wxRect cRect = GetClientRect();
    int width  = cRect.GetWidth() - 2* gapX_;
    int height = cRect.GetHeight() - 2* gapY_;

    dc.SetBrush(wxNullBrush);
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawRectangle(wxRect(gapX_-1, gapY_, width+2, height));

    bool noProfile = profiles_.empty();
    if (profiles_.empty())
    {
        Profile prof{ wxT(""), *wxBLACK, std::vector<double>(std::max(0, rangeX_.second - rangeX_.first + 1), 0.0) };
        profiles_.push_back(prof);
    }

    std::vector<CursorData> curDatas;
    wxPoint mPos = point ? *point + wxPoint(-gapX_, 0) : wxPoint();
    for (int p=0; p<static_cast<int>(profiles_.size()); ++p)
    {
        const Profile &profile = profiles_[p];
        if (plane_==p && profile.seq.size()>1)
        {
            auto mmIt = std::minmax_element(profile.seq.cbegin(), profile.seq.cend());
            double minVal = *mmIt.first;
            double maxVal = *mmIt.second;
            double yInter = maxVal - minVal;

            CursorData curd = { 0, 0, wxPoint(),  profile.color };
            curd.pos.x = point ? point->x : 0;
            double minDist = std::numeric_limits<double>::max();

            std::vector<wxPoint> points;
            if (yInter>0.1)
            {
                int px = -1;
                double py = 0;
                for (int n = 0; n<profile.seq.size(); ++n)
                {
                    int x = wxRound(n / (profile.seq.size() - 1.0) * width);
                    double y = (profile.seq[n] - minVal) / yInter;

                    if (x != px)
                    {
                        points.push_back(wxPoint(px, wxRound((1 - py)*height)));
                        if (point)
                        {
                            double dist = std::abs(mPos.x - px);
                            if (dist < minDist)
                            {
                                minDist = dist;
                                curd.pos = points.back();
                                curd.x = n;
                                curd.y = profile.seq[n];
                            }
                        }
                        px = x;
                        py = y;
                    }
                    else
                    {
                        py = std::max(py, y);
                    }
                }
                points.push_back(wxPoint(px, wxRound((1 - py)*height)));
                if (point)
                {
                    double dist = std::abs(mPos.x - px);
                    if (dist < minDist)
                    {
                        minDist = dist;
                        curd.pos = points.back();
                        curd.x = profile.seq.size() - 1.0;
                        curd.y = profile.seq.back();
                    }
                }
            }
            else
            {
                int px = -1;
                int py = height;
                if (0==wxRound(minVal) && 0==wxRound(maxVal))
                {
                    py = 0;
                }
                for (int n = 0; n<profile.seq.size(); ++n)
                {
                    int x = wxRound(n / (profile.seq.size() - 1.0) * width);
                    if (x != px)
                    {
                        points.push_back(wxPoint(px, py));
                        if (point)
                        {
                            double dist = std::abs(mPos.x - px);
                            if (dist < minDist)
                            {
                                minDist = dist;
                                curd.pos = points.back();
                                curd.x = n;
                                curd.y = profile.seq[n];
                            }
                        }
                        px = x;
                    }
                }
                points.push_back(wxPoint(px, py));
                if (point)
                {
                    double dist = std::abs(mPos.x - px);
                    if (dist < minDist)
                    {
                        minDist = dist;
                        curd.pos = points.back();
                        curd.x = profile.seq.size() - 1.0;
                        curd.y = profile.seq.back();
                    }
                }
            }

            if (points.size()>1)
            {
                if (point) curDatas.push_back(curd);
                dc.SetPen(wxPen(profile.color));
                dc.DrawLines(static_cast<int>(points.size()), points.data(), gapX_, gapY_);
            }
        }
    }

    DrawThumbs(dc);
    if (dragingThumb_<0 && highlightThumb_<0)
    {
        DrawCursors(dc, curDatas);
    }

    if (noProfile)
    {
        profiles_.clear();
    }
}

void HistogramWidget::DrawCursors(wxGCDC &dc, const std::vector<CursorData> &curDatas) const
{
    wxRect cRect = GetClientRect();
    int width = cRect.GetWidth() - 2 * gapX_;
    int height = cRect.GetHeight() - 2 * gapY_;

    for (const CursorData &cd : curDatas)
    {
        dc.SetBrush(wxNullBrush);
        dc.SetPen(wxPen(cd.color));
        dc.DrawCircle(cd.pos + wxPoint(gapX_, gapY_), 3);
    }

    if (!curDatas.empty())
    {
        const CursorData &cd = curDatas.front();
        dc.SetBrush(wxNullBrush);
        dc.SetPen(wxPen(*wxBLACK, 1, wxPENSTYLE_LONG_DASH));
        dc.DrawLine(wxPoint(cd.pos.x + gapX_, gapY_), wxPoint(cd.pos.x + gapX_, gapY_ + height));

        wxBrush bkBrush(GetBackgroundColour());
        dc.SetBrush(bkBrush);
        wxString xStr = wxString::Format(wxT("%.0f"), cd.x);

        wxPoint xp(cd.pos.x + gapX_, gapY_ + height);

        wxCoord fwidth{ 0 };
        wxCoord fheight{ 0 };
        dc.GetTextExtent(xStr, &fwidth, &fheight);

        wxPoint xPt = xp + wxPoint(-fwidth / 2, -fheight / 2);
        wxRect xBox(xPt, wxSize(fwidth, fheight));
        xBox.Inflate(2);
        if (xBox.GetLeft() < 0)
        {
            xBox.Offset(-xBox.GetLeft(), 0);
        }
        if (xBox.GetRight() > cRect.GetWidth() - 1)
        {
            xBox.Offset(cRect.GetWidth() - 1 - xBox.GetRight(), 0);
        }
        dc.SetPen(wxPen(*wxBLACK, 1, wxPENSTYLE_SOLID));
        dc.DrawRectangle(xBox);
        dc.DrawText(xStr, xBox.Deflate(2).GetTopLeft());

        wxString yStr = wxString::Format(wxT("%.0f"), cd.y);
        wxPoint yp(cd.pos.x + gapX_, gapY_);
        dc.GetTextExtent(yStr, &fwidth, &fheight);
        wxPoint yPt = yp + wxPoint(-fwidth / 2, -fheight / 2);
        wxRect yBox(yPt, wxSize(fwidth, fheight));
        yBox.Inflate(2);
        if (yBox.GetLeft() < 0)
        {
            yBox.Offset(-yBox.GetLeft(), 0);
        }
        if (yBox.GetRight() > cRect.GetWidth() - 1)
        {
            yBox.Offset(cRect.GetWidth() - 1 - yBox.GetRight(), 0);
        }
        dc.DrawRectangle(yBox);
        dc.SetTextForeground(curDatas.front().color);
        dc.DrawText(yStr, yBox.Deflate(2).GetTopLeft());
    }
}

void HistogramWidget::DrawThumbs(wxGCDC &dc) const
{
    wxRect cRect = GetClientRect();
    int width = cRect.GetWidth() - 2 * gapX_;
    int height = cRect.GetHeight() - 2 * gapY_;

    for (int t = 0; t<static_cast<int>(thumbs_.size()); ++t)
    {
        int x = wxRound((thumbs_[t] + 0.0)*width / (rangeX_.second - rangeX_.first) + gapX_);
        wxPoint pt1{ x, gapY_ };
        wxPoint pt2{ x, gapY_ + height };

        if (highlightThumb_ == t || dragingThumb_ == t)
        {
            dc.SetPen(wxPen(*wxRED, 2, wxPENSTYLE_SOLID));
            dc.DrawLine(pt1, pt2);
        }
        else
        {
            dc.SetPen(*wxBLACK_PEN);
            dc.DrawLine(pt1, pt2);
        }
    }
}