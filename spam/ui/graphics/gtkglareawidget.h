#ifndef _WX_GTK_GL_AREA_WIDGET_H_
#define _WX_GTK_GL_AREA_WIDGET_H_

#include "glfwd.h"
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
#include <vtkNamedColors.h>
#include <vtkDataSet.h>
class ExternalVTKWidget;
class vtkExternalOpenGLCamera;
class vtkOpenGLRenderer;
class vtkRenderer;
class vtkActor;
class vtkActor2D;
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
    void ImportSTL(const GLGUID &parentGuid, const std::string &inputFilename);
    void ImportVTK(const GLGUID &parentGuid, const std::string &inputFilename);
    void ImportVTU(const GLGUID &parentGuid, const std::string &inputFilename);
    void ImportOBJ(const GLGUID &parentGuid, const std::string &inputFilename);
    void ImportPLY(const GLGUID &parentGuid, const std::string &inputFilename);
    void CloseModel();

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
    void OnAddGeomBody(const GLGUID &partGuid, const int geomShape);
    void OnColorChanged(const std::vector<GLGUID> &guids, const std::vector<vtkColor4d> &newColors);
    void OnVisibilityChanged(const std::vector<GLGUID> &guids, const std::vector<int> &visibles);
    void OnShowNodeChanged(const std::vector<GLGUID> &guids, const std::vector<int> &visibles);
    void OnRepresentationChanged(const std::vector<GLGUID> &guids, const std::vector<int> &reps);
    void OnEntitiesDeleted(const std::vector<GLGUID> &guids);
    void OnHighlightChanged(const std::vector<GLGUID> &oldGuids, const std::vector<GLGUID> &newGuids);
    void OnImportModel(const GLGUID &parentGuid);
    void OnExportBody(const GLGUID &parentGuid);

protected:
    virtual void DoApplyWidgetStyle(GtkRcStyle *style) wxOVERRIDE;
    virtual wxSize DoGetBestSize() const wxOVERRIDE;

private:
    void PositionAxis(const int oldx, const int oldy, const int newx, const int newy);
    void DrawRubberBox(const wxPoint &cPos);
    void RemoveApexArrays(vtkDataSet *ds);

private:
    static void ComputeWorldToDisplay(vtkRenderer* ren, double x, double y, double z, double displayPt[3]);
    static void ComputeDisplayToWorld(vtkRenderer* ren, double x, double y, double z, double worldPt[4]);
    static void realize_cb(GtkWidget *widget, gpointer user_data);
    static void unrealize_cb(GtkWidget *widget, gpointer user_data);
    static GdkGLContext* create_context_cb( GtkGLArea* self, gpointer user_data);
    static gboolean render_cb(GtkGLArea *area, GdkGLContext *context, gpointer user_data);

private:
    typedef wxControl base_type;

private:
    wxPoint anchorPos_;
    wxPoint lastPos_;
    vtkSmartPointer<ExternalVTKWidget> externalVTKWidget;
    vtkSmartPointer<vtkOpenGLRenderer> rootRenderer;
    vtkSmartPointer<vtkOpenGLRenderer> axisRenderer;
    vtkSmartPointer<vtkOpenGLRenderer> rubberBoxRenderer;
    vtkSmartPointer<vtkActor2D> rubberBoxActor;
    std::map<GLGUID, SPDispNode> allActors_;
    SPGLModelTreeView modelTreeView_;
    vtkNew<vtkNamedColors> colors_;
    vtkNew<vtkStringArray> colorNames_;
    vtkIdType colorIndex_ = 0;
    wxString lastExportDir_;

    wxDECLARE_DYNAMIC_CLASS(wxGLAreaWidget);
};

#endif // _WX_GTK_GL_AREA_WIDGET_H_

