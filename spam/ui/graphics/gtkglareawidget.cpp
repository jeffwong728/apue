#include "wx/wxprec.h"
#include <wx/wx.h>
#include <wx/log.h>
#include "dispnode.h"
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
#include <vtkMath.h>
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
#include <vtkPLYReader.h>
#include <vtkOBJReader.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkXMLGenericDataObjectReader.h>
#include <vtkOBBTree.h>
#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtksys/SystemTools.hxx>
#if defined(M_PI) && defined(_WIN32)
#undef M_PI
#endif
// VIS includes
#include <IVtkOCC_Shape.hxx>
#include <IVtkTools_ShapeDataSource.hxx>
#include <IVtkTools_ShapeObject.hxx>
#include <IVtkTools_ShapePicker.hxx>
#include <IVtkTools_SubPolyDataFilter.hxx>

// OCCT includes
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>
#include <TColStd_PackedMapOfInteger.hxx>

//gl_FrontFacing
//gl_PrimitiveID
//vtkOpenGLPolyDataMapper::SetCameraShaderParameters
//VTK_ENABLE_KITS

namespace {
void MakeCurrentCallback(vtkObject* vtkNotUsed(caller),
    long unsigned int vtkNotUsed(eventId), void* clientData, void* vtkNotUsed(callData))
{
    GtkWidget *widget = (GtkWidget *)clientData;
    GdkGLContext *context = gtk_gl_area_get_context(GTK_GL_AREA(widget));
    GdkGLContext *ccontext = gdk_gl_context_get_current();
    if (context != ccontext)
    {
        gtk_gl_area_make_current(GTK_GL_AREA(widget));
        GError *e = gtk_gl_area_get_error(GTK_GL_AREA(widget));
        if (e) wxLogMessage(wxString(e->message));
    }
}

void IsCurrentCallback(vtkObject* vtkNotUsed(caller),
    long unsigned int vtkNotUsed(eventId), void* clientData, void* callData)
{
    GtkWidget *widget = (GtkWidget *)clientData;
    GdkGLContext *context = gtk_gl_area_get_context(GTK_GL_AREA(widget));
    GdkGLContext *ccontext = gdk_gl_context_get_current();
    bool& cstatus = *reinterpret_cast<bool*>(callData);
    cstatus = context == ccontext;
}

void EPOXY_CALLSPEC glDebugOutput(GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char *message,
    const void *userParam)
{
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::ostringstream oss;
    oss << "---------------" << std::endl;
    oss << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             oss << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   oss << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: oss << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     oss << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     oss << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           oss << "Source: Other"; break;
    } oss << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               oss << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: oss << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  oss << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         oss << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         oss << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              oss << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          oss << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           oss << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               oss << "Type: Other"; break;
    } oss << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         oss << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       oss << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          oss << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: oss << "Severity: notification"; break;
    } oss << std::endl;
    oss << std::endl;

    wxLogMessage(wxString(oss.str()));
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
    g_signal_connect(glWidget, "create-context", G_CALLBACK(create_context_cb), this);

    GtkWidget *box = gtk_search_bar_new();
    gtk_search_bar_set_search_mode(GTK_SEARCH_BAR(box), TRUE);
    gtk_search_bar_set_show_close_button(GTK_SEARCH_BAR(box), FALSE);
    gtk_widget_set_halign(box, GTK_ALIGN_START);
    gtk_widget_set_valign(box, GTK_ALIGN_START);
    gtk_overlay_add_overlay(GTK_OVERLAY(m_widget), box);

    modelTreeView_ = GLModelTreeView::MakeNew(parent);
    GtkWidget *expander = gtk_expander_new("Model Tree");
    gtk_expander_set_resize_toplevel(GTK_EXPANDER(expander), FALSE);
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

    modelTreeView_->sig_AddGeomBody.connect(std::bind(&wxGLAreaWidget::OnAddGeomBody, this, std::placeholders::_1, std::placeholders::_2));
    modelTreeView_->sig_ColorChanged.connect(std::bind(&wxGLAreaWidget::OnColorChanged, this, std::placeholders::_1, std::placeholders::_2));
    modelTreeView_->sig_VisibilityChanged.connect(std::bind(&wxGLAreaWidget::OnVisibilityChanged, this, std::placeholders::_1, std::placeholders::_2));
    modelTreeView_->sig_ShowNodeChanged.connect(std::bind(&wxGLAreaWidget::OnShowNodeChanged, this, std::placeholders::_1, std::placeholders::_2));
    modelTreeView_->sig_RepresentationChanged.connect(std::bind(&wxGLAreaWidget::OnRepresentationChanged, this, std::placeholders::_1, std::placeholders::_2));
    modelTreeView_->sig_EntitiesDeleted.connect(std::bind(&wxGLAreaWidget::OnEntitiesDeleted, this, std::placeholders::_1));
    modelTreeView_->sig_ImportModel.connect(std::bind(&wxGLAreaWidget::OnImportModel, this, std::placeholders::_1));

    colors_->GetColorNames(colorNames_);
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

