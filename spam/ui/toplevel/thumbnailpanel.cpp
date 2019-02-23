#include "thumbnailpanel.h"
#include <wx/artprov.h>
#include <wx/wrapsizer.h>
#include <ui/spam.h>
#include <ui/cv/cairocanvas.h>

ThumbnailPanel::ThumbnailPanel(wxWindow* parent)
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
    // Root sizer, vertical
    wxSizer * const sizerRoot = new wxBoxSizer(wxVERTICAL);
    sizerRoot->Add(MakeToolBar(), wxSizerFlags().Expand())->SetId(kStationThumbnailToolBar);

    long style = wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL | wxTH_TEXT_LABEL | wxTH_IMAGE_LABEL | wxTH_SINGLE_SELECT;
    auto thumbWindow = new wxThumbnailCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);
    sizerRoot->Add(thumbWindow, wxSizerFlags(1).Expand())->SetId(kStationThumbnailCtrl);
    thumbWindow->Bind(wxEVT_COMMAND_THUMBNAIL_LEFT_DCLICK, &ThumbnailPanel::OnThumbnailActivate, this);

    // Set sizer for the panel
    SetSizer(sizerRoot);
    GetSizer()->SetSizeHints(this);
    Show();
}

ThumbnailPanel::~ThumbnailPanel()
{
}

wxThumbnailCtrl *ThumbnailPanel::GetThumbnailCtrl() const
{
    return dynamic_cast<wxThumbnailCtrl *>(GetSizer()->GetItemById(kStationThumbnailCtrl)->GetWindow());
}

wxToolBar *ThumbnailPanel::MakeToolBar()
{
    wxToolBar *tb = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
    tb->SetToolBitmapSize(wxSize(16, 16));

    tb->AddTool(kSpamID_STATION_THUMBNAIL_DELETE, wxT("Delete"),    wxArtProvider::GetBitmap(wxART_DELETE, wxART_TOOLBAR, wxSize(16, 16)));
    tb->AddTool(kSpamID_STATION_THUMBNAIL_SAVE, wxT("Save"), wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR, wxSize(16, 16)));
    tb->Realize();

    tb->Bind(wxEVT_TOOL, &ThumbnailPanel::OnDelete, this, kSpamID_STATION_THUMBNAIL_DELETE);
    tb->Bind(wxEVT_TOOL, &ThumbnailPanel::OnSave, this, kSpamID_STATION_THUMBNAIL_SAVE);

    return tb;
}

void ThumbnailPanel::OnThumbnailActivate(wxThumbnailEvent &e)
{
    wxThumbnailCtrl *tmCtrl = GetThumbnailCtrl();
    if (tmCtrl)
    {
        auto tmItem = dynamic_cast<wxPreloadImageThumbnailItem *>(tmCtrl->GetItem(e.GetIndex()));
        if (tmItem)
        {
            CairoCanvas *cav = Spam::FindCanvas(tmItem->GetGroupUUID());
            if (cav)
            {
                cav->SwitchImage(tmItem->GetFilename().ToStdString());
            }
        }
    }
}

void ThumbnailPanel::OnDelete(wxCommandEvent &cmd)
{
}

void ThumbnailPanel::OnSave(wxCommandEvent &cmd)
{
}