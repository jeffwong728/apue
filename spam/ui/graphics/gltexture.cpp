#include "gltexture.h"
#include <wx/wxprec.h>
#include <wx/wx.h>
#include <opencv2/opencv.hpp>
#include <epoxy/gl.h>

SPGLTexture GLTexture::MakeNew(const boost::filesystem::path &textureFile)
{
    boost::system::error_code ec;
    if (boost::filesystem::exists(textureFile, ec) && boost::filesystem::is_regular_file(textureFile, ec))
    {
        cv::Mat srcMat = cv::imread(cv::String(wxString(textureFile.native())), cv::IMREAD_COLOR), fImg, bkImg;
        if (!srcMat.empty() && CV_8U == srcMat.depth())
        {
            cv::cvtColor(srcMat, fImg, cv::COLOR_BGR2RGB);
            cv::flip(fImg, bkImg, 0);

            GLuint textureId = 0;
            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bkImg.cols, bkImg.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, bkImg.data);

            return std::make_shared<GLTexture>(this_is_private{ 0 }, textureId);
        }
    }

    return SPGLTexture();
}

GLTexture::GLTexture(const this_is_private&, const GLuint textureId)
    : ID_(textureId)
{
}

GLTexture::~GLTexture()
{
    glDeleteTextures(1, &ID_);
}
