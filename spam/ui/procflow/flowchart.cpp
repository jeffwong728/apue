#include "flowchart.h"
#include "stepbase.h"
#include <wx/artprov.h>
#include <wx/display.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <wx/dnd.h>
#include <wx/log.h>
#include <algorithm>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 )
#include <2geom/2geom.h>
#pragma warning( pop )

#include "initstep.h"
#include "endstep.h"
#include "cvtstep.h"
#include "threshstep.h"

class DnDText : public wxTextDropTarget
{
public:
    DnDText(FlowChart *pOwner) { m_pOwner = pOwner; }

    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& text) wxOVERRIDE;

private:
    FlowChart *m_pOwner;
};

bool DnDText::OnDropText(wxCoord x, wxCoord y, const wxString& text)
{
    m_pOwner->AppendStep(x, y, text);
    return true;
}

struct FlowChartState
{
    FlowChartState(FlowChart *const chartCtrl) : flowChart(chartCtrl) {}
    virtual ~FlowChartState() {}
    virtual void OnLeftMouseDown(wxMouseEvent &e) {}
    virtual void OnMouseMove(wxMouseEvent &e) {}
    virtual void OnLeftMouseUp(wxMouseEvent &e) {}
    virtual void Enter() {}
    virtual void Exit() {}
    FlowChart *const flowChart;
};

struct FlowChartContext
{
    FlowChartContext(FlowChart *const chartCtrl) : flowChart(chartCtrl) {}
    virtual ~FlowChartContext() {}
    FlowChart *const flowChart;
};

struct FreeStateContext : public FlowChartContext
{
    FreeStateContext(FlowChart *const chartCtrl) : FlowChartContext(chartCtrl) {}

    void StartDraging(wxMouseEvent &e)
    {
        anchorPos = e.GetPosition();
        lastPos = e.GetPosition();
    }

    void StartEditing(wxMouseEvent &e)
    {
        anchorPos = e.GetPosition();
        lastPos = e.GetPosition();
    }

    void Draging(wxMouseEvent &e)
    {
        const wxPoint thisPos = e.GetPosition();
        const wxPoint oldMinPos(std::min(anchorPos.x, lastPos.x), std::min(anchorPos.y, lastPos.y));
        const wxPoint oldMaxPos(std::max(anchorPos.x, lastPos.x), std::max(anchorPos.y, lastPos.y));
        const wxPoint newMinPos(std::min(anchorPos.x, thisPos.x), std::min(anchorPos.y, thisPos.y));
        const wxPoint newMaxPos(std::max(anchorPos.x, thisPos.x), std::max(anchorPos.y, thisPos.y));
        const wxRect  oldRect(oldMinPos, oldMaxPos);
        const wxRect  newRect(newMinPos, newMaxPos);
        flowChart->DrawRubberBand(oldRect, newRect);
        lastPos = thisPos;
    }

    void Editing(wxMouseEvent &e)
    {
        const wxPoint thisPos = e.GetPosition();
        if (currSelected)
        {
            currSelected->SetSelected();
        }
        flowChart->DoEditing(currSelected, anchorPos, lastPos, thisPos);
        lastPos = thisPos;
    }

    void EndDraging(wxMouseEvent &e)
    {
        const wxPoint thisPos = e.GetPosition();
        const wxPoint oldMinPos(std::min(anchorPos.x, lastPos.x), std::min(anchorPos.y, lastPos.y));
        const wxPoint oldMaxPos(std::max(anchorPos.x, lastPos.x), std::max(anchorPos.y, lastPos.y));
        const wxRect  oldRect(oldMinPos, oldMaxPos);
        flowChart->DrawRubberBand(oldRect, wxRect());

        if (oldRect.GetWidth() < 2 && oldRect.GetHeight() < 2)
        {
            if (wxMOD_CONTROL == e.GetModifiers())
            {
                flowChart->TogglePointSelect(e.GetPosition());
            }
            else
            {
                flowChart->ExclusivePointSelect(e.GetPosition());
            }
        }
        else
        {
            if (wxMOD_CONTROL == e.GetModifiers())
            {
                flowChart->ToggleBoxSelect(oldRect);
            }
            else
            {
                flowChart->ExclusiveBoxSelect(oldRect);
            }
        }

        lastPos = wxPoint();
        anchorPos = wxPoint();
        currSelected.reset();
    }

