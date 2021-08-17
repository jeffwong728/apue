#ifndef _WX_GTK_GL_AREA_WIDGET_H_
#define _WX_GTK_GL_AREA_WIDGET_H_

#include <wx/defs.h>
#include <wx/event.h>
#include <wx/control.h>
#include <gtk/gtk.h>
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
    void OnSize(wxSizeEvent &e);
    void OnLeftMouseDown(wxMouseEvent &e);
    void OnLeftMouseUp(wxMouseEvent &e);
    void OnRightMouseDown(wxMouseEvent &e);
    void OnRightMouseUp(wxMouseEvent &e);
    void OnMiddleDown(wxMouseEvent &e);
    void OnMiddleUp(wxMouseEvent &e);
    void OnMouseMotion(wxMouseEvent &e);

protected:
    virtual void DoApplyWidgetStyle(GtkRcStyle *style) wxOVERRIDE;
    virtual wxSize DoGetBestSize() const wxOVERRIDE;

private:
    GLuint LoadTexture();
    void StartOrthogonal();
    void EndOrthogonal();
    void DrawBackground();

private:
    static void init_buffers(GLuint *vao_out, GLuint *buffer_out);
    static void init_bk_buffers(GLuint *vao_out, GLuint *buffer_out);
    static GLuint create_shader(int type, const char *src);
    static void init_shaders(const char *vertex_shader_code, const char *fragment_shader_code, GLuint *program_out, GLuint *mvp_out);
    static void compute_mvp(float *res, float  phi, float  theta, float  psi);
    static void draw_triangle(wxGLAreaWidget *glArea);
    static void realize_cb(GtkWidget *widget, gpointer user_data);
    static void unrealize_cb(GtkWidget *widget, gpointer user_data);
    static gboolean render_cb(GtkGLArea *area, GdkGLContext *context, gpointer user_data);

private:
    typedef wxControl base_type;

private:
    GLuint position_buffer = 0;
    GLuint program = 0;
    GLuint mvp_location = 0;
    GLuint bk_texture = 0;
    GLuint bk_position_buffer = 0;
    GLuint bk_program = 0;
    float rotation_angles[N_AXIS] = { 0.0 };
    float anchor_angles[N_AXIS] = { 0.0 };
    wxPoint anchorPos_;
    wxPoint lastPos_;

    wxDECLARE_DYNAMIC_CLASS(wxGLAreaWidget);
};

#endif // _WX_GTK_GL_AREA_WIDGET_H_

