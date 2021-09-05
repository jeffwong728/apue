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
    model_ = gtk_tree_store_new(NUM_COLUMNS, G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, G_TYPE_INT);

    vtkNew<vtkNamedColors> colors;
    vtkNew<vtkStringArray> colorNames;
    colors->GetColorNames(colorNames);
    const int numColors = colors->GetNumberOfColors();

    assemPixBuf = gdk_pixbuf_new_from_resource_at_scale("/org/mvlab/spam/res/model/assembly.svg", 16, 16, TRUE, nullptr);
    partPixBuf = gdk_pixbuf_new_from_resource_at_scale("/org/mvlab/spam/res/model/part.svg", 16, 16, TRUE, nullptr);
    bodyPixBuf = gdk_pixbuf_new_from_resource_at_scale("/org/mvlab/spam/res/model/solid.body.svg", 16, 16, TRUE, nullptr);

    GtkTreeIter itModel;
    gtk_tree_store_append(model_, &itModel, NULL);
    gtk_tree_store_set(model_, &itModel, ENTITY_NAME, "Model", ENTITY_DISPLAY_MODE, "Surface", ENTITY_TYPE, ENTITY_TYPE_MODEL, -1);

    for (int ass=0; ass<2; ++ass)
    {
        GtkTreeIter itAssem;
        std::string assemName("Assembly ");
        assemName += std::to_string(ass);
        gtk_tree_store_append(model_, &itAssem, &itModel);
        gtk_tree_store_set(model_, &itAssem, ENTITY_NAME, assemName.c_str(), ENTITY_ICON, assemPixBuf, ENTITY_DISPLAY_MODE, "Surface", ENTITY_TYPE, ENTITY_TYPE_ASSEMBLY, -1);
        for (int pat=0; pat<3; ++pat)
        {
            GtkTreeIter itPart;
            std::string partName("Part ");
            partName += std::to_string(pat);
            gtk_tree_store_append(model_, &itPart, &itAssem);
            gtk_tree_store_set(model_, &itPart, ENTITY_NAME, partName.c_str(), ENTITY_ICON, partPixBuf, ENTITY_DISPLAY_MODE, "Surface", ENTITY_TYPE, ENTITY_TYPE_PART, -1);
            for (int i = 1; i <= 5; ++i)
            {
                std::string bodyName("Body ");
                bodyName += std::to_string(i);

                vtkColor4ub color = colors->GetColor4ub(colorNames->GetValue(i%numColors));
                guint32 pixel = ((gint)(color.GetRed()) << 24 | ((gint)(color.GetGreen())) << 16 | ((gint)(color.GetBlue())) << 8 | ((gint)(color.GetAlpha())));
                GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 13, 13);
                gdk_pixbuf_fill(pixbuf, pixel);
                GtkTreeIter itBody;
                gtk_tree_store_append(model_, &itBody, &itPart);
                gtk_tree_store_set(model_, &itBody, ENTITY_NAME, bodyName.c_str(), ENTITY_ICON, bodyPixBuf, ENTITY_VISIBILITY, FALSE, ENTITY_DISPLAY_MODE, "Surface", ENTITY_COLOR, pixbuf, ENTITY_TYPE, ENTITY_TYPE_SOLID_BODY, -1);
                g_object_unref(pixbuf);
            }
        }
    }

    treeView_ = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model_));
    gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(treeView_), TRUE);
    gtk_tree_view_set_enable_tree_lines(GTK_TREE_VIEW(treeView_), FALSE);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(treeView_), GTK_TREE_VIEW_GRID_LINES_BOTH);
    gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(treeView_), FALSE);
    gtk_widget_set_opacity(treeView_, 1.0);
    g_object_unref(model_);
    model_ = nullptr;

    GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new();
    auto col_offset = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeView_), -1, "Name", renderer, "pixbuf", ENTITY_ICON, NULL);
    auto column = gtk_tree_view_get_column(GTK_TREE_VIEW(treeView_), col_offset - 1);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_add_attribute(column, renderer, "text", ENTITY_NAME);
    g_object_set(renderer, "editable", TRUE, NULL);

    renderer = gtk_cell_renderer_toggle_new();
    col_offset = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeView_), -1, "Visibility", renderer, "active", ENTITY_VISIBILITY, NULL);
    column = gtk_tree_view_get_column(GTK_TREE_VIEW(treeView_), col_offset - 1);
    gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN(column), GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_clickable(GTK_TREE_VIEW_COLUMN(column), FALSE);
    auto pixbuf = gdk_pixbuf_new_from_resource_at_scale("/org/mvlab/spam/res/model/eye.svg", 20, 20, TRUE, nullptr);
    auto image = gtk_image_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);
    gtk_widget_show(image);
    gtk_tree_view_column_set_widget(column, image);
    g_signal_connect(renderer, "toggled", G_CALLBACK(on_visibility_toggled), treeView_);

    GtkTreeIter iter;
    renderer = gtk_cell_renderer_combo_new();
    GtkListStore *values = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_list_store_append(values, &iter); gtk_list_store_set(values, &iter, 0, "Surface", -1);
    gtk_list_store_append(values, &iter); gtk_list_store_set(values, &iter, 0, "Surface With Edges", -1);
    gtk_list_store_append(values, &iter); gtk_list_store_set(values, &iter, 0, "Wireframe", -1);
    g_object_set(renderer, "has-entry", FALSE, "model", values, "text-column", 0, "editable", TRUE, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeView_), -1, "Representation", renderer, "text", ENTITY_DISPLAY_MODE, NULL);
    g_signal_connect(renderer, "edited", G_CALLBACK(on_representation_changed), this);
    g_object_unref(values);

    renderer = gtk_cell_renderer_toggle_new();
    col_offset = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeView_), -1, "Vertex", renderer, "active", ENTITY_SHOW_VERTEX, NULL);
    column = gtk_tree_view_get_column(GTK_TREE_VIEW(treeView_), col_offset - 1);
    pixbuf = gdk_pixbuf_new_from_resource_at_scale("/org/mvlab/spam/res/model/vertex.svg", 20, 20, TRUE, nullptr);
    image = gtk_image_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);
    gtk_widget_show(image);
    gtk_tree_view_column_set_widget(column, image);

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeView_), -1, "Color", renderer, "pixbuf", ENTITY_COLOR, NULL);

    mainView_ = gtk_scrolled_window_new(NULL, NULL);
    gtk_window_set_resizable(GTK_WINDOW(mainView_), TRUE);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(mainView_), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(mainView_), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(mainView_), treeView_);
    gtk_widget_set_size_request(mainView_, 480, 600);

    g_signal_connect(treeView_, "row-activated", G_CALLBACK(on_row_activated), this);
    g_signal_connect(treeView_, "button-press-event", G_CALLBACK(on_button_pressed), this);
    g_signal_connect(treeView_, "popup-menu", G_CALLBACK(on_popup_menu), this);
}

