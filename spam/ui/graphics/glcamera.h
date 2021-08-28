#ifndef SPAM_UI_GRAPHICS_GL_CAMERA_H
#define SPAM_UI_GRAPHICS_GL_CAMERA_H
#include "glfwd.h"

class GLCamera
{
    struct this_is_private;
public:
    static SPGLCamera MakeNew();

public:
    explicit GLCamera(const this_is_private&);
    ~GLCamera();

public:
    GLCamera(const GLCamera &) = delete;
    GLCamera &operator=(const GLCamera &) = delete;

public:
    const glm::mat4 GetModelView() const { return modelview_; }
    const glm::mat4 GetProjection() const { return projection_; }

private:
    struct this_is_private
    {
        explicit this_is_private(int) {}
    };

private:
    glm::mat4 modelview_;
    glm::mat4 projection_;
};

#endif // SPAM_UI_GRAPHICS_GL_CAMERA_H
