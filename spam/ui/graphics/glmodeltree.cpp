#include "glmodeltree.h"
#include <wx/wxprec.h>
#include <wx/wx.h>
#include <vtkNew.h>
#include <vtkNamedColors.h>

SPGLModelTreeView GLModelTreeView::MakeNew()
{
    return std::make_shared<GLModelTreeView>(this_is_private{ 0 });
}

GLModelTreeView::GLModelTreeView(const this_is_private&)
    : treeView_(nullptr), model_(nullptr), mainView_(nullptr)
{
    GtkTreeIter iter, child_iter;
    model_ = gtk_tree_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING, GDK_TYPE_PIXBUF);

    gtk_tree_store_append(model_, &iter, NULL);
    gtk_tree_store_set(model_, &iter, ENTITY_NAME, "part1", -1);

    vtkNew<vtkNamedColors> colors;
    vtkNew<vtkStringArray> colorNames;
    colors->GetColorNames(colorNames);
    const int numColors = colors->GetNumberOfColors();

    for (int i=1; i<=50; ++i)
    {
        std::string strName("body");
        strName += std::to_string(i);

        vtkColor4ub color = colors->GetColor4ub(colorNames->GetValue(i%numColors));
        guint32 pixel = ((gint)(color.GetRed()) << 24 | ((gint)(color.GetGreen())) << 16 | ((gint)(color.GetBlue())) << 8 | ((gint)(color.GetAlpha())));
        GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 13, 13);
        gdk_pixbuf_fill(pixbuf, pixel);
        gtk_tree_store_append(model_, &child_iter, &iter);
        gtk_tree_store_set(model_, &child_iter, ENTITY_NAME, strName.c_str(), ENTITY_VISIBILITY, FALSE, ENTITY_DISPLAY_MODE, "wireframe", ENTITY_COLOR, pixbuf, -1);
        g_object_unref(pixbuf);
    }

    treeView_ = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model_));
    gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(treeView_), TRUE);
    gtk_tree_view_set_enable_tree_lines(GTK_TREE_VIEW(treeView_), TRUE);
    gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(treeView_), FALSE);
    gtk_widget_set_opacity(treeView_, 1.0);
    g_object_unref(model_);
    model_ = nullptr;

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeView_), -1, "Name", renderer, "text", ENTITY_NAME, NULL);

    renderer = gtk_cell_renderer_toggle_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeView_), -1, "Visibility", renderer, "active", ENTITY_VISIBILITY, NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeView_), -1, "Display Mode", renderer, "text", ENTITY_DISPLAY_MODE, NULL);

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeView_), -1, "Color", renderer, "pixbuf", ENTITY_COLOR, NULL);

    mainView_ = gtk_scrolled_window_new(NULL, NULL);
    gtk_window_set_resizable(GTK_WINDOW(mainView_), TRUE);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(mainView_), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(mainView_), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(mainView_), treeView_);
    gtk_widget_set_size_request(mainView_, 350, 200);

    g_signal_connect(treeView_, "row-activated", G_CALLBACK(on_row_activated), this);
}

GLModelTreeView::~GLModelTreeView()
{
}

void GLModelTreeView::on_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);

    GtkTreeIter iter;
    if (gtk_tree_model_get_iter(model, &iter, path))
    {
        auto tree_path_str  = gtk_tree_path_to_string(path);
        auto name = wxString(gtk_tree_view_column_get_title(column));
        auto rowPath = wxString(tree_path_str);
        g_free(tree_path_str);

        wxLogMessage(rowPath.Append(wxT(", Column ")).Append(name).Append(wxT(" Clicked")));
    }
}
