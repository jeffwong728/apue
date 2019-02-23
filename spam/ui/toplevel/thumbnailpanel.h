#ifndef SPAM_UI_TOP_LEVEL_THUMBNAIL_PANEL_H
#define SPAM_UI_TOP_LEVEL_THUMBNAIL_PANEL_H
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <ui/cmndef.h>
#include <ui/misc/thumbnailctrl.h>

class ThumbnailPanel : public wxPanel
{
public:
    ThumbnailPanel(wxWindow* parent);
    ~ThumbnailPanel();

public:
    wxThumbnailCtrl *GetThumbnailCtrl() const;

private:
    wxToolBar *MakeToolBar();
    void OnThumbnailActivate(wxThumbnailEvent &e);

private:
    void OnDelete(wxCommandEvent &cmd);
    void OnSave(wxCommandEvent &cmd);
};
#endif //SPAM_UI_TOP_LEVEL_THUMBNAIL_PANEL_H