void wxGLAreaWidget::ImportSTL(const GLGUID &parentGuid, const std::string &inputFilename)
{
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName(inputFilename.c_str());
    reader->Update();

    SPDispNodes newDispNodes = GLDispNode::MakeNew(reader->GetOutput(), rootRenderer);
    if (!newDispNodes.empty())
    {
        const auto numColors = colors_->GetNumberOfColors();
        for (const auto &newDispNode : newDispNodes)
        {
            newDispNode->SetCellColor(colors_->GetColor3d(colorNames_->GetValue((colorIndex_++) % numColors)).GetData());
            newDispNode->SetEdgeColor(colors_->GetColor3d("Black").GetData());
            newDispNode->SetNodeColor(colors_->GetColor3d("Blue").GetData());
            allActors_[newDispNode->GetGUID()] = newDispNode;
        }
        boost::filesystem::path p(inputFilename);
        modelTreeView_->AddPart(parentGuid, p.filename().string(), newDispNodes);
        rootRenderer->ResetCamera();
        Refresh(false);
    }
}

void wxGLAreaWidget::ImportVTK(const GLGUID &parentGuid, const std::string &inputFilename)
{
    vtkNew<vtkGenericDataObjectReader> reader;
    reader->SetFileName(inputFilename.c_str());
    reader->Update();

    SPDispNodes newDispNodes;
    if (reader->IsFilePolyData())
    {
        newDispNodes = GLDispNode::MakeNew(reader->GetPolyDataOutput(), rootRenderer);
    }
    else if (reader->IsFileUnstructuredGrid())
    {
        newDispNodes = GLDispNode::MakeNew(reader->GetUnstructuredGridOutput(), rootRenderer);
    }
    else
    {
        wxMessageBox(wxString(wxT("Don't know how to read this vtk file")), wxString(wxT("Import Failed")));
    }

    if (!newDispNodes.empty())
    {
        const auto numColors = colors_->GetNumberOfColors();
        for (const auto &newDispNode : newDispNodes)
        {
            newDispNode->SetCellColor(colors_->GetColor3d(colorNames_->GetValue((colorIndex_++)%numColors)).GetData());
            newDispNode->SetEdgeColor(colors_->GetColor3d("Black").GetData());
            newDispNode->SetNodeColor(colors_->GetColor3d("Blue").GetData());
            allActors_[newDispNode->GetGUID()] = newDispNode;
        }
        boost::filesystem::path p(inputFilename);
        modelTreeView_->AddPart(parentGuid, p.filename().string(), newDispNodes);
        rootRenderer->ResetCamera();
        Refresh(false);
    }
}

void wxGLAreaWidget::ImportVTU(const GLGUID &parentGuid, const std::string &inputFilename)
{
    vtkNew<vtkXMLGenericDataObjectReader> reader;
    reader->SetFileName(inputFilename.c_str());
    reader->Update();

    SPDispNodes newDispNodes;
    if (reader->GetPolyDataOutput())
    {
        newDispNodes = GLDispNode::MakeNew(reader->GetPolyDataOutput(), rootRenderer);
    }
    else if (reader->GetUnstructuredGridOutput())
    {
        newDispNodes = GLDispNode::MakeNew(reader->GetUnstructuredGridOutput(), rootRenderer);
    }
    else
    {
        wxMessageBox(wxString(wxT("Don't know how to read this vtk file")), wxString(wxT("Import Failed")));
    }

    if (!newDispNodes.empty())
    {
        const auto numColors = colors_->GetNumberOfColors();
        for (const auto &newDispNode : newDispNodes)
        {
            newDispNode->SetCellColor(colors_->GetColor3d(colorNames_->GetValue((colorIndex_++) % numColors)).GetData());
            newDispNode->SetEdgeColor(colors_->GetColor3d("Black").GetData());
            newDispNode->SetNodeColor(colors_->GetColor3d("Blue").GetData());
            allActors_[newDispNode->GetGUID()] = newDispNode;
        }
        boost::filesystem::path p(inputFilename);
        modelTreeView_->AddPart(parentGuid, p.filename().string(), newDispNodes);
        rootRenderer->ResetCamera();
        Refresh(false);
    }
}