    void EndEditing(wxMouseEvent &e)
    {
        const wxPoint oldMinPos(std::min(anchorPos.x, lastPos.x), std::min(anchorPos.y, lastPos.y));
        const wxPoint oldMaxPos(std::max(anchorPos.x, lastPos.x), std::max(anchorPos.y, lastPos.y));
        const wxRect  oldRect(oldMinPos, oldMaxPos);

        if (oldRect.GetWidth() < 2 && oldRect.GetHeight() < 2)
        {
            if (wxMOD_CONTROL == e.GetModifiers())
            {
                flowChart->TogglePointSelect(e.GetPosition());
            }
            else
            {
                flowChart->ExclusivePointSelect(e.GetPosition());
            }
        }

        lastPos = wxPoint();
        anchorPos = wxPoint();
        currSelected.reset();
    }

    wxPoint anchorPos;
    wxPoint lastPos;
    SPStepBase currSelected;
};

struct FreeIdleState : public FlowChartState
{
    FreeIdleState(FlowChart *const chartCtrl, FreeStateContext *const ctx) : FlowChartState(chartCtrl), context(ctx)
    {
    }

    void Enter() wxOVERRIDE
    {
        context->currSelected.reset();
        wxLogMessage(wxT("FreeIdleState Enter."));
    }

    void Exit() wxOVERRIDE { wxLogMessage(wxT("FreeIdleState Quit.")); }

    void OnLeftMouseDown(wxMouseEvent &e) wxOVERRIDE
    {
        context->currSelected.reset();
        if (wxMOD_CONTROL != e.GetModifiers())
        {
            context->currSelected = flowChart->XORPointSelect(e.GetPosition());
        }
        else
        {
            context->currSelected = flowChart->GetSelect(e.GetPosition());
        }

        if (context->currSelected)
        {
            flowChart->SwitchState(FlowChart::kStateFreeEditing);
            context->StartEditing(e);
        }
        else
        {
            flowChart->SwitchState(FlowChart::kStateFreeDraging);
            context->StartDraging(e);
        }
    }

    void OnMouseMove(wxMouseEvent &e)
    {
        flowChart->HighlightTest(e.GetPosition());
    }

    FreeStateContext *const context;
};

struct FreeDragingState : public FlowChartState
{
    FreeDragingState(FlowChart *const chartCtrl, FreeStateContext *const ctx) : FlowChartState(chartCtrl), context(ctx)
    {}

    void Enter() wxOVERRIDE { wxLogMessage(wxT("FreeDragingState Enter.")); }
    void Exit() wxOVERRIDE { wxLogMessage(wxT("FreeDragingState Quit.")); }

    void OnMouseMove(wxMouseEvent &e) wxOVERRIDE
    {
        context->Draging(e);
    }

    void OnLeftMouseUp(wxMouseEvent &e) wxOVERRIDE
    {
        context->EndDraging(e);
        flowChart->SwitchState(FlowChart::kStateFreeIdle);
    }

    FreeStateContext *const context;
};

struct FreeEditingState : public FlowChartState
{
    FreeEditingState(FlowChart *const chartCtrl, FreeStateContext *const ctx) : FlowChartState(chartCtrl), context(ctx)
    {}

    void Enter() wxOVERRIDE { wxLogMessage(wxT("FreeEditingState Enter.")); }
    void Exit() wxOVERRIDE { wxLogMessage(wxT("FreeEditingState Quit.")); }

    void OnMouseMove(wxMouseEvent &e) wxOVERRIDE
    {
        context->Editing(e);
    }

    void OnLeftMouseUp(wxMouseEvent &e) wxOVERRIDE
    {
        context->EndEditing(e);
        flowChart->SwitchState(FlowChart::kStateFreeIdle);
    }

    FreeStateContext *const context;
};

struct ConnectStateContext : public FlowChartContext
{
    ConnectStateContext(FlowChart *const chartCtrl) : FlowChartContext(chartCtrl) {}
    wxPoint anchorPos;
    wxPoint lastPos;
    StepBase::Port srcPort;
};

