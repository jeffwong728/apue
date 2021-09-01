#include "wx/wxprec.h"
#include <wx/wx.h>
#include <wx/log.h>
#include "gtkglareawidget.h"
#include "glmodeltree.h"
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
#include <vtkPolygon.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkTexture.h>
#include <vtkProperty.h>
#include <ExternalVTKWidget.h>
#include <vtkExternalOpenGLCamera.h>
#include <vtkExternalOpenGLRenderWindow.h>
#include <vtkOpenGLRenderer.h>
#include <vtkTransform.h>
#include <vtkSTLReader.h>

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

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, FALSE);
    gtk_widget_set_halign(box, GTK_ALIGN_START);
    gtk_widget_set_valign(box, GTK_ALIGN_START);
    gtk_overlay_add_overlay(GTK_OVERLAY(m_widget), box);

    modelTreeView_ = GLModelTreeView::MakeNew();
    GtkWidget *expander = gtk_expander_new("Model Tree");
    gtk_expander_set_resize_toplevel(GTK_EXPANDER(expander), TRUE);
    gtk_container_add(GTK_CONTAINER(expander), modelTreeView_->GetWidget());
    gtk_container_add(GTK_CONTAINER(box), expander);

    gtk_widget_show(glWidget);
    gtk_widget_show_all(box);

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

void wxGLAreaWidget::ImportSTL(const std::string &inputFilename)
{
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName(inputFilename.c_str());
    reader->Update();

    // Visualize
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(reader->GetOutputPort());

    vtkNew<vtkActor> actor;
    vtkNew<vtkNamedColors> colors;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetDiffuse(1.0);
    actor->GetProperty()->SetDiffuseColor(colors->GetColor3d("IndianRed").GetData());
    actor->GetProperty()->SetSpecular(0.3);
    actor->GetProperty()->SetSpecularPower(60.0);

    rootRenderer->AddActor(actor);
    rootRenderer->ResetCamera();
    Refresh(false);
}

void wxGLAreaWidget::OnSize(wxSizeEvent &e)
{
    const int w = e.GetSize().GetWidth();
    const int h = e.GetSize().GetHeight();
    if (externalVTKWidget) externalVTKWidget->GetRenderWindow()->SetSize(w, h);
}

void wxGLAreaWidget::OnLeftMouseDown(wxMouseEvent &e)
{
    if (e.LeftIsDown() && e.RightIsDown())
    {
        rootRenderer->ResetCamera();
        Refresh(false);
    }

    lastPos_ = e.GetPosition();
}

void wxGLAreaWidget::OnLeftMouseUp(wxMouseEvent &e)
{
    lastPos_ = e.GetPosition();
}

void wxGLAreaWidget::OnRightMouseDown(wxMouseEvent &e)
{
    if (e.LeftIsDown() && e.RightIsDown())
    {
        rootRenderer->ResetCamera();
        Refresh(false);
    }

    anchorPos_ = e.GetPosition();
    lastPos_ = e.GetPosition();
}

void wxGLAreaWidget::OnRightMouseUp(wxMouseEvent &e)
{
    anchorPos_ = wxPoint();
    lastPos_ = wxPoint();
}

void wxGLAreaWidget::OnMiddleDown(wxMouseEvent &e)
{
    anchorPos_ = e.GetPosition();
    lastPos_ = e.GetPosition();
}

