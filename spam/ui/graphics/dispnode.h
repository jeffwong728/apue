#ifndef SPAM_UI_GRAPHICS_GL_DISP_NODE_H
#define SPAM_UI_GRAPHICS_GL_DISP_NODE_H
#include "glfwd.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class GLDispNode
{
    struct this_is_private;
public:
    static SPDispNode MakeNew(const WPDispNode &parent, const std::vector<glm::vec3> &vPositions, const std::vector<GLuint> &faceIdxs, const std::vector<GLuint> &edgeIdxs);

public:
    explicit GLDispNode(const this_is_private&, const WPDispNode &parent, const GLuint VBO, const GLuint VAO, const GLuint EBO);
    ~GLDispNode();

public:
    GLDispNode(const GLDispNode &) = delete;
    GLDispNode &operator=(const GLDispNode &) = delete;

public:
    void RenderMe() const;

public:
    WPDispNode GetParent() { return parent_; }
    const WPDispNode GetParent() const { return parent_; }

private:
    struct this_is_private 
    {
        explicit this_is_private(int) {}
    };

private:
    WPDispNode parent_;
    SPDispNodes children_;
    SPGLCamera camera_;
    SPGLTexture texture_;
    glm::vec4 faceColor_;
    glm::vec4 edgeColor_;
    glm::vec4 vertColor_;
    const GLuint VAO_ = 0;
    const GLuint VBO_ = 0;
    const GLuint EBO_ = 0;
};

#endif // SPAM_UI_GRAPHICS_GL_DISP_NODE_H
