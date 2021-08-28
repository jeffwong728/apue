#include "glprog.h"
#include <wx/log.h>
#include <epoxy/gl.h>

GLProgram::GLProgram()
{
    ID_ = glCreateProgram();
}

GLProgram::~GLProgram()
{
    glDeleteProgram(ID_);
}

void GLProgram::Build()
{
    std::vector<std::unique_ptr<GLShader>> shaders;
    for (const auto &fPaths : shader_files_)
    {
        shaders.push_back(std::make_unique<GLShader>(fPaths.first, fPaths.second));
    }

    for (const std::unique_ptr<GLShader> &shader : shaders)
    {
        AttachShader(*shader);
    }

    Link();

    for (const std::unique_ptr<GLShader> &shader : shaders)
    {
        DetachShader(*shader);
    }
}

void GLProgram::Use()
{
    if (glIsProgram(ID_))
    {
        glUseProgram(ID_);
    }
}

void GLProgram::Reset()
{
    glUseProgram(0);
}

void GLProgram::AttachShaderFile(const GLenum shaderType, const boost::filesystem::path &filePath)
{
    shader_files_[shaderType] = filePath;
}

void GLProgram::SetUniform(const std::string &name, const glm::vec4 &val)
{
    const GLuint uLoc = glGetUniformLocation(ID_, name.c_str());
    glUniform4fv(uLoc, 1, glm::value_ptr(val));
}

void GLProgram::SetUniform(const std::string &name, const glm::mat4 &val)
{
    const GLuint uLoc = glGetUniformLocation(ID_, name.c_str());
    glUniformMatrix4fv(uLoc, 1, GL_FALSE, glm::value_ptr(val));
}

const bool GLProgram::CheckLinkErrors() const
{
    if (glIsProgram(ID_))
    {
        GLint success = GL_FALSE;
        glGetProgramiv(ID_, GL_LINK_STATUS, &success);
        if (GL_FALSE == success)
        {
            wxLogMessage(wxT("Shader linking error:"));
            GLint logLen = 0;
            glGetProgramiv(ID_, GL_INFO_LOG_LENGTH, &logLen);
            if (logLen)
            {
                GLsizei length = 0;
                std::vector<char> strLog;
                strLog.resize(logLen + 1);
                glGetProgramInfoLog(ID_, logLen, &length, strLog.data());
                wxLogMessage(wxString(strLog.data()));
            }
        }
        else
        {
            return true;
        }
    }

    return false;
}

void GLProgram::AttachShader(const GLShader &glShader)
{
    if (glIsProgram(ID_) && glIsShader(glShader.ID()))
    {
        glAttachShader(ID_, glShader.ID());
    }
}

void GLProgram::DetachShader(const GLShader &glShader)
{
    if (glIsProgram(ID_) && glIsShader(glShader.ID()))
    {
        glAttachShader(ID_, glShader.ID());
    }
}

const bool GLProgram::Link()
{
    if (glIsProgram(ID_))
    {
        glLinkProgram(ID_);
        if (CheckLinkErrors())
        {
            return true;
        }
        else
        {
            glDeleteProgram(ID_);
            ID_ = 0;
        }
    }

    return false;
}