void wxGLAreaWidget::OnMiddleUp(wxMouseEvent &e)
{
    anchorPos_ = wxPoint();
    lastPos_ = wxPoint();
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
        double viewFocus[4], focalDepth, viewPoint[3];
        double newPickPoint[4], oldPickPoint[4], motionVector[3];

        const int h = externalVTKWidget->GetRenderWindow()->GetSize()[1];
        vtkCamera* camera = rootRenderer->GetActiveCamera();
        camera->GetFocalPoint(viewFocus);
        this->ComputeWorldToDisplay(rootRenderer, viewFocus[0], viewFocus[1], viewFocus[2], viewFocus);
        focalDepth = viewFocus[2];

        this->ComputeDisplayToWorld(rootRenderer, x, h - y, focalDepth, newPickPoint);
        this->ComputeDisplayToWorld(rootRenderer, lastPos_.x, h - lastPos_.y, focalDepth, oldPickPoint);

        motionVector[0] = oldPickPoint[0] - newPickPoint[0];
        motionVector[1] = oldPickPoint[1] - newPickPoint[1];
        motionVector[2] = oldPickPoint[2] - newPickPoint[2];

        camera->GetFocalPoint(viewFocus);
        camera->GetPosition(viewPoint);
        camera->SetFocalPoint(motionVector[0] + viewFocus[0], motionVector[1] + viewFocus[1], motionVector[2] + viewFocus[2]);
        camera->SetPosition(motionVector[0] + viewPoint[0], motionVector[1] + viewPoint[1], motionVector[2] + viewPoint[2]);
        rootRenderer->UpdateLightsGeometryToFollowCamera();

        if (dx || dx)
        {
            Refresh(false);
        }
    }

    if (e.Dragging() && e.MiddleIsDown() && this->axisRenderer)
    {
        const int* size = this->axisRenderer->GetRenderWindow()->GetSize();

        double delta_elevation = -20.0 / size[1];
        double delta_azimuth = -20.0 / size[0];

        double rxf = dx * delta_azimuth * 10.0;
        double ryf = dy * delta_elevation * -10.0;

        vtkCamera* camera = this->axisRenderer->GetActiveCamera();
        camera->Azimuth(rxf);
        camera->Elevation(ryf);
        camera->OrthogonalizeViewUp();

        camera = this->rootRenderer->GetActiveCamera();
        camera->Azimuth(rxf);
        camera->Elevation(ryf);
        camera->OrthogonalizeViewUp();

        this->axisRenderer->ResetCameraClippingRange();
        this->axisRenderer->UpdateLightsGeometryToFollowCamera();

        this->rootRenderer->ResetCameraClippingRange();
        this->rootRenderer->UpdateLightsGeometryToFollowCamera();

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
    vtkCamera* camera = this->rootRenderer->GetActiveCamera();
    camera->SetParallelScale(camera->GetParallelScale() * s);
    this->rootRenderer->UpdateLightsGeometryToFollowCamera();
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

void wxGLAreaWidget::ComputeWorldToDisplay(vtkRenderer* ren, double x, double y, double z, double displayPt[3])
{
    ren->SetWorldPoint(x, y, z, 1.0);
    ren->WorldToDisplay();
    ren->GetDisplayPoint(displayPt);
}

void wxGLAreaWidget::ComputeDisplayToWorld(vtkRenderer* ren, double x, double y, double z, double worldPt[4])
{
    ren->SetDisplayPoint(x, y, z);
    ren->DisplayToWorld();
    ren->GetWorldPoint(worldPt);
    if (worldPt[3])
    {
        worldPt[0] /= worldPt[3];
        worldPt[1] /= worldPt[3];
        worldPt[2] /= worldPt[3];
        worldPt[3] = 1.0;
    }
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

    wxGLAreaWidget *glArea = reinterpret_cast<wxGLAreaWidget *>(user_data);
    glArea->externalVTKWidget = vtkSmartPointer<ExternalVTKWidget>::New();
    auto renWin = glArea->externalVTKWidget->GetRenderWindow();

    renWin->AutomaticWindowPositionAndResizeOff();
    renWin->SetUseExternalContent(false);

    vtkNew<vtkCallbackCommand> callback;
    callback->SetCallback(MakeCurrentCallback);
    renWin->AddObserver(vtkCommand::WindowMakeCurrentEvent, callback);
    renWin->SetNumberOfLayers(3);

    vtkNew<vtkPoints> points;
    points->InsertNextPoint(1.0, 1.0, 0.0);
    points->InsertNextPoint(1.0, -1.0, 0.0);
    points->InsertNextPoint(-1.0, -1.0, 0.0);
    points->InsertNextPoint(-1.0, 1.0, 0.0);

    vtkNew<vtkCellArray> polygons;
    vtkNew<vtkPolygon> polygon;
    polygon->GetPointIds()->SetNumberOfIds(4); // make a quad
    polygon->GetPointIds()->SetId(0, 0);
    polygon->GetPointIds()->SetId(1, 1);
    polygon->GetPointIds()->SetId(2, 2);
    polygon->GetPointIds()->SetId(3, 3);

    polygons->InsertNextCell(polygon);

    vtkNew<vtkPolyData> quad;
    quad->SetPoints(points);
    quad->SetPolys(polygons);

    vtkNew<vtkPolyDataMapper> bkmapper;
    bkmapper->SetInputData(quad);

    vtkNew<vtkNamedColors> colors;
    vtkNew<vtkActor> texturedQuad;
    texturedQuad->SetMapper(bkmapper);

    boost::system::error_code ec;
    boost::filesystem::path p = boost::dll::program_location(ec);
    p = p.parent_path();
    p.append(wxT("res")).append(wxT("Apex.png"));

    if (boost::filesystem::exists(p, ec) && boost::filesystem::is_regular_file(p, ec))
    {
        std::string texFilePath(wxString(p.native()));
        vtkNew<vtkFloatArray> textureCoordinates;
        textureCoordinates->SetNumberOfComponents(2);
        textureCoordinates->SetName("TextureCoordinates");

        float tuple[2] = { 1.0, 1.0 };
        textureCoordinates->InsertNextTuple(tuple);
        tuple[0] = 1.0; tuple[1] = 0.0;
        textureCoordinates->InsertNextTuple(tuple);
        tuple[0] = 0.0; tuple[1] = 0.0;
        textureCoordinates->InsertNextTuple(tuple);
        tuple[0] = 0.0; tuple[1] = 1.0;
        textureCoordinates->InsertNextTuple(tuple);
        quad->GetPointData()->SetTCoords(textureCoordinates);

        vtkNew<vtkImageReader2Factory> readerFactory;
        vtkSmartPointer<vtkImageReader2> textureFile;
        textureFile.TakeReference(readerFactory->CreateImageReader2(texFilePath.c_str()));
        textureFile->SetFileName(texFilePath.c_str());
        textureFile->Update();

        vtkNew<vtkTexture> atext;
        atext->SetInputConnection(textureFile->GetOutputPort());
        atext->InterpolateOn();
        texturedQuad->SetTexture(atext);
    }

    auto bkRenderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
    bkRenderer->AddActor(texturedQuad);
    bkRenderer->SetLayer(0);
    bkRenderer->InteractiveOff();

    auto bkCamera = vtkSmartPointer<vtkExternalOpenGLCamera>::New();
    bkRenderer->SetActiveCamera(bkCamera);
    const auto vMat = glm::highp_dmat4(1.0);
    const auto pMat = glm::ortho(-1.0, 1.0, -1.0, 1.0, -10.0, 10.0);
    bkCamera->SetViewTransformMatrix(glm::value_ptr(vMat));
    bkCamera->SetProjectionTransformMatrix(glm::value_ptr(pMat));
    renWin->AddRenderer(bkRenderer);

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
    sphereActor->VisibilityOff();
    sphereActor->SetMapper(sphereMapper);

    // a renderer and render window
    glArea->axisRenderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
    glArea->axisRenderer->GetActiveCamera()->ParallelProjectionOn();
    renWin->AddRenderer(glArea->axisRenderer);
    glArea->axisRenderer->SetLayer(1);

    // add the actors to the scene
    glArea->axisRenderer->AddActor(sphereActor);
    glArea->axisRenderer->SetBackground(colors->GetColor3d("BkgColor").GetData());

    vtkNew<vtkAxesActor> axes;
    axes->VisibilityOff();
    axes->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(12);
    axes->GetYAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(12);
    axes->GetZAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(12);
    axes->SetAxisLabels(false);
    glArea->axisRenderer->AddActor(axes);
    glArea->axisRenderer->ResetCamera();
    glArea->axisRenderer->GetActiveCamera()->Azimuth(45);
    glArea->axisRenderer->GetActiveCamera()->Elevation(-30);
    glArea->axisRenderer->GetActiveCamera()->Zoom(0.5);

    glArea->rootRenderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
    glArea->rootRenderer->GetActiveCamera()->ParallelProjectionOn();
    renWin->AddRenderer(glArea->rootRenderer);
    glArea->rootRenderer->SetLayer(2);

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
    glArea->axisRenderer = nullptr;
    glArea->externalVTKWidget = nullptr;
    glArea->rootRenderer = nullptr;
}

gboolean wxGLAreaWidget::render_cb(GtkGLArea *area, GdkGLContext *context, gpointer user_data)
{
    wxGLAreaWidget *glArea = reinterpret_cast<wxGLAreaWidget *>(user_data);
    glArea->externalVTKWidget->GetRenderWindow()->Render();
    return TRUE;
}
