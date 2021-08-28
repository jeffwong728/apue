#ifndef SPAM_UI_GRAPHICS_GL_DISP_TREE_H
#define SPAM_UI_GRAPHICS_GL_DISP_TREE_H
#include "glfwd.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class GLDispTree
{
public:
    GLDispTree();
    ~GLDispTree();

public:
    GLDispTree(const GLDispTree &) = delete;
    GLDispTree &operator=(const GLDispTree &) = delete;

public:
    SPDispNode GetRoot() { return root_node_; }
    const SPDispNode GetRoot() const { return root_node_; }

private:
    SPDispNode root_node_;
};

#endif // SPAM_UI_GRAPHICS_GL_DISP_TREE_H
