#ifndef SPAM_UI_GRAPHICS_GL_DISP_NODE_H
#define SPAM_UI_GRAPHICS_GL_DISP_NODE_H
#include "glfwd.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkPolyData.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkUnstructuredGrid.h>
#include <vtkOpenGLRenderer.h>
#include <vtkNamedColors.h>
#include <vtkFloatArray.h>
class vtkProp;
class vtkPlanes;

class GLDispNode
{
protected:
    struct this_is_private;
public:
    static SPDispNodes MakeNew(const vtkSmartPointer<vtkPolyData> &pdSource, const vtkSmartPointer<vtkOpenGLRenderer> &renderer);
    static SPDispNodes MakeNew(const vtkSmartPointer<vtkUnstructuredGrid> &ugSource, const vtkSmartPointer<vtkOpenGLRenderer> &renderer);

public:
    explicit GLDispNode(const this_is_private&, const GraphicsEntityType type);
    virtual ~GLDispNode();

public:
    GLDispNode(const GLDispNode &) = delete;
    GLDispNode &operator=(const GLDispNode &) = delete;

public:
    const GLGUID GetGUID() { return guid_; }
    const GLGUID GetGUID() const { return guid_; }
    const int GetEntityType() const { return type_; }

    const vtkColor4ub GetColor() const;
    const std::string GetName() const;
    void SetVisible(const int visible);
    virtual void ShowNode(const int visible);
    virtual void SetRepresentation(const int rep);
    virtual void SetCellColor(const double *c);
    virtual void SetEdgeColor(const double *c);
    virtual void SetNodeColor(const double *c);

public:
    virtual bool Select2DCells(vtkPlanes *frustum);
    virtual bool Select3DCells(vtkPlanes *frustum);
    virtual bool ExportVTK(const std::string &dir);

protected:
    virtual void SetDefaultDisplay() = 0;

protected:
    struct this_is_private 
    {
        explicit this_is_private(int) {}
    };

protected:
    const GLGUID guid_;
    const GraphicsEntityType type_;
    vtkWeakPointer<vtkOpenGLRenderer> renderer_;
    vtkSmartPointer<vtkPolyData> poly_data_;
    vtkSmartPointer<vtkActor> actor_;
    vtkSmartPointer<vtkPolyDataMapper> mapper_;
    int representation_{ kGREP_VTK_SURFACE };
};

#endif // SPAM_UI_GRAPHICS_GL_DISP_NODE_H
