#include "openprojdlg.h"
#include <wx/filepicker.h>
#include <wx/statline.h>
#include <helper/h5db.h>
#include <helper/commondef.h>

OpenProjDlg::OpenProjDlg(wxWindow* parent, const wxString &dbPath, const wxString &selProj)
    : wxDialog(parent, wxID_ANY, wxT("Project"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER|wxDEFAULT_DIALOG_STYLE)
    , dbPath_(dbPath)
    , selProj_(selProj)
{
    wxSizer *const sizer = new wxBoxSizer(wxVERTICAL);
    auto sizerFlags = wxSizerFlags().Expand().DoubleBorder(wxLEFT | wxRIGHT);

    const auto &projNames = H5DB::GetSpamProjects(dbPath_);
    int n = static_cast<int>(projNames.size());
    const wxString *choices = projNames.empty() ? nullptr: &projNames[0];
    const wxPoint& pos = wxDefaultPosition;
    const wxSize& size = wxDefaultSize;
    const wxString& wildcard = wxT("Spam DB files (*.spam_db)|*.spam_db|HDF5 files (*.h5;*.hdf5)|*.h5;*.hdf5");

    auto filePicker = new wxFilePickerCtrl(this, kSpamID_FILE_PICKER, dbPath, wxT("Hello!"), wildcard);
    auto dbPathLabelCtrl  = new wxStaticText(this, wxID_ANY, wxT("DB Path"));
    auto projLabelCtrl    = new wxStaticText(this, wxID_ANY, wxT("All Projects"));
    auto projListBox      = new wxListBox(this, kSpamID_PROJ_LIST_BOX, pos, size, n, choices);
    auto stdBtnBox        = CreateSeparatedButtonSizer(wxOK | wxCANCEL);
    auto dbPathLabelFlags = wxSizerFlags().DoubleBorder(wxLEFT | wxRIGHT | wxTOP).Left();
    auto projLabelFlags   = wxSizerFlags().DoubleBorder(wxLEFT | wxRIGHT).Left();
    auto stdBtnBoxFlags   = wxSizerFlags().Expand().Proportion(0).DoubleBorder(wxLEFT | wxRIGHT | wxBOTTOM);

    sizer->Add(dbPathLabelCtrl, dbPathLabelFlags)->SetId(kSpamOpenProjDBFileLabelCtrl);
    sizer->Add(filePicker, sizerFlags.Proportion(0))->SetId(kSpamOpenProjDBFilePickerCtrl);
    sizer->Add(projLabelCtrl, projLabelFlags)->SetId(kSpamOpenProjProjLabelCtrl);
    sizer->Add(projListBox, sizerFlags.Proportion(1))->SetId(kSpamOpenProjProjListBoxCtrl);
    sizer->Add(stdBtnBox, stdBtnBoxFlags)->SetId(kSpamOpenProjStdBtnBox);

    filePicker->Bind(wxEVT_FILEPICKER_CHANGED, &OpenProjDlg::OnDBChange, this, kSpamID_FILE_PICKER);

    auto fpSizer = filePicker->GetSizer();
    if (fpSizer)
    {
        fpSizer->SetSizeHints(filePicker);
        fpSizer->Fit(filePicker);
    }

    SetSizer(sizer);
    GetSizer()->SetSizeHints(this);
    GetSizer()->Fit(this);
}

bool OpenProjDlg::TransferDataToWindow()
{
    auto projListBox = dynamic_cast<wxListBox *>(FindWindow(kSpamID_PROJ_LIST_BOX));
    if (projListBox)
    {
        if (!projListBox->SetStringSelection(selProj_))
        {
            projListBox->SetSelection(static_cast<int>(projListBox->GetCount()) - 1);
        }
    }

    return true;
}

bool OpenProjDlg::TransferDataFromWindow()
{
    auto filePicker = dynamic_cast<const wxFilePickerCtrl *>(FindWindow(kSpamID_FILE_PICKER));
    if (filePicker)
    {
        dbPath_ = filePicker->GetPath();
    }

    auto projListBox = dynamic_cast<const wxListBox *>(FindWindow(kSpamID_PROJ_LIST_BOX));
    if (projListBox)
    {
        selProj_ = projListBox->GetStringSelection();
    }

    return true;
}

void OpenProjDlg::OnDBChange(wxFileDirPickerEvent &e)
{
    if (dbPath_!=e.GetPath())
    {
        dbPath_ = e.GetPath();

        auto projListBox = dynamic_cast<wxListBox *>(FindWindow(kSpamID_PROJ_LIST_BOX));
        if (projListBox)
        {
            projListBox->Clear();
            const auto &projNames = H5DB::GetSpamProjects(dbPath_);
            if (!projNames.empty())
            {
                projListBox->Append(projNames);
                projListBox->SetSelection(static_cast<int>(projListBox->GetCount()) - 1);
            }
        }
    }
}