GLModelTreeView::~GLModelTreeView()
{
    g_object_unref(assemPixBuf);
    g_object_unref(partPixBuf);
    g_object_unref(bodyPixBuf);
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

void GLModelTreeView::on_visibility_toggled(GtkCellRendererToggle *celltoggle, gchar *path_string, GtkTreeView *tree_view)
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_path_free(path);

    gboolean active = FALSE;
    gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, ENTITY_VISIBILITY, &active, -1);
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, ENTITY_VISIBILITY, !active, -1);
    set_children_visibility(model, &iter, !active);
}

void GLModelTreeView::on_representation_changed(GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, gpointer data)
{
    GtkTreeIter iter;
    GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
    GLModelTreeView *myself = (GLModelTreeView *)data;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(myself->treeView_));
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, ENTITY_DISPLAY_MODE, new_text, -1);
}

void GLModelTreeView::on_add_part(GtkWidget *menuitem, gpointer userdata)
{
    GtkTreeIter itParent;
    GtkTreeModel *model = nullptr;
    GLModelTreeView *myself = (GLModelTreeView *)userdata;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(myself->treeView_));
    if (gtk_tree_selection_get_selected(selection, &model, &itParent))
    {
        GtkTreeIter itPart;
        std::string partName("Part");
        gtk_tree_store_append(GTK_TREE_STORE(model), &itPart, &itParent);
        gtk_tree_store_set(GTK_TREE_STORE(model), &itPart, ENTITY_NAME, partName.c_str(), ENTITY_ICON, myself->partPixBuf, ENTITY_DISPLAY_MODE, "Surface", ENTITY_TYPE, ENTITY_TYPE_PART, -1);
    }
}