struct ConnectIdleState : public FlowChartState
{
    ConnectIdleState(FlowChart *const chartCtrl, ConnectStateContext *const ctx) : FlowChartState(chartCtrl), context(ctx)
    {
    }

    void Enter() wxOVERRIDE
    {
        context->srcPort.step.reset();
        context->srcPort.index = -1;
    }

    void OnLeftMouseDown(wxMouseEvent &e) wxOVERRIDE
    {
        context->srcPort.step.reset();
        context->srcPort.index = -1;
    }

    void OnMouseMove(wxMouseEvent &e)
    {
        flowChart->PortHighlightTest(e.GetPosition(), false);
    }

    ConnectStateContext *const context;
};

struct ConnectConnectingState : public FlowChartState
{
    ConnectConnectingState(FlowChart *const chartCtrl, ConnectStateContext *const ctx) : FlowChartState(chartCtrl), context(ctx)
    {}

    void OnMouseMove(wxMouseEvent &e) wxOVERRIDE
    {
    }

    void OnLeftMouseUp(wxMouseEvent &e) wxOVERRIDE
    {
    }

    void OnLeftMouseDown(wxMouseEvent &e) wxOVERRIDE
    {
    }

    ConnectStateContext *const context;
};

FlowChart::FlowChart(wxWindow* parent)
{
    wxWindow::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    InheritAttributes();

    Bind(wxEVT_ENTER_WINDOW, &FlowChart::OnEnterWindow,     this, wxID_ANY);
    Bind(wxEVT_LEAVE_WINDOW, &FlowChart::OnLeaveWindow,     this, wxID_ANY);
    Bind(wxEVT_LEFT_DOWN,    &FlowChart::OnLeftMouseDown,   this, wxID_ANY);
    Bind(wxEVT_LEFT_UP,      &FlowChart::OnLeftMouseUp,     this, wxID_ANY);
    Bind(wxEVT_RIGHT_DOWN,   &FlowChart::OnRightMouseDown,  this, wxID_ANY);
    Bind(wxEVT_RIGHT_UP,     &FlowChart::OnRightMouseUp,    this, wxID_ANY);
    Bind(wxEVT_MOTION,       &FlowChart::OnMouseMotion,     this, wxID_ANY);
    Bind(wxEVT_MOUSEWHEEL,   &FlowChart::OnMouseWheel,      this, wxID_ANY);
    Bind(wxEVT_PAINT,        &FlowChart::OnPaint,           this, wxID_ANY);

    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create());
    if (gc)
    {
        wxDouble width{0};
        wxDouble height{0};
        gc->SetFont(*wxNORMAL_FONT, *wxCYAN);
        gc->GetTextExtent(wxT("123"), &width, &height);
        gapX_ = wxRound(height);
        gapY_ = wxRound(height);
    }

    allStates_.resize(kStateGuard);
    allContexts_.resize(kStateContextGuard);
    allContexts_[kStateContextFree]     = std::make_unique<FreeStateContext>(this);
    allContexts_[kStateContextConnect]  = std::make_unique<ConnectStateContext>(this);
    allStates_[kStateFreeIdle]      = std::make_unique<FreeIdleState>(this, dynamic_cast<FreeStateContext*>(allContexts_[kStateContextFree].get()));
    allStates_[kStateFreeDraging]   = std::make_unique<FreeDragingState>(this, dynamic_cast<FreeStateContext*>(allContexts_[kStateContextFree].get()));
    allStates_[kStateFreeEditing]   = std::make_unique<FreeEditingState>(this, dynamic_cast<FreeStateContext*>(allContexts_[kStateContextFree].get()));
    allStates_[kStateConnectIdle] = std::make_unique<ConnectIdleState>(this, dynamic_cast<ConnectStateContext*>(allContexts_[kStateContextConnect].get()));
    allStates_[kStateConnectConnecting] = std::make_unique<ConnectConnectingState>(this, dynamic_cast<ConnectStateContext*>(allContexts_[kStateContextConnect].get()));
    currentState_ = allStates_[kStateFreeIdle].get();
    currentState_->Enter();

    SetDropTarget(new DnDText(this));
}

