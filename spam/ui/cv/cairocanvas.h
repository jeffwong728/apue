#ifndef SPAM_UI_CV_CAIRO_CANVAS_H
#define SPAM_UI_CV_CAIRO_CANVAS_H
#include <wx/wxprec.h>
#include <wx/vscroll.h>
#include <wx/dnd.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <ui/projs/modelfwd.h>
#include <ui/misc/instructiontip.h>
#include <ui/proc/rgn.h>
#include <string>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/2geom.h>
#pragma warning( pop )
#include <boost/signals2.hpp>

#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif
#ifdef WINDING
#undef WINDING
#endif
#include <cairomm/cairomm.h>


namespace bs2 = boost::signals2;

class CairoCanvas : public wxScrolledCanvas
{
public:
    CairoCanvas(wxWindow* parent, const std::string &cvWinName, const wxString &uuidStation, const wxSize& size);
    ~CairoCanvas();

public:
    typedef bs2::keywords::mutex_type<bs2::dummy_mutex> bs2_dummy_mutex;
    bs2::signal_type<void(wxMouseEvent &),          bs2_dummy_mutex>::type sig_EnterWindow;
    bs2::signal_type<void(wxMouseEvent &),          bs2_dummy_mutex>::type sig_LeaveWindow;
    bs2::signal_type<void(wxMouseEvent &),          bs2_dummy_mutex>::type sig_LeftMouseDown;
    bs2::signal_type<void(wxMouseEvent &),          bs2_dummy_mutex>::type sig_LeftMouseUp;
    bs2::signal_type<void(wxMouseEvent &),          bs2_dummy_mutex>::type sig_MouseMotion;
    bs2::signal_type<void(wxMouseEvent &),          bs2_dummy_mutex>::type sig_LeftDClick;
    bs2::signal_type<void(wxMouseEvent &),          bs2_dummy_mutex>::type sig_MiddleDown;
    bs2::signal_type<void(wxKeyEvent &),            bs2_dummy_mutex>::type sig_KeyDown;
    bs2::signal_type<void(wxKeyEvent &),            bs2_dummy_mutex>::type sig_KeyUp;
    bs2::signal_type<void(wxKeyEvent &),            bs2_dummy_mutex>::type sig_Char;
    bs2::signal_type<void(const ImageBufferItem &), bs2_dummy_mutex>::type sig_ImageBufferItemAdd;
    bs2::signal_type<void(const ImageBufferItem &), bs2_dummy_mutex>::type sig_ImageBufferItemUpdate;

public:
    cv::Mat GetOriginalImage() const { return srcImg_; }
    const ImageBufferZone &GetImageBufferZone() const { return imgBufferZone_; }
    void ShowImage(const cv::Mat &img);
    void SwitchImage(const std::string &iName);
    void ExtentImage();
    void ScaleImage(double scaleVal);
    void ScaleUp(double val);
    void ScaleDown(double val);
    double GetMatScale() const;
    bool HasImage() const { return !srcMat_.empty() && !disMat_.empty(); }
    bool IsInImageRect(const wxPoint &pt) const;
    wxRealPoint ScreenToDispImage(const wxPoint &pt);
    wxRealPoint ScreenToImage(const wxPoint &pt) const;
    void DrawDrawables(const SPDrawableNodeVector &des);
    void EraseDrawables(const SPDrawableNodeVector &des);
    void HighlightDrawable(const SPDrawableNode &de);
    void DimDrawable(const SPDrawableNode &de);
    void DrawPathVector(const Geom::PathVector &pth, const Geom::OptRect &rect);
    void DrawBox(const Geom::Path &pth);
    void DrawBox(const Geom::OptRect &oldRect, const Geom::OptRect &newRect);
    void AddRect(const RectData &rd);
    void AddLine(const LineData &ld);
    void AddEllipse(const GenericEllipseArcData &ed);
    void AddPolygon(const PolygonData &pd);
    void AddBeziergon(const BezierData &bd);
    void DoEdit(const int toolId, const SPDrawableNodeVector &selEnts, const SpamMany &mementos);
    void DoTransform(const SPDrawableNodeVector &selEnts, const SpamMany &mementos);
    void DoNodeEdit(const SPDrawableNodeVector &selEnts, const SpamMany &mementos);
    void DoUnion(const SPDrawableNodeVector &selEnts);
    void DoIntersection(const SPDrawableNodeVector &selEnts);
    void DoXOR(const SPDrawableNode &dn1, const SPDrawableNode &dn2);
    void DoDifference(const SPDrawableNode &dn1, const SPDrawableNode &dn2);
    SPStationNode GetStation();
    SPDrawableNode FindDrawable(const Geom::Point &pt);
    SPDrawableNode FindDrawable(const Geom::Point &pt, const double sx, const double sy, SelectionData &sd);
    void SelectDrawable(const Geom::Rect &box, SPDrawableNodeVector &ents);
    std::string GetUUID() const { return stationUUID_.ToStdString(); }
    void ModifyDrawable(const SPDrawableNode &ent, const Geom::Point &pt, const SelectionData &sd, const int editMode);
    void DismissInstructionTip();
    void SetInstructionTip(std::vector<wxString> &&messages, const wxPoint &pos);
    void SetInstructionTip(const wxString &message, const std::string &icon, const wxPoint &pos);
    void StopInstructionTip();
    void ShowPixelValue(const wxPoint &pos);
    void PopupImageInfomation(const wxPoint &pos);
    void PushImageIntoBufferZone(const std::string &name);
    void PushRegionsIntoBufferZone(const std::string &name, const SPSpamRgnVector &rgns);

private:
    void OnSize(wxSizeEvent& event);
    void OnEnterWindow(wxMouseEvent &e);
    void OnLeaveWindow(wxMouseEvent &e);
    void OnLeftMouseDown(wxMouseEvent &e);
    void OnLeftMouseUp(wxMouseEvent &e);
    void OnMouseMotion(wxMouseEvent &e);
    void OnMiddleDown(wxMouseEvent &e);
    void OnLeftDClick(wxMouseEvent &e);
    void OnKeyDown(wxKeyEvent &e);
    void OnKeyUp(wxKeyEvent &e);
    void OnChar(wxKeyEvent &e);
    void OnPaint(wxPaintEvent& e);
    void OnEraseBackground(wxEraseEvent &e);
    void OnContextMenu(wxContextMenuEvent& e);
    void OnDeleteEntities(wxCommandEvent &cmd);
    void OnPushToBack(wxCommandEvent &cmd);
    void OnBringToFront(wxCommandEvent &cmd);
    void OnTipTimer(wxTimerEvent &e);
    void Draw(wxDC &dc, const Geom::OptRect &rect);
    void Draw(wxDC &dc, const Geom::Path &pth);
    void DrawBox(wxDC &dc, const Geom::Path &pth);
    void DrawBox(wxDC &dc, const Geom::OptRect &oldRect, const Geom::OptRect &newRect);
    void DrawEntities(Cairo::RefPtr<Cairo::Context> &cr);
    void DrawEntities(Cairo::RefPtr<Cairo::Context> &cr, const SPDrawableNode &highlight);
    void ConpensateHandle(wxRect &invalidRect) const;

private:
    void MoveAnchor(const wxSize &sViewport, const wxSize &disMatSize);
    void ScaleShowImage(const wxSize &sToSize);
    wxSize GetDispMatSize(const wxSize &sViewport, const wxSize &srcMatSize);
    wxSize GetFitSize(const wxSize &sViewport, const wxSize &srcMatSize);

private:
    cv::String cvWndName_;
    wxString stationUUID_;
    cv::Mat srcImg_;
    cv::Mat srcMat_;
    cv::Mat disMat_;
    cv::Mat scrMat_;
    int anchorX_;
    int anchorY_;
    int cRect_{ 0 };
    int cLine_{ 0 };
    int cEllipse_{0};
    int cPolygon_{ 0 };
    int cBeziergon_{0};
    std::unique_ptr<wxTimer> tipTimer_;
    std::unique_ptr<InstructionTip> tip_;
    std::vector<wxString> tipMessages_;
    std::string tipIcon_;
    wxPoint     tipPos_;
    ImageBufferZone imgBufferZone_;
    RgnBufferZone   rgnBufferZone_;
};

class DnDImageFile : public wxFileDropTarget
{
public:
    DnDImageFile(wxWindow *pOwner = NULL) { ownerPanel_ = pOwner; }
    wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult) wxOVERRIDE;
    wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult defResult) wxOVERRIDE;
    bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) wxOVERRIDE;

private:
    wxWindow *ownerPanel_;
};

#endif //SPAM_UI_CV_CAIRO_CANVAS_H