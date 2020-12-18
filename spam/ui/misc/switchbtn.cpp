#include "wx/wxprec.h"
#include "switchbtn.h"
#include "wx/gtk/private.h"
#include "wx/gtk/private/eventsdisabler.h"
#include "wx/gtk/private/list.h"

extern "C" {
static void gtk_switchbutton_clicked_callback(GtkWidget *WXUNUSED(widget), GParamSpec *WXUNUSED(pspec), wxSwitchButton *cb)
{
    // Generate a wx event.
    wxCommandEvent event(wxEVT_SWITCHBUTTON, cb->GetId());
    event.SetInt(cb->GetValue());
    event.SetEventObject(cb);
    cb->HandleWindowEvent(event);
}
}

wxDEFINE_EVENT( wxEVT_SWITCHBUTTON, wxCommandEvent );


wxIMPLEMENT_DYNAMIC_CLASS(wxSwitchButton, wxControl);

bool wxSwitchButton::Create(wxWindow *parent, wxWindowID id,
                            const wxPoint &pos,
                            const wxSize &size, long style,
                            const wxValidator& validator,
                            const wxString &name)
{
    if (!PreCreation(parent, pos, size) ||
        !CreateBase(parent, id, pos, size, style, validator, name ))
    {
        wxFAIL_MSG(wxT("wxSwitchButton creation failed"));
        return false;
    }

    m_widget = gtk_switch_new();
    g_object_ref_sink(m_widget);

    g_signal_connect (m_widget, "notify::active",
                      G_CALLBACK (gtk_switchbutton_clicked_callback),
                      this);

    m_parent->DoAddChild(this);

    PostCreation(size);

    return true;
}

void wxSwitchButton::GTKDisableEvents()
{
    g_signal_handlers_block_by_func(m_widget,
                                (gpointer)gtk_switchbutton_clicked_callback, this);
}

void wxSwitchButton::GTKEnableEvents()
{
    g_signal_handlers_unblock_by_func(m_widget,
                                (gpointer)gtk_switchbutton_clicked_callback, this);
}

void wxSwitchButton::SetValue(bool state)
{
    wxCHECK_RET(m_widget != NULL, wxT("invalid switch button"));

    if (state == GetValue())
        return;

    wxGtkEventsDisabler<wxSwitchButton> noEvents(this);

    gtk_switch_set_active(GTK_SWITCH(m_widget), state);
}

bool wxSwitchButton::GetValue() const
{
    wxCHECK_MSG(m_widget != NULL, false, wxT("invalid switch button"));

    return gtk_switch_get_active(GTK_SWITCH(m_widget)) != 0;
}

void wxSwitchButton::DoApplyWidgetStyle(GtkRcStyle *style)
{
    GTKApplyStyle(m_widget, style);
    GtkWidget* child = gtk_bin_get_child(GTK_BIN(m_widget));
    GTKApplyStyle(child, style);

#ifndef __WXGTK4__
    wxGCC_WARNING_SUPPRESS(deprecated-declarations)
    // for buttons with images, the path to the label is (at least in 2.12)
    // GtkButton -> GtkAlignment -> GtkHBox -> GtkLabel
    if ( GTK_IS_ALIGNMENT(child) )
    {
        GtkWidget* box = gtk_bin_get_child(GTK_BIN(child));
        if ( GTK_IS_BOX(box) )
        {
            wxGtkList list(gtk_container_get_children(GTK_CONTAINER(box)));
            for (GList* item = list; item; item = item->next)
            {
                GTKApplyStyle(GTK_WIDGET(item->data), style);
            }
        }
    }
    wxGCC_WARNING_RESTORE(0)
#endif
}
