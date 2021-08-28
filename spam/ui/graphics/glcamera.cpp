#include "glcamera.h"

SPGLCamera GLCamera::MakeNew()
{
    return std::make_shared<GLCamera>(this_is_private{ 0 });
}

GLCamera::GLCamera(const this_is_private&)
{
    glm::mat4 normView = glm::mat4(1.f);
    normView = glm::rotate(normView, glm::radians(45.f), glm::vec3(-1.0f, 0.0f, 0.0f));
    normView = glm::rotate(normView, glm::radians(45.f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelview_ = normView;
    projection_ = glm::mat4(1.0f);
}

GLCamera::~GLCamera()
{
}
