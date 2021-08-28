#ifndef SPAM_UI_GRAPHICS_GL_FORWARD_H
#define SPAM_UI_GRAPHICS_GL_FORWARD_H
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/filesystem.hpp>

typedef unsigned int GLuint;
typedef unsigned int GLenum;
class GLShader;
class GLProgram;
class GLCamera;
class GLTexture;
class GLDispNode;
class GLDispTree;
using SPGLShader  = std::shared_ptr<GLShader>;
using SPGLProgram = std::shared_ptr<GLProgram>;
using SPGLCamera  = std::shared_ptr<GLCamera>;
using SPGLTexture = std::shared_ptr<GLTexture>;
using SPDispNode  = std::shared_ptr<GLDispNode>;
using SPDispTree  = std::shared_ptr<GLDispTree>;
using WPGLShader  = std::weak_ptr<GLShader>;
using WPGLProgram = std::weak_ptr<GLProgram>;
using WPGLCamera  = std::weak_ptr<GLCamera>;
using WPGLTexture = std::weak_ptr<GLTexture>;
using WPDispNode  = std::weak_ptr<GLDispNode>;
using WPDispTree  = std::weak_ptr<GLDispTree>;
using SPDispNodes = std::vector<SPDispNode>;

#endif // SPAM_UI_GRAPHICS_GL_FORWARD_H
