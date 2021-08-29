#include "wx/wxprec.h"
#include <wx/wx.h>
#include <wx/log.h>
#include "gtkglareawidget.h"
#include "wx/gtk/private.h"
#include "wx/gtk/private/eventsdisabler.h"
#include "wx/gtk/private/list.h"
#include <opencv2/opencv.hpp>
#include <epoxy/gl.h>
#include <GL/glu.h>
#include <boost/filesystem.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <vtkNew.h>
#include <vtkLight.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkCallbackCommand.h>
#include <vtkLogger.h>
#include <vtkNamedColors.h>
#include <vtkAxesActor.h>
#include <vtkSphereSource.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextProperty.h>
#include <ExternalVTKWidget.h>
#include <vtkExternalOpenGLRenderWindow.h>

//gl_FrontFacing
//gl_PrimitiveID
//vtkOpenGLPolyDataMapper::SetCameraShaderParameters

namespace {
void MakeCurrentCallback(vtkObject* vtkNotUsed(caller),
    long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData), void* vtkNotUsed(callData))
{
}
};

wxIMPLEMENT_DYNAMIC_CLASS(wxGLAreaWidget, wxControl);

bool wxGLAreaWidget::Create(wxWindow *parent, wxWindowID id,
                            const wxPoint &pos,
                            const wxSize &size, long style,
                            const wxValidator& validator,
                            const wxString &name)
{
    if (!PreCreation(parent, pos, size) ||
        !CreateBase(parent, id, pos, size, style, validator, name ))
    {
        wxFAIL_MSG(wxT("wxGLAreaWidget creation failed"));
        return false;
    }

    m_widget = gtk_overlay_new();
    g_object_ref_sink(m_widget);

    GtkWidget *glWidget = gtk_gl_area_new();
    gtk_container_add(GTK_CONTAINER(m_widget), glWidget);
    gtk_widget_add_events(glWidget, GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK);
    gtk_gl_area_set_required_version(GTK_GL_AREA(glWidget), 4, 5);

    g_signal_connect(glWidget, "realize", G_CALLBACK(realize_cb), this);
    g_signal_connect(glWidget, "unrealize", G_CALLBACK(unrealize_cb), this);
    g_signal_connect(glWidget, "render", G_CALLBACK(render_cb), this);

    GtkWidget *expander = gtk_expander_new("Model Tree");
    gtk_widget_set_halign(expander, GTK_ALIGN_START);
    gtk_widget_set_valign(expander, GTK_ALIGN_START);
    gtk_overlay_add_overlay(GTK_OVERLAY(m_widget), expander);

    GtkWidget *entry = gtk_entry_new();
    gtk_widget_set_opacity(entry, 0.6);
    gtk_container_add(GTK_CONTAINER(expander), entry);

    gtk_widget_show(glWidget);
    gtk_widget_show_all(expander);

    m_parent->DoAddChild(this);

    PostCreation(size);

    Bind(wxEVT_SIZE,        &wxGLAreaWidget::OnSize,            this, wxID_ANY);
    Bind(wxEVT_MOTION,      &wxGLAreaWidget::OnMouseMotion,     this, wxID_ANY);
    Bind(wxEVT_LEFT_DCLICK, &wxGLAreaWidget::OnMiddleUp,        this, wxID_ANY);
    Bind(wxEVT_MIDDLE_DOWN, &wxGLAreaWidget::OnMiddleDown,      this, wxID_ANY);
    Bind(wxEVT_LEFT_DOWN,   &wxGLAreaWidget::OnLeftMouseDown,   this, wxID_ANY);
    Bind(wxEVT_LEFT_UP,     &wxGLAreaWidget::OnLeftMouseUp,     this, wxID_ANY);
    Bind(wxEVT_RIGHT_DOWN,  &wxGLAreaWidget::OnRightMouseDown,  this, wxID_ANY);
    Bind(wxEVT_RIGHT_UP,    &wxGLAreaWidget::OnRightMouseUp,    this, wxID_ANY);
    Bind(wxEVT_SET_FOCUS,   &wxGLAreaWidget::OnSetFocus,        this, wxID_ANY);
    Bind(wxEVT_KILL_FOCUS,  &wxGLAreaWidget::OnKillFocus,       this, wxID_ANY);
    Bind(wxEVT_MOUSEWHEEL,  &wxGLAreaWidget::OnMouseWheel,      this, wxID_ANY);

    norm_ = glm::mat4(1.f);
    norm_ = glm::rotate(norm_, glm::radians(45.f), glm::vec3(-1.0f, 0.0f, 0.0f));
    norm_ = glm::rotate(norm_, glm::radians(45.f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelview_ = norm_;
    projection_ = glm::mat4(1.0f);

    vtkLogger::LogToFile("everything.log", vtkLogger::TRUNCATE, vtkLogger::VERBOSITY_INFO);

    return true;
}

void wxGLAreaWidget::GTKDisableEvents()
{
}

void wxGLAreaWidget::GTKEnableEvents()
{
}

void wxGLAreaWidget::SetAxisAngleValue(const int axisIndex, const int angleVal)
{
    wxCHECK_RET(m_widget != NULL, wxT("invalid switch button"));
    Refresh(false);
}

bool wxGLAreaWidget::GetValue() const
{
    wxCHECK_MSG(m_widget != NULL, false, wxT("invalid switch button"));

    return gtk_switch_get_active(GTK_SWITCH(m_widget)) != 0;
}

void wxGLAreaWidget::OnSize(wxSizeEvent &e)
{
    const int w = e.GetSize().GetWidth();
    const int h = e.GetSize().GetHeight();

    top_ = 1.f;
    bottom_ = -1.f;
    left_ = -(float)w / (float)h;
    right_ = -left_;

    projection_ = glm::ortho(left_, right_, bottom_, top_, zNear_, zFar_);
    if (externalVTKWidget) externalVTKWidget->GetRenderWindow()->SetSize(w, h);
}

void wxGLAreaWidget::OnLeftMouseDown(wxMouseEvent &e)
{
    if (e.LeftIsDown() && e.RightIsDown())
    {
        modelview_ = norm_;
        Refresh(false);
    }

    lastPos_ = e.GetPosition();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    pos(&dragPosX_, &dragPosY_, &dragPosZ_, lastPos_.x, lastPos_.y, viewport);
}

void wxGLAreaWidget::OnLeftMouseUp(wxMouseEvent &e)
{
    lastPos_ = e.GetPosition();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    pos(&dragPosX_, &dragPosY_, &dragPosZ_, lastPos_.x, lastPos_.y, viewport);
}

void wxGLAreaWidget::OnRightMouseDown(wxMouseEvent &e)
{
    if (e.LeftIsDown() && e.RightIsDown())
    {
        modelview_ = norm_;
        Refresh(false);
    }

    anchorPos_ = e.GetPosition();
    lastPos_ = e.GetPosition();
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    pos(&dragPosX_, &dragPosY_, &dragPosZ_, lastPos_.x, lastPos_.y, viewport);
}

void wxGLAreaWidget::OnRightMouseUp(wxMouseEvent &e)
{
    anchorPos_ = wxPoint();
    lastPos_ = wxPoint();
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    pos(&dragPosX_, &dragPosY_, &dragPosZ_, lastPos_.x, lastPos_.y, viewport);
}

void wxGLAreaWidget::OnMiddleDown(wxMouseEvent &e)
{
    anchorPos_ = e.GetPosition();
    lastPos_ = e.GetPosition();
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    pos(&dragPosX_, &dragPosY_, &dragPosZ_, lastPos_.x, lastPos_.y, viewport);
}

void wxGLAreaWidget::OnMiddleUp(wxMouseEvent &e)
{
    anchorPos_ = wxPoint();
    lastPos_ = wxPoint();
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    pos(&dragPosX_, &dragPosY_, &dragPosZ_, lastPos_.x, lastPos_.y, viewport);
}

void wxGLAreaWidget::OnMouseMotion(wxMouseEvent &e)
{
    const int x = e.GetPosition().x;
    const int y = e.GetPosition().y;
    const int dx = x - lastPos_.x;
    const int dy = y - lastPos_.y;
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    if (e.Dragging() && e.RightIsDown())
    {
        float px, py, pz;
        pos(&px, &py, &pz, x, y, viewport);
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, glm::vec3(px - dragPosX_, py - dragPosY_, pz - dragPosZ_));
        modelview_ = m * modelview_;

        dragPosX_ = px;
        dragPosY_ = py;
        dragPosZ_ = pz;

        if (dx || dx)
        {
            Refresh(false);
        }
    }

    if (e.Dragging() && e.MiddleIsDown())
    {
        const float ax = dy;
        const float ay = dx;
        const float az = 0.0;
        const float angle = vlen(ax, ay, az) / (double)(viewport[2] + 1) * 360.f;

        glm::mat4 invm = glm::inverse(modelview_);
        const float bx = invm[0][0] * ax + invm[1][0] * ay + invm[2][0] * az;
        const float by = invm[0][1] * ax + invm[1][1] * ay + invm[2][1] * az;
        const float bz = invm[0][2] * ax + invm[1][2] * ay + invm[2][2] * az;

        modelview_ = glm::translate(modelview_, glm::vec3(0.f, 0.f, 0.f));
        modelview_ = glm::rotate(modelview_, glm::radians(angle), glm::vec3(bx, by, bz));
        modelview_ = glm::translate(modelview_, glm::vec3(0.f, 0.f, 0.f));

        if (dx || dx)
        {
            Refresh(false);
        }
    }

    lastPos_ = e.GetPosition();
}

void wxGLAreaWidget::OnMouseWheel(wxMouseEvent &e)
{
    const float s = e.GetWheelRotation() > 0 ? 1.2f : 0.8f;
    modelview_ = glm::scale(modelview_, glm::vec3(s, s, s));
    Refresh(false);
}

void wxGLAreaWidget::OnSetFocus(wxFocusEvent &e)
{
    wxLogMessage(wxT("wxGLAreaWidget::OnSetFocus."));
}

void wxGLAreaWidget::OnKillFocus(wxFocusEvent &e)
{
    wxLogMessage(wxT("wxGLAreaWidget::OnKillFocus."));
}

void wxGLAreaWidget::DoApplyWidgetStyle(GtkRcStyle *style)
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

wxSize wxGLAreaWidget::DoGetBestSize() const
{
    return wxSize(64, 48);
}

void wxGLAreaWidget::pos(float *px, float *py, float *pz, const int x, const int y, const int *viewport) const
{
    *px = (double)(x - viewport[0]) / (double)(viewport[2]);
    *py = (double)(y - viewport[1]) / (double)(viewport[3]);

    *px = left_ + (*px)*(right_ - left_);
    *py = top_ + (*py)*(bottom_ - top_);
    *pz = zNear_;
}

void wxGLAreaWidget::realize_cb(GtkWidget *widget, gpointer user_data)
{
    gtk_gl_area_make_current(GTK_GL_AREA(widget));

    if (gtk_gl_area_get_error(GTK_GL_AREA(widget)) != NULL)
        return;

    GdkGLContext *context = gtk_gl_area_get_context(GTK_GL_AREA(widget));
    if (gdk_gl_context_get_use_es(context) || gdk_gl_context_is_legacy(context))
    {
        return;
    }

    gint major = 0;
    gint minor = 0;
    gtk_gl_area_get_required_version(GTK_GL_AREA(widget), &major, &minor);

    wxGLAreaWidget *glArea = reinterpret_cast<wxGLAreaWidget *>(user_data);
    glArea->externalVTKWidget = vtkSmartPointer<ExternalVTKWidget>::New();
    auto renWin = glArea->externalVTKWidget->GetRenderWindow();

    renWin->AutomaticWindowPositionAndResizeOff();
    renWin->SetUseExternalContent(false);

    assert(renWin != nullptr);
    vtkNew<vtkCallbackCommand> callback;
    callback->SetCallback(MakeCurrentCallback);
    renWin->AddObserver(vtkCommand::WindowMakeCurrentEvent, callback);

    vtkNew<vtkNamedColors> colors;
    std::array<unsigned char, 4> bkg{{26, 51, 102, 255}};
    colors->SetColor("BkgColor", bkg.data());
    vtkNew<vtkSphereSource> sphereSource;
    sphereSource->SetCenter(0.0, 0.0, 0.0);
    sphereSource->SetRadius(0.1);

    // create a mapper
    vtkNew<vtkPolyDataMapper> sphereMapper;
    sphereMapper->SetInputConnection(sphereSource->GetOutputPort());

    // create an actor
    vtkNew<vtkActor> sphereActor;
    sphereActor->SetMapper(sphereMapper);

    // a renderer and render window
    vtkNew<vtkOpenGLRenderer> renderer;
    renWin->AddRenderer(renderer);

    // add the actors to the scene
    renderer->AddActor(sphereActor);
    renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());

    vtkNew<vtkAxesActor> axes;
    axes->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(12);
    axes->GetYAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(12);
    axes->GetZAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(12);
    axes->SetAxisLabels(false);
    renderer->AddActor(axes);
    renderer->ResetCamera();
    renderer->GetActiveCamera()->Azimuth(45);
    renderer->GetActiveCamera()->Elevation(-30);
    renderer->GetActiveCamera()->Zoom(0.5);

    const wxString gtkMajorVersion(std::to_string(gtk_get_major_version()));
    const wxString gtkMinorVersion(std::to_string(gtk_get_minor_version()));
    const wxString gtkMicroVersion(std::to_string(gtk_get_micro_version()));
    wxLogMessage(wxString("GTK Version: ") + gtkMajorVersion + wxString(wxT(".")) + gtkMinorVersion + wxString(wxT(".")) + gtkMicroVersion);

    const wxString glVendor(reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
    const wxString glRenderer(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    const wxString glVersion(reinterpret_cast<const char *>(glGetString(GL_VERSION)));
    wxLogMessage(wxString("OpenGL Vendor: ") + glVendor);
    wxLogMessage(wxString("OpenGL Renderer: ") + glRenderer);
    wxLogMessage(wxString("OpenGL Version: ") + glVersion);
}

void wxGLAreaWidget::unrealize_cb(GtkWidget *widget, gpointer user_data)
{
    gtk_gl_area_make_current(GTK_GL_AREA(widget));

    if (gtk_gl_area_get_error(GTK_GL_AREA(widget)) != NULL)
        return;

    wxGLAreaWidget *glArea = reinterpret_cast<wxGLAreaWidget *>(user_data);
    glArea->externalVTKWidget = nullptr;
}

gboolean wxGLAreaWidget::render_cb(GtkGLArea *area, GdkGLContext *context, gpointer user_data)
{
    wxGLAreaWidget *glArea = reinterpret_cast<wxGLAreaWidget *>(user_data);
    glArea->externalVTKWidget->GetRenderWindow()->Render();
    return TRUE;
}
