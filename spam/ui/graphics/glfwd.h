#ifndef SPAM_UI_GRAPHICS_GL_FORWARD_H
#define SPAM_UI_GRAPHICS_GL_FORWARD_H
#include <memory>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <gtk/gtk.h>

typedef unsigned int GLuint;
typedef unsigned int GLenum;
class GLShader;
class GLProgram;
class GLCamera;
class GLTexture;
class GLDispNode;
class GLDispTree;
class GLModelTreeView;

enum class DisplayEntityType
{
    kDET_VERTS = 0,
    kDET_LINES = 1,
    kDET_POLYS = 2,
    kDET_STRIPS = 3,
    kDET_NUMCATS = 4
};

struct GLGUID
{
    const guint64 part1;
    const guint64 part2;

    GLGUID(const guint64 pat1, const guint64 pat2) : part1(pat1), part2(pat2) {}

    static const GLGUID MakeNew()
    {
        const boost::uuids::uuid uuid = boost::uuids::random_generator()();
        return { *reinterpret_cast<const guint64*>(uuid.begin()), *reinterpret_cast<const guint64*>(uuid.begin()+8) };
    }
};

using SPGLShader            = std::shared_ptr<GLShader>;
using SPGLProgram           = std::shared_ptr<GLProgram>;
using SPGLCamera            = std::shared_ptr<GLCamera>;
using SPGLTexture           = std::shared_ptr<GLTexture>;
using SPDispNode            = std::shared_ptr<GLDispNode>;
using SPDispTree            = std::shared_ptr<GLDispTree>;
using SPGLModelTreeView     = std::shared_ptr<GLModelTreeView>;
using WPGLShader            = std::weak_ptr<GLShader>;
using WPGLProgram           = std::weak_ptr<GLProgram>;
using WPGLCamera            = std::weak_ptr<GLCamera>;
using WPGLTexture           = std::weak_ptr<GLTexture>;
using WPDispNode            = std::weak_ptr<GLDispNode>;
using WPDispTree            = std::weak_ptr<GLDispTree>;
using WPGLModelTreeView     = std::weak_ptr<GLModelTreeView>;
using SPDispNodes           = std::vector<SPDispNode>;
using GLGUIDS               = std::vector<GLGUID>;

inline bool operator<(const GLGUID &v1, const GLGUID &v2)
{
    if (v1.part1 == v2.part1)
    {
        return v1.part2 < v2.part2;
    }
    else
    {
        return v1.part1 < v2.part1;
    }
}

#endif // SPAM_UI_GRAPHICS_GL_FORWARD_H
