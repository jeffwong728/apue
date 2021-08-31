#ifndef _WX_GTK_GL_AREA_WIDGET_H_
#define _WX_GTK_GL_AREA_WIDGET_H_

#include <wx/defs.h>
#include <wx/event.h>
#include <wx/control.h>
#include <gtk/gtk.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ui/graphics/glprog.h>
#include <memory>
#include <vtkSmartPointer.h>
class ExternalVTKWidget;
class vtkExternalOpenGLCamera;
class vtkOpenGLRenderer;
class vtkRenderer;
typedef unsigned int GLuint;

class wxGLAreaWidget: public wxControl
{
    enum {
        X_AXIS,
        Y_AXIS,
        Z_AXIS,

        N_AXIS
    };

public:
    // construction/destruction
    wxGLAreaWidget() {}
    wxGLAreaWidget(wxWindow *parent,
                   wxWindowID id,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxValidator& validator = wxDefaultValidator,
                   const wxString& name = wxString("wxGLAreaWidget"))
    {
        Create(parent, id, pos, size, style, validator, name);
    }

    // Create the control
    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize, long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxString("wxGLAreaWidget"));

    // Get/set the value
    void SetAxisAngleValue(const int axisIndex, const int angleVal);
    bool GetValue() const;

    void GTKDisableEvents();
    void GTKEnableEvents();
    void MoveWindow(int x, int y, int width, int height) { DoMoveWindow(x, y, width, height); }

public:
    void ImportSTL(const std::string &inputFilename);

public:
    void OnSize(wxSizeEvent &e);
    void OnLeftMouseDown(wxMouseEvent &e);
    void OnLeftMouseUp(wxMouseEvent &e);
    void OnRightMouseDown(wxMouseEvent &e);
    void OnRightMouseUp(wxMouseEvent &e);
    void OnMiddleDown(wxMouseEvent &e);
    void OnMiddleUp(wxMouseEvent &e);
    void OnMouseMotion(wxMouseEvent &e);
    void OnMouseWheel(wxMouseEvent &e);
    void OnSetFocus(wxFocusEvent &e);
    void OnKillFocus(wxFocusEvent &e);

protected:
    virtual void DoApplyWidgetStyle(GtkRcStyle *style) wxOVERRIDE;
    virtual wxSize DoGetBestSize() const wxOVERRIDE;

private:
    static void ComputeWorldToDisplay(vtkRenderer* ren, double x, double y, double z, double displayPt[3]);
    static void ComputeDisplayToWorld(vtkRenderer* ren, double x, double y, double z, double worldPt[4]);
    static void realize_cb(GtkWidget *widget, gpointer user_data);
    static void unrealize_cb(GtkWidget *widget, gpointer user_data);
    static gboolean render_cb(GtkGLArea *area, GdkGLContext *context, gpointer user_data);

private:
    typedef wxControl base_type;

private:
    wxPoint anchorPos_;
    wxPoint lastPos_;
    vtkSmartPointer<ExternalVTKWidget> externalVTKWidget;
    vtkSmartPointer<vtkOpenGLRenderer> rootRenderer;
    vtkSmartPointer<vtkOpenGLRenderer> axisRenderer;

    wxDECLARE_DYNAMIC_CLASS(wxGLAreaWidget);
};

#endif // _WX_GTK_GL_AREA_WIDGET_H_