void wxGLAreaWidget::ImportOBJ(const GLGUID &parentGuid, const std::string &inputFilename)
{
    vtkNew<vtkOBJReader> reader;
    reader->SetFileName(inputFilename.c_str());
    reader->Update();

    SPDispNodes newDispNodes = GLDispNode::MakeNew(reader->GetOutput(), rootRenderer);
    if (!newDispNodes.empty())
    {
        const auto numColors = colors_->GetNumberOfColors();
        for (const auto &newDispNode : newDispNodes)
        {
            newDispNode->SetCellColor(colors_->GetColor3d(colorNames_->GetValue((colorIndex_++) % numColors)).GetData());
            newDispNode->SetEdgeColor(colors_->GetColor3d("Black").GetData());
            newDispNode->SetNodeColor(colors_->GetColor3d("Blue").GetData());
            allActors_[newDispNode->GetGUID()] = newDispNode;
        }
        boost::filesystem::path p(inputFilename);
        modelTreeView_->AddPart(parentGuid, p.filename().string(), newDispNodes);
        rootRenderer->ResetCamera();
        Refresh(false);
    }
}

void wxGLAreaWidget::ImportPLY(const GLGUID &parentGuid, const std::string &inputFilename)
{
    vtkNew<vtkPLYReader> reader;
    reader->SetFileName(inputFilename.c_str());
    reader->Update();

    SPDispNodes newDispNodes = GLDispNode::MakeNew(reader->GetOutput(), rootRenderer);
    if (!newDispNodes.empty())
    {
        const auto numColors = colors_->GetNumberOfColors();
        for (const auto &newDispNode : newDispNodes)
        {
            newDispNode->SetCellColor(colors_->GetColor3d(colorNames_->GetValue((colorIndex_++) % numColors)).GetData());
            newDispNode->SetEdgeColor(colors_->GetColor3d("Black").GetData());
            newDispNode->SetNodeColor(colors_->GetColor3d("Blue").GetData());
            allActors_[newDispNode->GetGUID()] = newDispNode;
        }
        boost::filesystem::path p(inputFilename);
        modelTreeView_->AddPart(parentGuid, p.filename().string(), newDispNodes);
        rootRenderer->ResetCamera();
        Refresh(false);
    }
}

void wxGLAreaWidget::CloseModel()
{
    rootRenderer->RemoveAllViewProps();
    allActors_.clear();
    modelTreeView_->CloseModel();
    Refresh(false);
}

