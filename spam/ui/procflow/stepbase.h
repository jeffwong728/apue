#ifndef SPAM_UI_PROCFLOW_STEP_BASE_H
#define SPAM_UI_PROCFLOW_STEP_BASE_H
#include <string>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
class wxGCDC;

class StepBase
{
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
    void SetRect(const wxRect &rc) { posRect_ = rc; }
    wxRect GetBoundingBox() const;

private:
    std::string uuid_;
    wxString typeName_;
    wxRect posRect_;
};
#endif //SPAM_UI_PROCFLOW_STEP_BASE_H
