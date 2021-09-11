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
class vtkProp;

class GLDispNode
{
    struct this_is_private;
public:
    static SPDispNodes MakeNew(const vtkSmartPointer<vtkPolyData> &pdSource, const vtkSmartPointer<vtkOpenGLRenderer> &renderer);
    static SPDispNodes MakeNew(const vtkSmartPointer<vtkUnstructuredGrid> &ugSource, const vtkSmartPointer<vtkOpenGLRenderer> &renderer);

public:
    explicit GLDispNode(const this_is_private&, const DisplayEntityType type);
    ~GLDispNode();

public:
    GLDispNode(const GLDispNode &) = delete;
    GLDispNode &operator=(const GLDispNode &) = delete;

public:
    const GLGUID GetGUID() { return guid_; }
    const GLGUID GetGUID() const { return guid_; }

    const vtkColor4ub GetColor() const;
    const std::string GetName() const;
    void SetVisible(const int visible);
    void ShowNode(const int visible);
    void SetRepresentation(const int rep);
    void SetCellColor(const double *c);
    void SetEdgeColor(const double *c);
    void SetNodeColor(const double *c);

private:
    void SetDefaultDisplay();

private:
    struct this_is_private 
    {
        explicit this_is_private(int) {}
    };

private:
    const GLGUID guid_;
    const DisplayEntityType type_;
    vtkWeakPointer<vtkOpenGLRenderer> renderer_;
    vtkSmartPointer<vtkPolyData> poly_data_;
    vtkSmartPointer<vtkActor> actor_;
};

#endif // SPAM_UI_GRAPHICS_GL_DISP_NODE_H
