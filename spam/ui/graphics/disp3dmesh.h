#ifndef SPAM_UI_GRAPHICS_GL_3D_MESH_NODE_H
#define SPAM_UI_GRAPHICS_GL_3D_MESH_NODE_H
#include "dispnode.h"

class GL3DMeshNode : public GLDispNode
{
    struct this_is_private;
public:
    static SPDispNodes MakeNew(const vtkSmartPointer<vtkPolyData> &pdSource, const vtkSmartPointer<vtkOpenGLRenderer> &renderer);
    static SPDispNodes MakeNew(const vtkSmartPointer<vtkUnstructuredGrid> &ugSource, const vtkSmartPointer<vtkOpenGLRenderer> &renderer);

public:
    explicit GL3DMeshNode(const this_is_private&);
    ~GL3DMeshNode();

public:
    GL3DMeshNode(const GL3DMeshNode &) = delete;
    GL3DMeshNode &operator=(const GL3DMeshNode &) = delete;

public:
    const vtkColor4ub GetColor() const;
    const std::string GetName() const;
    void SetVisible(const int visible);
    void ShowNode(const int visible) override;
    void SetRepresentation(const int rep) override;
    void SetCellColor(const double *c) override;
    void SetEdgeColor(const double *c) override;
    void SetNodeColor(const double *c) override;

protected:
    void SetDefaultDisplay() override;

protected:
    struct this_is_private 
    {
        explicit this_is_private(int) {}
    };
};

#endif // SPAM_UI_GRAPHICS_GL_3D_MESH_NODE_H
