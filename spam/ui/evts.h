#ifndef SPAM_UI_EVTS_H
#define SPAM_UI_EVTS_H
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <memory>
#include <vector>
#include <ui/projs/modelfwd.h>

class ModelEvent : public wxEvent
{
public:
    ModelEvent(wxEventType eventType, int winid, const wxDataViewModel *model)
        : wxEvent(winid, eventType)
        , model_(model)
    {
    }

    // accessors
    const wxDataViewModel* GetModel() const { return model_; }

    // implement the base class pure virtual
    virtual wxEvent *Clone() const { return new ModelEvent(*this); }
private:
    const wxDataViewModel *model_;
};

class DropImageEvent : public wxEvent
{
public:
    DropImageEvent(wxEventType eventType, int winid, wxWindow *dndTarget, const wxString &imageFilePath)
        : wxEvent(winid, eventType)
        , dndTarget_(dndTarget)
        , imageFilePath_(imageFilePath)
    {
    }

    // accessors
    const wxString& GetImageFilePath() const { return imageFilePath_; }
    wxWindow *GetDropTarget() const { return dndTarget_; }

    // implement the base class pure virtual
    virtual wxEvent *Clone() const { return new DropImageEvent(*this); }
private:
    wxWindow *dndTarget_;
    const wxString imageFilePath_;
};

wxDECLARE_EVENT(spamEVT_PROJECT_NEW,       ModelEvent);
wxDECLARE_EVENT(spamEVT_PROJECT_LOADED,    ModelEvent);
wxDECLARE_EVENT(spamEVT_DROP_IMAGE,        DropImageEvent);
#endif //SPAM_UI_EVTS_H