void FlowChart::AppendStep(wxCoord x, wxCoord y, const wxString& stepType)
{
    bool needRefresh = true;
    if (wxT("initstep") == stepType)
    {
        steps_.push_back(std::make_shared<InitStep>());
    }
    else if (wxT("endstep") == stepType)
    {
        steps_.push_back(std::make_shared<EndStep>());
    }
    else if (wxT("cvtstep") == stepType)
    {
        steps_.push_back(std::make_shared<CvtStep>());
    }
    else if (wxT("threshstep") == stepType)
    {
        steps_.push_back(std::make_shared<ThreshStep>());
    }
    else
    {
        needRefresh = false;
    }

    if (needRefresh)
    {
        wxAffineMatrix2D invMat = affMat_; invMat.Invert();
        wxRect bbox = steps_.back()->GetBoundingBox();
        const wxPoint2DDouble srcPoint(x - bbox.GetWidth()/2, y - bbox.GetHeight()/2);
        const wxPoint2DDouble dstPoint = invMat.TransformPoint(srcPoint);
        x = wxRound(dstPoint.m_x);
        y = wxRound(dstPoint.m_y);
        steps_.back()->SetRect(wxRect(x, y, -1, -1));
        Refresh(false);
    }
}

void FlowChart::SwitchState(const int newState)
{
    if (0 <= newState && newState < kStateGuard)
    {
        currentState_->Exit();
        currentState_ = allStates_[newState].get();
        currentState_->Enter();
    }
}

void FlowChart::DrawRubberBand(const wxRect &oldRect, const wxRect &newRect)
{
    wxRect rect = oldRect.Union(newRect);

    if (rect.IsEmpty()) {
        return;
    }

    rubberBandBox_ = newRect;
    const int iTol = 1;
    wxRect irects[4];
    if (!oldRect.IsEmpty())
    {
        const int t = oldRect.GetTop();
        const int b = oldRect.GetBottom();
        const int l = oldRect.GetLeft();
        const int r = oldRect.GetRight();

        irects[0] = wxRect(wxPoint(l - iTol, t - iTol), wxPoint(r + iTol, t + iTol));
        irects[1] = wxRect(wxPoint(l - iTol, b - iTol), wxPoint(r + iTol, b + iTol));
        irects[2] = wxRect(wxPoint(l - iTol, t + iTol), wxPoint(l + iTol, b - iTol));
        irects[3] = wxRect(wxPoint(r - iTol, t + iTol), wxPoint(r + iTol, b - iTol));
    }

    if (!newRect.IsEmpty())
    {
        const int t = newRect.GetTop();
        const int b = newRect.GetBottom();
        const int l = newRect.GetLeft();
        const int r = newRect.GetRight();

        irects[0].Union(wxRect(wxPoint(l - iTol, t - iTol), wxPoint(r + iTol, t + iTol)));
        irects[1].Union(wxRect(wxPoint(l - iTol, b - iTol), wxPoint(r + iTol, b + iTol)));
        irects[2].Union(wxRect(wxPoint(l - iTol, t + iTol), wxPoint(l + iTol, b - iTol)));
        irects[3].Union(wxRect(wxPoint(r - iTol, t + iTol), wxPoint(r + iTol, b - iTol)));
    }

    for (const auto &iRect : irects)
    {
        Refresh(false, &iRect);
    }
}

void FlowChart::AlignLeft()
{
    int minLeft = std::numeric_limits<int>::max();
    for (SPStepBase &step : steps_)
    {
        if (step->IsSelected())
        {
            const int l = step->GetPositionRect().GetLeft();
            minLeft = std::min(l, minLeft);
        }
    }

    if (minLeft != std::numeric_limits<int>::max())
    {
        for (SPStepBase &step : steps_)
        {
            if (step->IsSelected())
            {
                const wxRect obbox = step->GetBoundingBox(affMat_);
                const int l = step->GetPositionRect().GetLeft();
                step->Translate(wxPoint(minLeft - l, 0));
                const wxRect nbbox = step->GetBoundingBox(affMat_).Union(obbox);
                Refresh(false, &nbbox);
            }
        }
    }
}

