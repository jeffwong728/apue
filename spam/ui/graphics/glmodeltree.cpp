#include "glmodeltree.h"
#include <wx/wxprec.h>
#include <wx/wx.h>
#include <vtkNew.h>
#include <vtkNamedColors.h>

SPGLModelTreeView GLModelTreeView::MakeNew(const wxWindow *const parent)
{
    return std::make_shared<GLModelTreeView>(this_is_private{ 0 }, parent);
}

GLModelTreeView::GLModelTreeView(const this_is_private&, const wxWindow *const parent)
    : treeView_(nullptr), model_(nullptr), mainView_(nullptr), parent_(parent)
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

bool GLModelTreeView::color_eq(const GdkRGBA *c1, const GdkRGBA *c2)
{
    return static_cast<int>(c1->red * 255) == static_cast<int>(c2->red * 255) &&
        static_cast<int>(c1->green * 255) == static_cast<int>(c2->green * 255) &&
        static_cast<int>(c1->blue * 255) == static_cast<int>(c2->blue * 255) &&
        static_cast<int>(c1->alpha * 255) == static_cast<int>(c2->alpha * 255);
}

void GLModelTreeView::on_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GLModelTreeView *myself = (GLModelTreeView *)user_data;

    GtkTreeIter iter;
    if (gtk_tree_model_get_iter(model, &iter, path))
    {
        auto tree_path_str  = gtk_tree_path_to_string(path);
        auto name = wxString(gtk_tree_view_column_get_title(column));
        auto rowPath = wxString(tree_path_str);
        myself->lastSelRowPath_.assign(tree_path_str);
        g_free(tree_path_str);

        wxLogMessage(rowPath.Append(wxT(", Column ")).Append(name).Append(wxT(" Clicked")));

        if (wxT("Color") == name)
        {
            GdkPixbuf *pixbuf = nullptr;
            gtk_tree_model_get(model, &iter, ENTITY_COLOR, &pixbuf, -1);
            if (pixbuf)
            {
                guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
                if (pixels)
                {
                    GdkRGBA color{ 0 };
                    color.red = pixels[0] / 255.0;
                    color.green = pixels[1] / 255.0;
                    color.blue = pixels[2] / 255.0;
                    color.alpha = pixels[3] / 255.0;
                    GtkWidget *dialog = gtk_color_chooser_dialog_new("Select a color", GTK_WINDOW(myself->parent_->GetParent()->m_widget));
                    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
                    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(dialog), TRUE);
                    g_object_set(dialog, "show-editor", FALSE, NULL);
                    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog), &color);
                    g_signal_connect(dialog, "response", G_CALLBACK(on_color_dialog_response), user_data);
                    gtk_widget_show_all(dialog);
                }
                g_object_unref(pixbuf);
            }
        }
    }
}

void GLModelTreeView::on_color_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
    if (response_id == GTK_RESPONSE_OK) {
        GdkRGBA color{1.0, 1.0, 1.0, 1.0 };
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &color);

        GtkTreeIter iter;
        GLModelTreeView *myself = (GLModelTreeView *)user_data;
        GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(myself->treeView_));
        if (gtk_tree_model_get_iter_from_string(model, &iter, myself->lastSelRowPath_.c_str()))
        {
            GdkPixbuf *pixbuf = nullptr;
            gtk_tree_model_get(model, &iter, ENTITY_COLOR, &pixbuf, -1);
            if (pixbuf)
            {
                guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
                if (pixels)
                {
                    GdkRGBA mycolor{ 0 };
                    mycolor.red = pixels[0] / 255.0;
                    mycolor.green = pixels[1] / 255.0;
                    mycolor.blue = pixels[2] / 255.0;
                    mycolor.alpha = pixels[3] / 255.0;

                    if (!color_eq(&color, &mycolor))
                    {
                        guint32 pixel = ((gint)(color.red * 255) << 24 | ((gint)(color.green*255)) << 16 | ((gint)(color.blue*255)) << 8 | ((gint)(color.alpha*255)));
                        gdk_pixbuf_fill(pixbuf, pixel);
                    }
                }

                g_object_unref(pixbuf);
            }
        }
    }

    gtk_widget_destroy(GTK_WIDGET(dialog));
}