void wxGLAreaWidget::OnSize(wxSizeEvent &e)
{
    const int w = e.GetSize().GetWidth();
    const int h = e.GetSize().GetHeight();
    if (externalVTKWidget)
    {
        externalVTKWidget->GetRenderWindow()->SetSize(w, h);
    }
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

        double rxf = dx * delta_azimuth * 20.0;
        double ryf = dy * delta_elevation * -20.0;

        const int w = externalVTKWidget->GetRenderWindow()->GetSize()[0];
        const int h = externalVTKWidget->GetRenderWindow()->GetSize()[1];

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

void wxGLAreaWidget::OnAddGeomBody(const GLGUID &partGuid, const int geomShape)
{
    vtkSmartPointer<IVtkTools_ShapeDataSource> aDS = vtkSmartPointer<IVtkTools_ShapeDataSource>::New();
    if (kPGS_SPHERE == geomShape)
    {
        const auto aShape = BRepPrimAPI_MakeBox(60, 80, 90).Shape();
        IVtkOCC_Shape::Handle aShapeImpl = new IVtkOCC_Shape(aShape);
        aDS->SetShape(aShapeImpl);
    }
    else
    {
        const auto aShape = BRepPrimAPI_MakeSphere(30.0, vtkMath::RadiansFromDegrees(30.0)).Shape();
        IVtkOCC_Shape::Handle aShapeImpl = new IVtkOCC_Shape(aShape);
        aDS->SetShape(aShapeImpl);
    }

    aDS->Update();

    SPDispNodes newDispNodes = GLDispNode::MakeNew(aDS->GetOutput(), rootRenderer);
    if (!newDispNodes.empty())
    {
        const auto numColors = colors_->GetNumberOfColors();
        for (const auto &newDispNode : newDispNodes)
        {
            newDispNode->SetCellColor(colors_->GetColor3d(colorNames_->GetValue((colorIndex_++) % numColors)).GetData());
            newDispNode->SetEdgeColor(colors_->GetColor3d("Black").GetData());
            newDispNode->SetNodeColor(colors_->GetColor3d("Blue").GetData());
            allActors_[newDispNode->GetGUID()] = newDispNode;
            modelTreeView_->AddGeomBody(partGuid, newDispNode);
        }

        rootRenderer->ResetCamera();
        Refresh(false);
    }
}

void wxGLAreaWidget::OnColorChanged(const std::vector<GLGUID> &guids, const std::vector<vtkColor4d> &newColors)
{
    bool needRefresh = false;
    if (guids.size() == newColors.size())
    {
        for (std::vector<GLGUID>::size_type n=0; n < guids.size(); ++n)
        {
            auto it = allActors_.find(guids[n]);
            if (it != allActors_.end())
            {
                it->second->SetCellColor(newColors[n].GetData());
                needRefresh = true;
            }
        }
    }

    if (needRefresh)
    {
        Refresh(false);
    }
}

void wxGLAreaWidget::OnVisibilityChanged(const std::vector<GLGUID> &guids, const std::vector<int> &visibles)
{
    bool needRefresh = false;
    if (guids.size() == visibles.size())
    {
        for (std::vector<GLGUID>::size_type n = 0; n < guids.size(); ++n)
        {
            auto it = allActors_.find(guids[n]);
            if (it != allActors_.end())
            {
                it->second->SetVisible(visibles[n]);
                needRefresh = true;
            }
        }
    }

    if (needRefresh)
    {
        Refresh(false);
    }
}

void wxGLAreaWidget::OnShowNodeChanged(const std::vector<GLGUID> &guids, const std::vector<int> &visibles)
{
    bool needRefresh = false;
    if (guids.size() == visibles.size())
    {
        for (std::vector<GLGUID>::size_type n = 0; n < guids.size(); ++n)
        {
            auto it = allActors_.find(guids[n]);
            if (it != allActors_.end())
            {
                it->second->ShowNode(visibles[n]);
                needRefresh = true;
            }
        }
    }

    if (needRefresh)
    {
        Refresh(false);
    }
}

void wxGLAreaWidget::OnRepresentationChanged(const std::vector<GLGUID> &guids, const std::vector<int> &reps)
{
    bool needRefresh = false;
    if (guids.size() == reps.size())
    {
        for (std::vector<GLGUID>::size_type n = 0; n < guids.size(); ++n)
        {
            auto it = allActors_.find(guids[n]);
            if (it != allActors_.end())
            {
                it->second->SetRepresentation(reps[n]);
                needRefresh = true;
            }
        }
    }

    if (needRefresh)
    {
        Refresh(false);
    }
}

void wxGLAreaWidget::OnEntitiesDeleted(const std::vector<GLGUID> &guids)
{
    bool needRefresh = false;
    for (const GLGUID &guid : guids)
    {
        auto it = allActors_.find(guid);
        if (it != allActors_.end())
        {
            needRefresh = true;
            allActors_.erase(it);
        }
    }

    if (needRefresh)
    {
        Refresh(false);
    }
}

void wxGLAreaWidget::OnImportModel(const GLGUID &parentGuid)
{
    wxString wildCard{ "Stereo lithography STL files (*.stl)|*.stl" };
    wildCard.Append("|VTK Formats (*.vtp;*.vtu)|*.vtp;*.vtu");
    wildCard.Append("|Legacy VTK Formats (*.vtk)|*.vtk");
    wildCard.Append("|Wavefront OBJ file (*.obj)|*.obj");
    wildCard.Append("|PLY files (*.ply)|*.ply");
    wildCard.Append("|All files (*.*)|*.*");

    wxFileDialog openFileDialog(this, wxT("Open model file"), "", "", wildCard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() != wxID_CANCEL)
    {
        auto fullPath = std::string(openFileDialog.GetPath());
        std::string extension = vtksys::SystemTools::GetFilenameLastExtension(fullPath);
        if (extension == ".ply")
        {
            this->ImportPLY(parentGuid, fullPath);
        }
        else if (extension == ".vtp" || extension == ".vtu")
        {
            this->ImportVTU(parentGuid, fullPath);
        }
        else if (extension == ".obj")
        {
            this->ImportOBJ(parentGuid, fullPath);
        }
        else if (extension == ".stl")
        {
            this->ImportSTL(parentGuid, fullPath);
        }
        else if (extension == ".vtk")
        {
            this->ImportVTK(parentGuid, fullPath);
        }
        else
        {
            wxMessageBox(wxString(wxT("Don't know how to read this file")), wxString(wxT("Import Failed")));
        }
    }
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

void wxGLAreaWidget::PositionAxis(const int oldx, const int oldy, const int newx, const int newy)
{
    double viewFocus[4], focalDepth, viewPoint[3];
    double newPickPoint[4], oldPickPoint[4], motionVector[3];

    const int h = externalVTKWidget->GetRenderWindow()->GetSize()[1];
    vtkCamera* camera = axisRenderer->GetActiveCamera();
    camera->GetFocalPoint(viewFocus);
    this->ComputeWorldToDisplay(axisRenderer, viewFocus[0], viewFocus[1], viewFocus[2], viewFocus);
    focalDepth = viewFocus[2];

    this->ComputeDisplayToWorld(axisRenderer, newx, newy, focalDepth, newPickPoint);
    this->ComputeDisplayToWorld(axisRenderer, oldx, oldy, focalDepth, oldPickPoint);

    motionVector[0] = oldPickPoint[0] - newPickPoint[0];
    motionVector[1] = oldPickPoint[1] - newPickPoint[1];
    motionVector[2] = oldPickPoint[2] - newPickPoint[2];

    camera->GetFocalPoint(viewFocus);
    camera->GetPosition(viewPoint);
    camera->SetFocalPoint(motionVector[0] + viewFocus[0], motionVector[1] + viewFocus[1], motionVector[2] + viewFocus[2]);
    camera->SetPosition(motionVector[0] + viewPoint[0], motionVector[1] + viewPoint[1], motionVector[2] + viewPoint[2]);
    axisRenderer->UpdateLightsGeometryToFollowCamera();
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

extern int add_BoxDemo(vtkRenderWindow *renWin, vtkRenderer *renderer);
extern int add_LinearCellDemo(vtkRenderWindow *renWin, vtkRenderer *renderer, const int index);

void wxGLAreaWidget::realize_cb(GtkWidget *widget, gpointer user_data)
{
    GdkGLContext *context = gtk_gl_area_get_context(GTK_GL_AREA(widget));
    if (gdk_gl_context_get_use_es(context) || gdk_gl_context_is_legacy(context))
    {
        return;
    }

    gtk_gl_area_make_current(GTK_GL_AREA(widget));

    if (gtk_gl_area_get_error(GTK_GL_AREA(widget)) != NULL)
        return;

    if (gdk_gl_context_get_debug_enabled(context))
    {
        wxLogMessage(wxString("GDK GL Context Debug Enabled"));
    }
    else
    {
        wxLogMessage(wxString("GDK GL Context Debug Disabled"));
    }

    wxGLAreaWidget *glArea = reinterpret_cast<wxGLAreaWidget *>(user_data);
    glArea->externalVTKWidget = vtkSmartPointer<ExternalVTKWidget>::New();
    auto renWin = glArea->externalVTKWidget->GetRenderWindow();
    const int w = gtk_widget_get_allocated_width(widget);
    const int h = gtk_widget_get_allocated_height(widget);
    glArea->externalVTKWidget->GetRenderWindow()->SetSize(w, h);

    renWin->AutomaticWindowPositionAndResizeOff();
    renWin->SetUseExternalContent(false);
    renWin->SetSupportsOpenGL(true);
    renWin->SetIsDirect(true);

    vtkNew<vtkCallbackCommand> mcCallback, icCallback;
    mcCallback->SetClientData(widget);
    mcCallback->SetCallback(MakeCurrentCallback);
    renWin->AddObserver(vtkCommand::WindowMakeCurrentEvent, mcCallback);
    icCallback->SetClientData(widget);
    icCallback->SetCallback(IsCurrentCallback);
    renWin->AddObserver(vtkCommand::WindowIsCurrentEvent, icCallback);
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
    sphereSource->SetRadius(0.15);

    // create a mapper
    vtkNew<vtkPolyDataMapper> sphereMapper;
    sphereMapper->SetInputConnection(sphereSource->GetOutputPort());

    // create an actor
    vtkNew<vtkActor> sphereActor;
    sphereActor->VisibilityOn();
    sphereActor->SetMapper(sphereMapper);

    // a renderer and render window
    glArea->axisRenderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
    glArea->axisRenderer->GetActiveCamera()->ParallelProjectionOn();
    renWin->AddRenderer(glArea->axisRenderer);
    glArea->axisRenderer->SetLayer(2);

    // add the actors to the scene
    glArea->axisRenderer->AddActor(sphereActor);
    glArea->axisRenderer->SetBackground(colors->GetColor3d("BkgColor").GetData());

    vtkNew<vtkAxesActor> axes;
    axes->VisibilityOn();
    axes->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(12);
    axes->GetYAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(12);
    axes->GetZAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(12);
    axes->GetXAxisCaptionActor2D()->GetTextActor()->SetTextScaleModeToNone();
    axes->GetYAxisCaptionActor2D()->GetTextActor()->SetTextScaleModeToNone();
    axes->GetZAxisCaptionActor2D()->GetTextActor()->SetTextScaleModeToNone();
    axes->SetAxisLabels(true);
    glArea->axisRenderer->AddActor(axes);
    glArea->axisRenderer->ResetCamera();
    glArea->axisRenderer->GetActiveCamera()->Azimuth(45);
    glArea->axisRenderer->GetActiveCamera()->Elevation(-30);
    glArea->axisRenderer->GetActiveCamera()->Zoom(1.5);
    glArea->axisRenderer->SetViewport(0.0, 0.0, 80.0/w, 80.0/h);

    glArea->rootRenderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
    glArea->rootRenderer->ResetCamera(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glArea->rootRenderer->GetActiveCamera()->ParallelProjectionOn();
    glArea->rootRenderer->GetActiveCamera()->Azimuth(45);
    glArea->rootRenderer->GetActiveCamera()->Elevation(-30);
    renWin->AddRenderer(glArea->rootRenderer);
    glArea->rootRenderer->SetLayer(1);

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

    int flags = 0; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        wxLogMessage(wxString("OpenGL Debug Context Enabled"));
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
    }
    else
    {
        wxLogMessage(wxString("OpenGL Debug Context Disabled"));
    }
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

GdkGLContext* wxGLAreaWidget::create_context_cb(GtkGLArea* self, gpointer user_data)
{
    GError *error = nullptr;
    GdkGLContext *context = gdk_window_create_gl_context(gtk_widget_get_window(GTK_WIDGET(self)), &error);
    if (error)
    {
        gtk_gl_area_set_error(self, error);
        g_clear_object(&context);
        g_clear_error(&error);
        return nullptr;
    }

#ifdef __WXDEBUG__
    gdk_gl_context_set_debug_enabled(context, TRUE);
#endif

    gdk_gl_context_set_use_es(context, FALSE);
    gdk_gl_context_set_required_version(context, 4, 5);

    gdk_gl_context_realize(context, &error);
    if (error)
    {
        gtk_gl_area_set_error(self, error);
        g_clear_object(&context);
        g_clear_error(&error);
        return nullptr;
    }

    return context;
}

gboolean wxGLAreaWidget::render_cb(GtkGLArea *area, GdkGLContext *context, gpointer user_data)
{
    wxGLAreaWidget *glArea = reinterpret_cast<wxGLAreaWidget *>(user_data);
    glArea->externalVTKWidget->GetRenderWindow()->Render();
    return TRUE;
}