void FlowChart::AlignVCenter()
{
    int minXCenter = std::numeric_limits<int>::max();
    for (SPStepBase &step : steps_)
    {
        if (step->IsSelected())
        {
            const wxRect rc = step->GetPositionRect();
            minXCenter = std::min((rc.GetLeft()+rc.GetRight())/2, minXCenter);
        }
    }

    if (minXCenter != std::numeric_limits<int>::max())
    {
        for (SPStepBase &step : steps_)
        {
            if (step->IsSelected())
            {
                const wxRect obbox = step->GetBoundingBox(affMat_);
                const wxRect rc = step->GetPositionRect();
                step->Translate(wxPoint(minXCenter - (rc.GetLeft() + rc.GetRight()) / 2, 0));
                const wxRect nbbox = step->GetBoundingBox(affMat_).Union(obbox);
                Refresh(false, &nbbox);
            }
        }
    }
}

SPStepBase FlowChart::GetSelect(const wxPoint &pos)
{
    SPStepBase newSelected;
    for (SPStepBase &step : steps_)
    {
        const wxRect bbox = step->GetBoundingBox(affMat_);
        if (bbox.Contains(pos))
        {
            newSelected = step;
            break;
        }
    }
    return newSelected;
}

bool FlowChart::AccumulatePointSelect(const wxPoint &pos)
{
    for (SPStepBase &step : steps_)
    {
        const wxRect bbox = step->GetBoundingBox(affMat_);
        if (bbox.Contains(pos))
        {
            if (!step->IsSelected())
            {
                step->SetSelected();
                const wxRect bbox = step->GetBoundingBox(affMat_);
                Refresh(false, &bbox);
            }

            return true;
        }
    }

    return false;
}

SPStepBase FlowChart::XORPointSelect(const wxPoint &pos)
{
    SPStepBase newSelected;
    for (SPStepBase &step : steps_)
    {
        const wxRect bbox = step->GetBoundingBox(affMat_);
        if (bbox.Contains(pos))
        {
            newSelected = step;
            break;
        }
    }

    if (!newSelected || (newSelected && newSelected->IsSelected()))
    {
        return newSelected;
    }

    for (SPStepBase &step : steps_)
    {
        if (step->IsSelected() && step != newSelected)
        {
            step->ClearSelected();
            const wxRect bbox = step->GetBoundingBox(affMat_);
            Refresh(false, &bbox);
        }
    }

    if (newSelected && !newSelected->IsSelected())
    {
        newSelected->SetSelected();
        const wxRect bbox = newSelected->GetBoundingBox(affMat_);
        Refresh(false, &bbox);
    }

    return newSelected;
}

void FlowChart::ExclusivePointSelect(const wxPoint &pos)
{
    SPStepBase newSelected;
    for (SPStepBase &step : steps_)
    {
        const wxRect bbox = step->GetBoundingBox(affMat_);
        if (bbox.Contains(pos))
        {
            newSelected = step;
            break;
        }
    }

    for (SPStepBase &step : steps_)
    {
        if (step->IsSelected() && step != newSelected)
        {
            step->ClearSelected();
            const wxRect bbox = step->GetBoundingBox(affMat_);
            Refresh(false, &bbox);
        }
    }

    if (newSelected && !newSelected->IsSelected())
    {
        newSelected->SetSelected();
        const wxRect bbox = newSelected->GetBoundingBox(affMat_);
        Refresh(false, &bbox);
    }
}

void FlowChart::TogglePointSelect(const wxPoint &pos)
{
    for (SPStepBase &step : steps_)
    {
        const wxRect bbox = step->GetBoundingBox(affMat_);
        if (bbox.Contains(pos))
        {
            step->ToggleSelected();
            const wxRect bbox = step->GetBoundingBox(affMat_);
            Refresh(false, &bbox);
            break;
        }
    }
}

void FlowChart::ExclusiveBoxSelect(const wxRect &rcBox)
{
    for (SPStepBase &step : steps_)
    {
        const wxRect bbox = step->GetBoundingBox(affMat_);
        const wxRect ibox = rcBox.Intersect(bbox);
        if (step->IsSelected())
        {
            if (ibox.IsEmpty())
            {
                step->ClearSelected();
                Refresh(false, &bbox);
            }
        }
        else
        {
            if (!ibox.IsEmpty())
            {
                step->SetSelected();
                Refresh(false, &bbox);
            }
        }
    }
}

