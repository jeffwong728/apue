#ifndef SPAM_UI_GRAPHICS_GL_MODEL_TREE_VIEW_H
#define SPAM_UI_GRAPHICS_GL_MODEL_TREE_VIEW_H
#include "glfwd.h"
#include <gtk/gtk.h>

class GLModelTreeView
{
    enum
    {
        ENTITY_NAME = 0,
        ENTITY_VISIBILITY,
        ENTITY_DISPLAY_MODE,
        ENTITY_COLOR,

        NUM_COLUMNS
    };

    struct this_is_private;
public:
    static SPGLModelTreeView MakeNew();

public:
    explicit GLModelTreeView(const this_is_private&);
    ~GLModelTreeView();

public:
    GLModelTreeView(const GLModelTreeView &) = delete;
    GLModelTreeView &operator=(const GLModelTreeView &) = delete;

public:
    GtkWidget *GetWidget() { return mainView_; }
    const GtkWidget *GetWidget() const { return mainView_; }

private:
    struct this_is_private
    {
        explicit this_is_private(int) {}
    };

private:
    GtkWidget *mainView_ = nullptr;
    GtkWidget *treeView_ = nullptr;
    GtkTreeStore *model_ = nullptr;
};

#endif // SPAM_UI_GRAPHICS_GL_MODEL_TREE_VIEW_H
