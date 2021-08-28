#ifndef SPAM_UI_GRAPHICS_GL_TEXTURE_H
#define SPAM_UI_GRAPHICS_GL_TEXTURE_H
#include "glfwd.h"

class GLTexture
{
public:
    static SPGLTexture MakeNew(const boost::filesystem::path &textureFile);

public:
    struct this_is_private;
    explicit GLTexture(const this_is_private&, const GLuint textureId);
    ~GLTexture();

public:
    GLTexture(const GLTexture &) = delete;
    GLTexture &operator=(const GLTexture &) = delete;

public:
    const GLuint ID() const { return ID_; }

private:
    struct this_is_private
    {
        explicit this_is_private(int) {}
    };

private:
    GLuint ID_ = 0;
};

#endif // SPAM_UI_GRAPHICS_GL_TEXTURE_H
