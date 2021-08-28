#include "glshader.h"
#include <wx/log.h>
#include <epoxy/gl.h>
#include <fstream>
#include <sstream>

GLShader::GLShader(const GLenum shaderType, const std::string &shaderCode)
    : Type_(shaderType), ID_(0), shader_code_(shaderCode)
{
    ConstructMe();
}

GLShader::GLShader(const GLenum shaderType, const boost::filesystem::path &filePath)
    : Type_(shaderType), ID_(0), shader_code_()
{
    boost::system::error_code ec;
    if (boost::filesystem::exists(filePath, ec) && boost::filesystem::is_regular_file(filePath, ec))
    {
        std::ifstream shaderFile;
        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            shaderFile.open(std::string(wxString(filePath.native())));
            std::stringstream shaderStream;
            shaderStream << shaderFile.rdbuf();
            shaderFile.close();
            shader_code_ = shaderStream.str();
            ConstructMe();
        }
        catch (std::ifstream::failure& e)
        {
            wxLogMessage(wxString(wxT("Open shader file error: ")).Append(wxString(e.what())));
        }
    }
}

void GLShader::ConstructMe()
{
    if (!shader_code_.empty())
    {
        const char* pShaderCode = shader_code_.c_str();
        ID_ = glCreateShader(Type_);
        glShaderSource(ID_, 1, &pShaderCode, NULL);
        glCompileShader(ID_);
        if (!CheckCompileErrors())
        {
            glDeleteShader(ID_);
            ID_ = 0;
        }
    }
}

GLShader::~GLShader()
{
    glDeleteShader(ID_);
}

const bool GLShader::CheckCompileErrors() const
{
    if (glIsShader(ID_))
    {
        GLint success = GL_FALSE;
        glGetShaderiv(ID_, GL_COMPILE_STATUS, &success);
        if (GL_FALSE == success)
        {
            wxLogMessage(wxT("Shader compiling error:"));
            GLint logLen = 0;
            glGetShaderiv(ID_, GL_INFO_LOG_LENGTH, &logLen);
            if (logLen)
            {
                GLsizei length = 0;
                std::vector<char> strLog;
                strLog.resize(logLen + 1);
                glGetShaderInfoLog(ID_, logLen, &length, strLog.data());
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