void FlowChart::ToggleBoxSelect(const wxRect &rcBox)
{
    for (SPStepBase &step : steps_)
    {
        const wxRect bbox = step->GetBoundingBox(affMat_);
        const wxRect ibox = rcBox.Intersect(bbox);
        if (!ibox.IsEmpty())
        {
            step->ToggleSelected();
            Refresh(false, &bbox);
        }
    }
}

void FlowChart::HighlightTest(const wxPoint &pos)
{
    SPStepBase newHighLight;
    SPStepBase oldHighLight;
    for (SPStepBase &step : steps_)
    {
        if (step->IsHighlight())
        {
            oldHighLight = step;
            break;
        }
    }

    for (SPStepBase &step : steps_)
    {
        const wxRect bbox = step->GetBoundingBox(affMat_);
        if (bbox.Contains(pos))
        {
            newHighLight = step;
        }
    }

    if (newHighLight != oldHighLight)
    {
        if (oldHighLight)
        {
            oldHighLight->ClearHighlight();
            const wxRect bbox = oldHighLight->GetBoundingBox(affMat_);
            Refresh(false, &bbox);
        }

        if (newHighLight)
        {
            newHighLight->SetHighlight();
            const wxRect bbox = newHighLight->GetBoundingBox(affMat_);
            Refresh(false, &bbox);
        }
    }
}

void FlowChart::PortHighlightTest(const wxPoint &pos, const bool expectInPort)
{
    SPStepBase newHighLight;
    SPStepBase oldHighLight;
    int oldPortIndex = -1;
    bool oldIsInPort = false;
    int newPortIndex = -1;
    bool newIsInPort = false;

    for (SPStepBase &step : steps_)
    {
        const auto [portIndex, portHighlight, portMatch, inPort] = step->GetPortStatus();
        if (portHighlight)
        {
            oldHighLight = step;
            oldPortIndex = portIndex;
            oldIsInPort = inPort;
            break;
        }
    }

    for (SPStepBase &step : steps_)
    {
        for (int ii = 0; ii < step->GetInPortCount(); ++ii)
        {
            const wxRect bbox = step->GetInPortBoundingBox(ii, affMat_);
            if (bbox.Contains(pos))
            {
                newHighLight = step;
                newPortIndex = ii;
                newIsInPort = true;
                break;
            }
        }

        if (newHighLight) break;

        for (int ii = 0; ii < step->GetOutPortCount(); ++ii)
        {
            const wxRect bbox = step->GetOutPortBoundingBox(ii, affMat_);
            if (bbox.Contains(pos))
            {
                newHighLight = step;
                newPortIndex = ii;
                newIsInPort = false;
                break;
            }
        }

        if (newHighLight) break;
    }

    if (newHighLight != oldHighLight || newPortIndex != newPortIndex || oldIsInPort != newIsInPort)
    {
        if (oldHighLight)
        {
            oldHighLight->ClearPortStatus();
            const wxRect bbox = oldHighLight->GetBoundingBox(affMat_);
            Refresh(false, &bbox);
        }

        if (newHighLight)
        {
            newHighLight->SetPortStatus(newPortIndex, newIsInPort, expectInPort == newIsInPort);
            const wxRect bbox = newHighLight->GetBoundingBox(affMat_);
            Refresh(false, &bbox);
        }
    }
}

void FlowChart::ClearStatus()
{
    for (SPStepBase &step : steps_)
    {
        if (step->IsSelected() || step->IsHighlight()  || step->IsConnectionMarks())
        {
            step->ClearSelected();
            step->ClearHighlight();
            step->ClearConnectionMarks();
            const wxRect bbox = step->GetBoundingBox(affMat_);
            Refresh(false, &bbox);
        }
    }
}

void FlowChart::SetConnectionMarks()
{
    for (SPStepBase &step : steps_)
    {
        step->SetConnectionMarks();
        const wxRect bbox = step->GetBoundingBox(affMat_);
        Refresh(false, &bbox);
    }
}

