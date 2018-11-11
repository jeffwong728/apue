#ifndef SPAM_UI_PROJS_STYLE_CELL_RENDERER_H
#define SPAM_UI_PROJS_STYLE_CELL_RENDERER_H
#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <wx/dataview.h>
#include "drawstyle.h"

class StyleCellRenderer: public wxDataViewCustomRenderer
{
public:
    static wxString GetDefaultType();
    explicit StyleCellRenderer(wxDataViewCellMode mode) : wxDataViewCustomRenderer(GetDefaultType(), mode, wxALIGN_CENTER) { }

public:
    bool Render( wxRect rect, wxDC *dc, int state ) wxOVERRIDE;
    bool ActivateCell(const wxRect& cell, wxDataViewModel *model, const wxDataViewItem &item, unsigned int col, const wxMouseEvent *mouseEvent) wxOVERRIDE;
    wxSize GetSize() const wxOVERRIDE;

    bool SetValue( const wxVariant &value ) wxOVERRIDE;
    bool GetValue( wxVariant &value ) const wxOVERRIDE;

#if wxUSE_ACCESSIBILITY
    wxString GetAccessibleDescription() const wxOVERRIDE { return wxEmptyString; }
#endif // wxUSE_ACCESSIBILITY

    bool HasEditorCtrl() const wxOVERRIDE { return false; }
    wxWindow* CreateEditorCtrl(wxWindow* parent, wxRect labelRect, const wxVariant& value) wxOVERRIDE { return nullptr; };
    bool GetValueFromEditorCtrl(wxWindow* ctrl, wxVariant& value) wxOVERRIDE { return false; }

private:
    DrawStyle drawStyle_;
};
#endif //SPAM_UI_PROJS_STYLE_CELL_RENDERER_H