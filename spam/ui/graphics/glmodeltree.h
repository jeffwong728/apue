#ifndef SPAM_UI_GRAPHICS_GL_MODEL_TREE_VIEW_H
#define SPAM_UI_GRAPHICS_GL_MODEL_TREE_VIEW_H
#include "glfwd.h"
#include <gtk/gtk.h>
class wxWindow;

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
    static SPGLModelTreeView MakeNew(const wxWindow *const parent);

public:
    explicit GLModelTreeView(const this_is_private&, const wxWindow *const parent);
    ~GLModelTreeView();

public:
    GLModelTreeView(const GLModelTreeView &) = delete;
    GLModelTreeView &operator=(const GLModelTreeView &) = delete;

public:
    GtkWidget *GetWidget() { return mainView_; }
    const GtkWidget *GetWidget() const { return mainView_; }

private:
    static bool color_eq(const GdkRGBA *c1, const GdkRGBA *c2);
    static void on_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
    static void on_color_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);

private:
    struct this_is_private
    {
        explicit this_is_private(int) {}
    };

private:
    const wxWindow *const parent_ = nullptr;
    GtkWidget *mainView_ = nullptr;
    GtkWidget *treeView_ = nullptr;
    GtkTreeStore *model_ = nullptr;
    std::string lastSelRowPath_;
};

#endif // SPAM_UI_GRAPHICS_GL_MODEL_TREE_VIEW_H
