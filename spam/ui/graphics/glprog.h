#ifndef SPAM_UI_GRAPHICS_GL_PROG_H
#define SPAM_UI_GRAPHICS_GL_PROG_H
#include "glshader.h"
#include <map>

class GLProgram
{
public:
    GLProgram();
    ~GLProgram();

public:
    GLProgram(const GLProgram &) = delete;
    GLProgram &operator=(const GLProgram &) = delete;

public:
    void Build();
    void Use();
    static void Reset();
    void AttachShaderFile(const GLenum shaderType, const boost::filesystem::path &filePath);

public:
    void SetUniform(const std::string &name, const glm::vec4 &val);
    void SetUniform(const std::string &name, const glm::mat4 &val);

private:
    void AttachShader(const GLShader &glShader);
    void DetachShader(const GLShader &glShader);
    const bool Link();
    const bool CheckLinkErrors() const;

private:
    GLuint ID_ = 0;
    std::map<GLenum, boost::filesystem::path> shader_files_;
};

#endif // SPAM_UI_GRAPHICS_GL_PROG_H