void FlowChart::DoEditing(SPStepBase &cstep, const wxPoint &apos, const wxPoint &lpos, const wxPoint &cpos)
{
    wxAffineMatrix2D invMat = affMat_; invMat.Invert();
    const wxPoint2DDouble deltaPos = invMat.TransformDistance(cpos - lpos);
    const wxPoint dPt(wxRound(deltaPos.m_x), wxRound(deltaPos.m_y));
    if (cstep && !cstep->IsSelected())
    {
        const wxRect obbox = cstep->GetBoundingBox(affMat_);
        cstep->Translate(dPt);
        const wxRect bbox = cstep->GetBoundingBox(affMat_).Union(obbox);
        Refresh(false, &bbox);
    }

    for (SPStepBase &step : steps_)
    {
        if (step->IsSelected())
        {
            const wxRect obbox = step->GetBoundingBox(affMat_);
            step->Translate(dPt);
            const wxRect bbox = step->GetBoundingBox(affMat_).Union(obbox);
            Refresh(false, &bbox);
        }
    }
}

void FlowChart::OnEnterWindow(wxMouseEvent &e)
{
}

void FlowChart::OnLeaveWindow(wxMouseEvent &e)
{
}

void FlowChart::OnLeftMouseDown(wxMouseEvent &e)
{
    if (e.LeftIsDown() && e.RightIsDown())
    {
        affMat_ = wxAffineMatrix2D();
        Refresh(false);
    }

    currentState_->OnLeftMouseDown(e);
}

void FlowChart::OnRightMouseDown(wxMouseEvent &e)
{
    lastPos_ = e.GetPosition();
    if (e.LeftIsDown() && e.RightIsDown())
    {
        affMat_ = wxAffineMatrix2D();
        Refresh(false);
    }
}

void FlowChart::OnRightMouseUp(wxMouseEvent &e)
{

}

void FlowChart::OnLeftMouseUp(wxMouseEvent &e)
{
    currentState_->OnLeftMouseUp(e);
}

void FlowChart::OnMouseMotion(wxMouseEvent &e)
{
    if (e.Dragging() && e.RightIsDown())
    {
        wxAffineMatrix2D invMat = affMat_; invMat.Invert();
        const auto deltaPos = invMat.TransformDistance(e.GetPosition() - lastPos_);
        affMat_.Translate(deltaPos.m_x, deltaPos.m_y);
        Refresh(false);
    }
    lastPos_ = e.GetPosition();
    currentState_->OnMouseMove(e);
}

void FlowChart::OnMouseWheel(wxMouseEvent &e)
{
    if (wxMOD_CONTROL == e.GetModifiers())
    {
        const auto tPt = affMat_.TransformPoint(wxPoint2DDouble(e.GetPosition()));
        affMat_.Translate(tPt.m_x, tPt.m_y);
        const double s = e.GetWheelRotation() > 0 ? 1.1 : 0.9;
        affMat_.Scale(s, s);
        affMat_.Translate(-tPt.m_x, -tPt.m_y);
        Refresh(false);
    }
}

void FlowChart::OnPaint(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(this);
    PrepareDC(dc);
    wxGCDC gcdc(dc);

    dc.SetBrush(wxBrush(wxColour(55, 56, 58)));
    dc.SetPen(wxNullPen);

    wxRegionIterator upd(GetUpdateRegion());
    while (upd)
    {
        int vX = upd.GetX();
        int vY = upd.GetY();
        int vW = upd.GetW();
        int vH = upd.GetH();

        wxRect cRect{ vX , vY , vW, vH };
        dc.DrawRectangle(cRect);

        upd++;
    }

    gcdc.SetTransformMatrix(affMat_);
    for (SPStepBase &step : steps_)
    {
        step->Draw(gcdc);
    }

    wxAffineMatrix2D idenMat;
    gcdc.SetTransformMatrix(idenMat);

    for (SPStepBase &step : steps_)
    {
        step->DrawHandles(gcdc, affMat_);
        step->DrawConnectionMarks(gcdc, affMat_);
    }

    if (!rubberBandBox_.IsEmpty())
    {
        wxPen ruberPen(*wxLIGHT_GREY, 1, wxPENSTYLE_SOLID);
        gcdc.SetPen(ruberPen);
        gcdc.SetBrush(wxNullBrush);
        gcdc.DrawRectangle(rubberBandBox_);
    }
}

void FlowChart::DrawBackground() const
{
    wxGCDC dc;
    wxColour backgroundColour = GetBackgroundColour();
    dc.SetBrush(wxBrush(backgroundColour));
    dc.SetPen(wxNullPen);
    wxRect cRect = GetClientRect();
    dc.DrawRectangle(cRect);
}
