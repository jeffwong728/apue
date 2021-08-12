#ifndef SPAM_UI_PROCFLOW_STEP_BASE_H
#define SPAM_UI_PROCFLOW_STEP_BASE_H
#include <string>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "stepfwd.h"
class wxGCDC;

class StepBase
{
public:
    enum StepDisplayStatusFlag : uint64_t
    {
        kSDSF_SELECTED          = 0x1,
        kSDSF_HIGHLIGHT         = 0x2,
        kSDSF_CONNECTION_MARKS  = 0x4,
        kSDSF_PORT_HIGHLIGHT    = 0x0100000000000000,
        kSDSF_PORT_MATCH        = 0x0200000000000000,
        kSDSF_PORT_INOUT        = 0x0400000000000000,
        kSDSF_PORT_INDEX        = 0xF800000000000000,
        kSDSF_ALL_FEATURES      = 0xFFFFFFFFFFFFFFFF
    };

    enum StepPortType : int
    {
        kSPT_NULL,
        kSPT_MAT,
        kSPT_REGION,
        kSPT_AFF_MAT,
        kSPT_ANY,
        KSPT_GUARD
    };

    struct Port
    {
        WPStepBase step;
        int index = -1;
    };

protected:
    StepBase(wxString &&typeName);
    StepBase(const wxString &typeName);

public:
    StepBase(const StepBase&) = delete;
    StepBase(StepBase&&) = delete;
    StepBase &operator=(const StepBase&) = delete;
    StepBase &operator=(StepBase&&) = delete;
    virtual ~StepBase() {}

public:
    virtual void Draw(wxGCDC &dc) const;

public:
    void DrawHandles(wxGCDC &dc, const wxAffineMatrix2D &affMat) const;
    void DrawConnectionMarks(wxGCDC &dc, const wxAffineMatrix2D &affMat) const;
    void SetRect(const wxRect &rc);
    const wxRect GetBoundingBox() const;
    const wxRect GetBoundingBox(const wxAffineMatrix2D &affMat) const;
    const wxRect GetInPortBoundingBox(const int portIndex, const wxAffineMatrix2D &affMat) const;
    const wxRect GetOutPortBoundingBox(const int portIndex, const wxAffineMatrix2D &affMat) const;
    void SetSelected() { statusFlags_ |= kSDSF_SELECTED; }
    void ClearSelected() { statusFlags_ &= ~kSDSF_SELECTED; }
    void ToggleSelected() { statusFlags_ ^= kSDSF_SELECTED; }
    bool IsSelected() const { return statusFlags_ & kSDSF_SELECTED; }
    void SetHighlight() { statusFlags_ |= kSDSF_HIGHLIGHT; }
    void ClearHighlight() { statusFlags_ &= ~kSDSF_HIGHLIGHT; }
    void ToggleHighlight() { statusFlags_ ^= kSDSF_HIGHLIGHT; }
    bool IsHighlight() const { return statusFlags_ & kSDSF_HIGHLIGHT; }
    void Translate(const wxPoint &dxy) { posRect_.Offset(dxy); }
    const wxRect GetPositionRect() const { return posRect_; }
    void SetConnectionMarks() { statusFlags_ |= kSDSF_CONNECTION_MARKS; }
    void ClearConnectionMarks() { statusFlags_ &= ~kSDSF_CONNECTION_MARKS; }
    bool IsConnectionMarks() const { return statusFlags_ & kSDSF_CONNECTION_MARKS; }
    void SetPortStatus(const int portIndex, const bool inPort, const bool matching);
    void ClearPortStatus();
    void TogglePortStatus();
    const std::tuple<int, bool, bool, bool> GetPortStatus() const;

public:
    virtual const int GetInPortCount() const = 0;
    virtual const int GetOutPortCount() const = 0;
    virtual const int GetInPortDegree(const int portIndex) const = 0;
    virtual const int GetOutPortDegree(const int portIndex) const = 0;
    virtual const int GetInPortType(const int portIndex) const = 0;
    virtual const int GetOutPortType(const int portIndex) const = 0;

private:
    virtual void DrawInternal(wxGCDC &dc) const;

protected:
    std::string uuid_;
    wxString typeName_;
    wxRect posRect_;
    wxSize htSize_;
    uint64_t statusFlags_{0};
};
#endif //SPAM_UI_PROCFLOW_STEP_BASE_H
