#ifndef SPAM_UI_GRAPHICS_GL_MODEL_TREE_VIEW_H
#define SPAM_UI_GRAPHICS_GL_MODEL_TREE_VIEW_H
#include "glfwd.h"
#include <boost/signals2.hpp>
#include <vtkNamedColors.h>
namespace bs2 = boost::signals2;
class wxWindow;

class GLModelTreeView
{
    enum
    {
        ENTITY_NAME = 0,
        ENTITY_ICON,
        ENTITY_VISIBILITY,
        ENTITY_DISPLAY_MODE,
        ENTITY_SHOW_VERTEX,
        ENTITY_COLOR,
        ENTITY_TYPE,
        ENTITY_GUID_PART_1,
        ENTITY_GUID_PART_2,

        NUM_COLUMNS
    };

    enum
    {
        ENTITY_TYPE_MODEL,
        ENTITY_TYPE_ASSEMBLY,
        ENTITY_TYPE_PART,
        ENTITY_TYPE_SOLID_BODY,
        NUM_ENTITY_TYPES
    };

    struct this_is_private;

public:
    typedef bs2::keywords::mutex_type<bs2::dummy_mutex> bs2_dummy_mutex;
    bs2::signal_type<void(const std::vector<GLGUID>&, const std::vector<vtkColor4d>&), bs2_dummy_mutex>::type sig_ColorChanged;
    bs2::signal_type<void(const std::vector<GLGUID>&, const std::vector<int>&), bs2_dummy_mutex>::type sig_VisibilityChanged;
    bs2::signal_type<void(const std::vector<GLGUID>&, const std::vector<int>&), bs2_dummy_mutex>::type sig_ShowNodeChanged;
    bs2::signal_type<void(const std::vector<GLGUID>&, const std::vector<int>&), bs2_dummy_mutex>::type sig_RepresentationChanged;
    bs2::signal_type<void(const GLGUID, const int), bs2_dummy_mutex>::type sig_AddGeomBody;

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
    void CloseModel();
    void AddPart(const std::string &partName, const SPDispNodes &dispNodes);
    void AddGeomBody(const GLGUID &partGUID, const SPDispNode &spGeomBody);

private:
    gboolean FindPart(const GLGUID &partGUID, GtkTreeIter *iter);

private:
    static bool color_eq(const GdkRGBA *c1, const GdkRGBA *c2);
    static void on_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
    static void on_color_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
    static void on_visibility_toggled(GtkCellRendererToggle *celltoggle, gchar *path_string, gpointer data);
    static void on_node_toggled(GtkCellRendererToggle *celltoggle, gchar *path_string, gpointer data);
    static void on_representation_changed(GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, gpointer data);
    static void on_add_part(GtkWidget *menuitem, gpointer userdata);
    static void on_add_assembly(GtkWidget *menuitem, gpointer userdata);
    static void on_add_geom_body(GtkWidget *menuitem, gpointer userdata);
    static void on_add_vtk_cell(GtkWidget *menuitem, gpointer userdata);
    static gboolean on_popup_menu(GtkWidget *treeview, gpointer userdata);
    static gboolean on_button_pressed(GtkWidget *treeview, GdkEventButton *e, gpointer userdata);

private:
    static void view_popup_menu(GtkWidget *treeview, GdkEventButton *e, gpointer userdata);
    static void set_children_visibility(GtkTreeModel* model, GtkTreeIter* iterParent, const gboolean visible, std::vector<GLGUID> &guids);
    static void set_children_node(GtkTreeModel* model, GtkTreeIter* iterParent, const gboolean visible, std::vector<GLGUID> &guids);
    static gboolean TreeModelForeachFunc(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter, gpointer data);

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
    GdkPixbuf *assemPixBuf = nullptr;
    GdkPixbuf * partPixBuf = nullptr;
    GdkPixbuf * bodyPixBuf = nullptr;
};

#endif // SPAM_UI_GRAPHICS_GL_MODEL_TREE_VIEW_H