void GLModelTreeView::on_add_assembly(GtkWidget *menuitem, gpointer userdata)
{
    GtkTreeIter itParent;
    GtkTreeModel *model = nullptr;
    GLModelTreeView *myself = (GLModelTreeView *)userdata;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(myself->treeView_));
    if (gtk_tree_selection_get_selected(selection, &model, &itParent))
    {
        GtkTreeIter itAssem;
        std::string assemName("Assembly");
        gtk_tree_store_append(GTK_TREE_STORE(model), &itAssem, &itParent);
        gtk_tree_store_set(GTK_TREE_STORE(model), &itAssem, ENTITY_NAME, assemName.c_str(), ENTITY_ICON, myself->assemPixBuf, ENTITY_DISPLAY_MODE, "Surface", ENTITY_TYPE, ENTITY_TYPE_ASSEMBLY, -1);
    }
}

gboolean GLModelTreeView::on_popup_menu(GtkWidget *treeview, gpointer userdata)
{
    view_popup_menu(treeview, NULL, userdata);
    return GDK_EVENT_STOP;
}

gboolean GLModelTreeView::on_button_pressed(GtkWidget *treeview, GdkEventButton *e, gpointer userdata)
{
    if (e->type == GDK_BUTTON_PRESS && e->button == 3)
    {
        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
        if (gtk_tree_selection_count_selected_rows(selection) <= 1)
        {
            GtkTreePath *path;
            if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint)e->x, (gint)e->y, &path, NULL, NULL, NULL))
            {
                gtk_tree_selection_unselect_all(selection);
                gtk_tree_selection_select_path(selection, path);
                gtk_tree_path_free(path);
            }
        }

        view_popup_menu(treeview, e, userdata);
        return GDK_EVENT_STOP;
    }

    return GDK_EVENT_PROPAGATE;
}

void GLModelTreeView::view_popup_menu(GtkWidget *treeview, GdkEventButton *e, gpointer userdata)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gint eType = -1;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gtk_tree_model_get(model, &iter, ENTITY_TYPE, &eType, -1);
    }

    if (eType >= 0 && eType < NUM_ENTITY_TYPES)
    {
        GtkWidget *menuitem = nullptr;
        GtkWidget *menu = gtk_menu_new();
        if (ENTITY_TYPE_MODEL == eType || ENTITY_TYPE_ASSEMBLY == eType)
        {
            menuitem = gtk_menu_item_new_with_label("Add Assembly");
            g_signal_connect(menuitem, "activate", G_CALLBACK(on_add_assembly), userdata);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

            menuitem = gtk_menu_item_new_with_label("Add Part");
            g_signal_connect(menuitem, "activate", G_CALLBACK(on_add_part), userdata);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

            menuitem = gtk_separator_menu_item_new();
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        }

        menuitem = gtk_menu_item_new_with_label("Show");
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        menuitem = gtk_menu_item_new_with_label("Show Only");
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        menuitem = gtk_menu_item_new_with_label("Show Reverse");
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        menuitem = gtk_menu_item_new_with_label("Show All");
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        menuitem = gtk_menu_item_new_with_label("Zoom To");
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        menuitem = gtk_menu_item_new_with_label("Hide");
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        gtk_widget_show_all(menu);
        gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, (e != NULL) ? e->button : 0, gdk_event_get_time((GdkEvent*)e));
    }
}

void GLModelTreeView::set_children_visibility(GtkTreeModel* model, GtkTreeIter* iterParent, const gboolean visible)
{
    GtkTreeIter  iter;
    if (gtk_tree_model_iter_children(model, &iter, iterParent))
    {
        gtk_tree_store_set(GTK_TREE_STORE(model), &iter, ENTITY_VISIBILITY, visible, -1);
        set_children_visibility(model, &iter, visible);

        while (gtk_tree_model_iter_next(model, &iter))
        {
            gtk_tree_store_set(GTK_TREE_STORE(model), &iter, ENTITY_VISIBILITY, visible, -1);
            set_children_visibility(model, &iter, visible);
        }
    }
}

gboolean GLModelTreeView::TreeModelForeachFunc(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter, gpointer data)
{
    return TRUE;
}
