#ifndef SPAM_UI_GRAPHICS_GL_SHADER_H
#define SPAM_UI_GRAPHICS_GL_SHADER_H
#include "glfwd.h"
typedef unsigned int GLuint;
typedef unsigned int GLenum;

class GLShader
{
public:
    GLShader(const GLenum shaderType, const std::string &shaderCode);
    GLShader(const GLenum shaderType, const boost::filesystem::path &filePath);
    ~GLShader();

public:
    GLShader(const GLShader &) = delete;
    GLShader &operator=(const GLShader &) = delete;

public:
    const GLuint ID() const { return ID_; }

private:
    void ConstructMe();
    const bool CheckCompileErrors() const;

private:
    GLenum Type_;
    GLuint ID_;
    std::string shader_code_;
};

#endif // SPAM_UI_GRAPHICS_GL_SHADER_H
