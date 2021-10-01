#include "glmodeltree.h"
#include "dispnode.h"
#include <wx/wxprec.h>
#include <wx/wx.h>
#include <vtkNew.h>
#include <vtkNamedColors.h>
#include <vtkProperty.h>
#include <string_view>
#include <queue>

SPGLModelTreeView GLModelTreeView::MakeNew(const wxWindow *const parent)
{
    return std::make_shared<GLModelTreeView>(this_is_private{ 0 }, parent);
}

GLModelTreeView::GLModelTreeView(const this_is_private&, const wxWindow *const parent)
    : treeView_(nullptr), model_(nullptr), mainView_(nullptr), parent_(parent)
{
    model_ = gtk_tree_store_new(NUM_COLUMNS, G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, G_TYPE_INT, G_TYPE_UINT64, G_TYPE_UINT64);

    vtkNew<vtkNamedColors> colors;
    vtkNew<vtkStringArray> colorNames;
    colors->GetColorNames(colorNames);
    const int numColors = colors->GetNumberOfColors();

    assemPixBuf = gdk_pixbuf_new_from_resource_at_scale("/org/mvlab/spam/res/model/assembly.svg", 16, 16, TRUE, nullptr);
    partPixBuf = gdk_pixbuf_new_from_resource_at_scale("/org/mvlab/spam/res/model/part.svg", 16, 16, TRUE, nullptr);
    bodyPixBuf = gdk_pixbuf_new_from_resource_at_scale("/org/mvlab/spam/res/model/solid.body.svg", 16, 16, TRUE, nullptr);

    GtkTreeIter itModel;
    const GLGUID modelGUID = GLGUID::MakeNew();
    gtk_tree_store_append(model_, &itModel, NULL);
    gtk_tree_store_set(model_, &itModel, ENTITY_NAME, "Model", ENTITY_DISPLAY_MODE, "Surface", ENTITY_TYPE, kENTITY_TYPE_MODEL, ENTITY_GUID_PART_1, modelGUID.part1, ENTITY_GUID_PART_2, modelGUID.part2, -1);

    for (int ass=0; ass<2; ++ass)
    {
        GtkTreeIter itAssem;
        std::string assemName("Assembly ");
        assemName += std::to_string(ass);
        const GLGUID assmGUID = GLGUID::MakeNew();
        gtk_tree_store_append(model_, &itAssem, &itModel);
        gtk_tree_store_set(model_, &itAssem, ENTITY_NAME, assemName.c_str(), ENTITY_ICON, assemPixBuf, ENTITY_DISPLAY_MODE, "Surface", ENTITY_TYPE, kENTITY_TYPE_ASSEMBLY, ENTITY_GUID_PART_1, assmGUID.part1, ENTITY_GUID_PART_2, assmGUID.part2, -1);
        for (int pat=0; pat<3; ++pat)
        {
            GtkTreeIter itPart;
            std::string partName("Part ");
            partName += std::to_string(pat);
            const GLGUID partGUID = GLGUID::MakeNew();
            gtk_tree_store_append(model_, &itPart, &itAssem);
            gtk_tree_store_set(model_, &itPart, ENTITY_NAME, partName.c_str(), ENTITY_ICON, partPixBuf, ENTITY_DISPLAY_MODE, "Surface", ENTITY_TYPE, kENTITY_TYPE_PART, ENTITY_GUID_PART_1, partGUID.part1, ENTITY_GUID_PART_2, partGUID.part2, -1);
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
                gtk_tree_store_set(model_, &itBody, ENTITY_NAME, bodyName.c_str(), ENTITY_ICON, bodyPixBuf, ENTITY_VISIBILITY, FALSE, ENTITY_DISPLAY_MODE, "Surface", ENTITY_COLOR, pixbuf, ENTITY_TYPE, kENTITY_TYPE_SOLID_BODY, -1);
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
    g_signal_connect(renderer, "toggled", G_CALLBACK(on_visibility_toggled), this);

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
    g_signal_connect(renderer, "toggled", G_CALLBACK(on_node_toggled), this);

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeView_), -1, "Color", renderer, "pixbuf", ENTITY_COLOR, NULL);

    mainView_ = gtk_scrolled_window_new(NULL, NULL);
    gtk_window_set_resizable(GTK_WINDOW(mainView_), TRUE);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(mainView_), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(mainView_), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(mainView_), treeView_);
    gtk_widget_set_size_request(mainView_, 480, 300);

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

void GLModelTreeView::CloseModel()
{
    GtkTreeIter itModel;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView_));
    if (gtk_tree_model_get_iter_first(model, &itModel))
    {
        GtkTreeIter itChild;
        while (gtk_tree_model_iter_children(model, &itChild, &itModel))
        {
            gtk_tree_store_remove(GTK_TREE_STORE(model), &itChild);
        }
    }
}

void GLModelTreeView::AddPart(const GLGUID &parentGuid, const std::string &partName, const SPDispNodes &dispNodes)
{
    GtkTreeIter itParent;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView_));

    gint parentType = FindEntity(parentGuid, &itParent);
    if (kENTITY_TYPE_MODEL != parentType && kENTITY_TYPE_ASSEMBLY != parentType)
    {
        parentType = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &itParent);
    }

    if (kENTITY_TYPE_MODEL == parentType || kENTITY_TYPE_ASSEMBLY == parentType)
    {
        GtkTreeIter itPart;
        gtk_tree_store_append(GTK_TREE_STORE(model), &itPart, &itParent);
        const GLGUID entityGUID = GLGUID::MakeNew();
        gtk_tree_store_set(GTK_TREE_STORE(model), &itPart, ENTITY_NAME, partName.c_str(), ENTITY_ICON, partPixBuf,
            ENTITY_VISIBILITY, TRUE, ENTITY_DISPLAY_MODE, "Surface", ENTITY_TYPE, kENTITY_TYPE_PART,
            ENTITY_GUID_PART_1, entityGUID.part1, ENTITY_GUID_PART_2, entityGUID.part2, -1);
        for (const SPDispNode &dispNode : dispNodes)
        {
            const vtkColor4ub color = dispNode->GetColor();
            const std::string bodyName = dispNode->GetName();
            const GLGUID bodyGUID = dispNode->GetGUID();
            const gint entityType = dispNode->GetEntityType();
            guint32 pixel = ((gint)(color.GetRed()) << 24 | ((gint)(color.GetGreen())) << 16 | ((gint)(color.GetBlue())) << 8 | ((gint)(color.GetAlpha())));
            GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 13, 13);
            gdk_pixbuf_fill(pixbuf, pixel);
            GtkTreeIter itBody;
            gtk_tree_store_append(GTK_TREE_STORE(model), &itBody, &itPart);
            gtk_tree_store_set(GTK_TREE_STORE(model), &itBody, ENTITY_NAME, bodyName.c_str(), ENTITY_ICON, bodyPixBuf, ENTITY_VISIBILITY, TRUE,
                ENTITY_DISPLAY_MODE, "Surface", ENTITY_COLOR, pixbuf, ENTITY_TYPE, entityType, ENTITY_GUID_PART_1, bodyGUID.part1, ENTITY_GUID_PART_2, bodyGUID.part2, -1);
            g_object_unref(pixbuf);
        }
    }
}

void GLModelTreeView::AddGeomBody(const GLGUID &partGUID, const SPDispNode &spGeomBody)
{
    GtkTreeIter itPart;
    if (spGeomBody && kENTITY_TYPE_PART == FindEntity(partGUID, &itPart))
    {
        GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView_));
        const vtkColor4ub color = spGeomBody->GetColor();
        const std::string bodyName = spGeomBody->GetName();
        const GLGUID bodyGUID = spGeomBody->GetGUID();
        const gint entityType = spGeomBody->GetEntityType();
        guint32 pixel = ((gint)(color.GetRed()) << 24 | ((gint)(color.GetGreen())) << 16 | ((gint)(color.GetBlue())) << 8 | ((gint)(color.GetAlpha())));
        GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 13, 13);
        gdk_pixbuf_fill(pixbuf, pixel);
        GtkTreeIter itBody;
        gtk_tree_store_append(GTK_TREE_STORE(model), &itBody, &itPart);
        gtk_tree_store_set(GTK_TREE_STORE(model), &itBody, ENTITY_NAME, bodyName.c_str(), ENTITY_ICON, bodyPixBuf, ENTITY_VISIBILITY, TRUE,
            ENTITY_DISPLAY_MODE, "Surface", ENTITY_COLOR, pixbuf, ENTITY_TYPE, entityType, ENTITY_GUID_PART_1, bodyGUID.part1, ENTITY_GUID_PART_2, bodyGUID.part2, -1);
        g_object_unref(pixbuf);
    }
}

const gint GLModelTreeView::FindEntity(const GLGUID &partGUID, GtkTreeIter *iter)
{
    GtkTreeIter itModel;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView_));
    iter->stamp = 0; iter->user_data = nullptr; iter->user_data2 = nullptr; iter->user_data3 = nullptr;
    if (partGUID.part1 && partGUID.part2 && gtk_tree_model_get_iter_first(model, &itModel))
    {
        std::queue<GtkTreeIter> itsToBeCheck;
        itsToBeCheck.push(itModel);
        while (!itsToBeCheck.empty())
        {
            gint eType = -1;
            guint64 part1 = 0;
            guint64 part2 = 0;
            GtkTreeIter &itThis = itsToBeCheck.front();
            gtk_tree_model_get(model, &itThis, ENTITY_TYPE, &eType, -1);
            gtk_tree_model_get(model, &itThis, ENTITY_GUID_PART_1, &part1, -1);
            gtk_tree_model_get(model, &itThis, ENTITY_GUID_PART_2, &part2, -1);
            if (partGUID.part1 == part1 && partGUID.part2 == part2)
            {
                *iter = itThis;
                return eType;
            }

            GtkTreeIter itChild;
            if (gtk_tree_model_iter_children(model, &itChild, &itThis))
            {
                itsToBeCheck.push(itChild);
                while (gtk_tree_model_iter_next(model, &itChild))
                {
                    itsToBeCheck.push(itChild);
                }
            }

            itsToBeCheck.pop();
        }
    }

    return kENTITY_TYPE_INVALID;
}

void GLModelTreeView::GetAllChildrenGUIDs(GtkTreeIter *const itParent, const gboolean includeParent, std::vector<GLGUID> &guids)
{
    if (itParent)
    {
        std::queue<GtkTreeIter> itsToBeCheck;
        itsToBeCheck.push(*itParent);
        GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView_));
        gboolean guidFirst = TRUE;
        while (!itsToBeCheck.empty())
        {
            GtkTreeIter &itThis = itsToBeCheck.front();
            if ((guidFirst && includeParent) || !guidFirst)
            {
                guint64 part1 = 0;
                guint64 part2 = 0;
                gtk_tree_model_get(model, &itThis, ENTITY_GUID_PART_1, &part1, -1);
                gtk_tree_model_get(model, &itThis, ENTITY_GUID_PART_2, &part2, -1);
                guids.emplace_back(part1, part2);
            }

            guidFirst = FALSE;

            GtkTreeIter itChild;
            if (gtk_tree_model_iter_children(model, &itChild, &itThis))
            {
                itsToBeCheck.push(itChild);
                while (gtk_tree_model_iter_next(model, &itChild))
                {
                    itsToBeCheck.push(itChild);
                }
            }

            itsToBeCheck.pop();
        }
    }
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

                        guint64 part1 = 0;
                        guint64 part2 = 0;
                        gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_1, &part1, -1);
                        gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_2, &part2, -1);
                        std::vector<GLGUID> guids;
                        guids.emplace_back(part1, part2);
                        std::vector<vtkColor4d> colors;
                        colors.emplace_back(color.red, color.green, color.blue);
                        myself->sig_ColorChanged(guids, colors);
                    }
                }

                g_object_unref(pixbuf);
            }
        }
    }

    gtk_widget_destroy(GTK_WIDGET(dialog));
}

void GLModelTreeView::on_visibility_toggled(GtkCellRendererToggle *celltoggle, gchar *path_string, gpointer data)
{
    GtkTreeIter iter;
    GLModelTreeView *myself = (GLModelTreeView *)data;
    GtkTreeView *tree_view = GTK_TREE_VIEW(myself->treeView_);
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_path_free(path);

    gboolean active = FALSE;
    gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, ENTITY_VISIBILITY, &active, -1);
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, ENTITY_VISIBILITY, !active, -1);

    guint64 part1 = 0;
    guint64 part2 = 0;
    std::vector<GLGUID> guids;
    gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_1, &part1, -1);
    gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_2, &part2, -1);
    guids.emplace_back(part1, part2);

    set_children_visibility(model, &iter, !active, guids);

    std::vector<int> visibles(guids.size(), !active);
    myself->sig_VisibilityChanged(guids, visibles);
}

void GLModelTreeView::on_node_toggled(GtkCellRendererToggle *celltoggle, gchar *path_string, gpointer data)
{
    GtkTreeIter iter;
    GLModelTreeView *myself = (GLModelTreeView *)data;
    GtkTreeView *tree_view = GTK_TREE_VIEW(myself->treeView_);
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_path_free(path);

    gboolean active = FALSE;
    gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, ENTITY_SHOW_VERTEX, &active, -1);
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, ENTITY_SHOW_VERTEX, !active, -1);

    guint64 part1 = 0;
    guint64 part2 = 0;
    std::vector<GLGUID> guids;
    gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_1, &part1, -1);
    gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_2, &part2, -1);
    guids.emplace_back(part1, part2);

    set_children_node(model, &iter, !active, guids);

    std::vector<int> visibles(guids.size(), !active);
    myself->sig_ShowNodeChanged(guids, visibles);
}

void GLModelTreeView::on_representation_changed(GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, gpointer data)
{
    GtkTreeIter iter;
    GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
    GLModelTreeView *myself = (GLModelTreeView *)data;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(myself->treeView_));
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, ENTITY_DISPLAY_MODE, new_text, -1);

    guint64 part1 = 0;
    guint64 part2 = 0;
    std::vector<GLGUID> guids;
    gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_1, &part1, -1);
    gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_2, &part2, -1);
    guids.emplace_back(part1, part2);

    std::vector<int> reps;
    if (std::string_view("Wireframe") == std::string_view(new_text))
    {
        reps.push_back(kGREP_VTK_WIREFRAME);
    }
    else if (std::string_view("Surface With Edges") == std::string_view(new_text))
    {
        reps.push_back(kGREP_SURFACE_WITH_EDGE);
    }
    else
    {
        reps.push_back(VTK_SURFACE);
    }

    myself->sig_RepresentationChanged(guids, reps);
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
        const GLGUID entityGUID = GLGUID::MakeNew();
        gtk_tree_store_append(GTK_TREE_STORE(model), &itPart, &itParent);
        gtk_tree_store_set(GTK_TREE_STORE(model), &itPart, ENTITY_NAME, partName.c_str(), ENTITY_ICON, myself->partPixBuf,
            ENTITY_DISPLAY_MODE, "Surface", ENTITY_TYPE, kENTITY_TYPE_PART, ENTITY_GUID_PART_1, entityGUID.part1, ENTITY_GUID_PART_2, entityGUID.part2, -1);
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
        const GLGUID entityGUID = GLGUID::MakeNew();
        gtk_tree_store_append(GTK_TREE_STORE(model), &itAssem, &itParent);
        gtk_tree_store_set(GTK_TREE_STORE(model), &itAssem, ENTITY_NAME, assemName.c_str(), ENTITY_ICON, myself->assemPixBuf,
            ENTITY_DISPLAY_MODE, "Surface", ENTITY_TYPE, kENTITY_TYPE_ASSEMBLY, ENTITY_GUID_PART_1, entityGUID.part1, ENTITY_GUID_PART_2, entityGUID.part2, -1);
    }
}

void GLModelTreeView::on_add_geom_body(GtkWidget *menuitem, gpointer userdata)
{
    GtkTreeIter itPart;
    GtkTreeModel *model = nullptr;
    GLModelTreeView *myself = (GLModelTreeView *)userdata;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(myself->treeView_));
    if (gtk_tree_selection_get_selected(selection, &model, &itPart))
    {
        gint eType = -1;
        guint64 part1 = 0;
        guint64 part2 = 0;
        gtk_tree_model_get(model, &itPart, ENTITY_TYPE, &eType, -1);
        gtk_tree_model_get(model, &itPart, ENTITY_GUID_PART_1, &part1, -1);
        gtk_tree_model_get(model, &itPart, ENTITY_GUID_PART_2, &part2, -1);
        if (kENTITY_TYPE_PART == eType)
        {
            const GLGUID partGUID{ part1 , part2 };
            std::string_view label(gtk_menu_item_get_label(GTK_MENU_ITEM(menuitem)));
            if (label == "Sphere")
            {
                myself->sig_AddGeomBody(partGUID, kPGS_SPHERE);
            }
            else if (label == "Cylinder")
            {
                myself->sig_AddGeomBody(partGUID, kPGS_CYLINDER);
            }
            else
            {
                myself->sig_AddGeomBody(partGUID, kPGS_BOX);
            }
        }
    }
}

void GLModelTreeView::on_add_vtk_cell(GtkWidget *menuitem, gpointer userdata)
{
}

void GLModelTreeView::on_delete_self(GtkWidget *menuitem, gpointer userdata)
{
    GtkTreeIter itParent;
    GtkTreeModel *model = nullptr;
    GLModelTreeView *myself = (GLModelTreeView *)userdata;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(myself->treeView_));
    if (gtk_tree_selection_get_selected(selection, &model, &itParent))
    {
        std::vector<GLGUID> guids;
        myself->GetAllChildrenGUIDs(&itParent, TRUE, guids);
        gtk_tree_store_remove(GTK_TREE_STORE(model), &itParent);

        if (!guids.empty())
        {
            myself->sig_EntitiesDeleted(guids);
        }
    }
}

void GLModelTreeView::on_delete_children(GtkWidget *menuitem, gpointer userdata)
{
    GtkTreeIter itParent;
    GtkTreeModel *model = nullptr;
    GLModelTreeView *myself = (GLModelTreeView *)userdata;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(myself->treeView_));
    if (gtk_tree_selection_get_selected(selection, &model, &itParent))
    {
        std::vector<GLGUID> guids;
        myself->GetAllChildrenGUIDs(&itParent, FALSE, guids);
        GtkTreeIter itChild;
        while (gtk_tree_model_iter_children(model, &itChild, &itParent))
        {
            gtk_tree_store_remove(GTK_TREE_STORE(model), &itChild);
        }

        if (!guids.empty())
        {
            myself->sig_EntitiesDeleted(guids);
        }
    }
}

void GLModelTreeView::on_import_model(GtkWidget *menuitem, gpointer userdata)
{
    GtkTreeIter itParent;
    GtkTreeModel *model = nullptr;
    GLModelTreeView *myself = (GLModelTreeView *)userdata;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(myself->treeView_));
    if (gtk_tree_selection_get_selected(selection, &model, &itParent))
    {
        gint eType = -1;
        guint64 part1 = 0;
        guint64 part2 = 0;
        gtk_tree_model_get(model, &itParent, ENTITY_TYPE, &eType, -1);
        gtk_tree_model_get(model, &itParent, ENTITY_GUID_PART_1, &part1, -1);
        gtk_tree_model_get(model, &itParent, ENTITY_GUID_PART_2, &part2, -1);

        const GLGUID guid(part1, part2);
        myself->sig_ImportModel(guid);
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

    if (eType >= kENTITY_TYPE_MODEL && eType < kENTITY_TYPE_GUARD)
    {
        GtkWidget *menuitem = nullptr;
        GtkWidget *menu = gtk_menu_new();
        if (kENTITY_TYPE_MODEL == eType || kENTITY_TYPE_ASSEMBLY == eType)
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

        bool hasDelete = false;
        if (kENTITY_TYPE_MODEL != eType)
        {
            menuitem = gtk_menu_item_new_with_label("Delete");
            g_signal_connect(menuitem, "activate", G_CALLBACK(on_delete_self), userdata);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
            hasDelete = true;
        }

        if (gtk_tree_model_iter_has_child(model, &iter))
        {
            menuitem = gtk_menu_item_new_with_label("Delete Children");
            g_signal_connect(menuitem, "activate", G_CALLBACK(on_delete_children), userdata);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
            hasDelete = true;
        }

        if (hasDelete)
        {
            menuitem = gtk_separator_menu_item_new();
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        }

        if (kENTITY_TYPE_MODEL == eType || kENTITY_TYPE_ASSEMBLY == eType)
        {
            menuitem = gtk_menu_item_new_with_label("Import Model");
            g_signal_connect(menuitem, "activate", G_CALLBACK(on_import_model), userdata);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

            menuitem = gtk_separator_menu_item_new();
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        }

        const std::string_view viNames[] = { "Show", "Show Only", "Show Reverse", "Show All", "Zoom To", "Hide" };
        for (const std::string_view &viName : viNames)
        {
            menuitem = gtk_menu_item_new_with_label(viName.data());
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        }

        if (kENTITY_TYPE_PART == eType)
        {
            menuitem = gtk_separator_menu_item_new();
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

            GtkWidget *geomSubMenu = gtk_menu_new();
            const std::string_view geomNames[] = { "Box", "Sphere", "Cylinder" };
            for (const std::string_view &geomName : geomNames)
            {
                GtkWidget *geomMI = gtk_menu_item_new_with_label(geomName.data());
                g_signal_connect(geomMI, "activate", G_CALLBACK(on_add_geom_body), userdata);
                gtk_menu_shell_append(GTK_MENU_SHELL(geomSubMenu), geomMI);
            }

            menuitem = gtk_menu_item_new_with_label("Add Geometry");
            gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), geomSubMenu);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

            GtkWidget *cellSubMenu = gtk_menu_new();
            const std::string_view cellNames[] = { "Triangle", "Quad", "Tetra", "Hexahedron", "Wedge" };
            for (const std::string_view &cellName : cellNames)
            {
                GtkWidget *cellMI = gtk_menu_item_new_with_label(cellName.data());
                g_signal_connect(cellMI, "activate", G_CALLBACK(on_add_vtk_cell), userdata);
                gtk_menu_shell_append(GTK_MENU_SHELL(cellSubMenu), cellMI);
            }

            menuitem = gtk_menu_item_new_with_label("Add VTK Cell");
            gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), cellSubMenu);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        }

        gtk_widget_show_all(menu);
        gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, (e != NULL) ? e->button : 0, gdk_event_get_time((GdkEvent*)e));
    }
}

void GLModelTreeView::set_children_visibility(GtkTreeModel* model, GtkTreeIter* iterParent, const gboolean visible, std::vector<GLGUID> &guids)
{
    GtkTreeIter  iter;
    if (gtk_tree_model_iter_children(model, &iter, iterParent))
    {
        guint64 part1 = 0;
        guint64 part2 = 0;
        gtk_tree_store_set(GTK_TREE_STORE(model), &iter, ENTITY_VISIBILITY, visible, -1);
        gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_1, &part1, -1);
        gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_2, &part2, -1);
        guids.emplace_back(part1, part2);
        set_children_visibility(model, &iter, visible, guids);

        while (gtk_tree_model_iter_next(model, &iter))
        {
            gtk_tree_store_set(GTK_TREE_STORE(model), &iter, ENTITY_VISIBILITY, visible, -1);
            gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_1, &part1, -1);
            gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_2, &part2, -1);
            guids.emplace_back(part1, part2);
            set_children_visibility(model, &iter, visible, guids);
        }
    }
}

void GLModelTreeView::set_children_node(GtkTreeModel* model, GtkTreeIter* iterParent, const gboolean visible, std::vector<GLGUID> &guids)
{
    GtkTreeIter  iter;
    if (gtk_tree_model_iter_children(model, &iter, iterParent))
    {
        guint64 part1 = 0;
        guint64 part2 = 0;
        gtk_tree_store_set(GTK_TREE_STORE(model), &iter, ENTITY_SHOW_VERTEX, visible, -1);
        gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_1, &part1, -1);
        gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_2, &part2, -1);
        guids.emplace_back(part1, part2);
        set_children_node(model, &iter, visible, guids);

        while (gtk_tree_model_iter_next(model, &iter))
        {
            gtk_tree_store_set(GTK_TREE_STORE(model), &iter, ENTITY_SHOW_VERTEX, visible, -1);
            gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_1, &part1, -1);
            gtk_tree_model_get(model, &iter, ENTITY_GUID_PART_2, &part2, -1);
            guids.emplace_back(part1, part2);
            set_children_node(model, &iter, visible, guids);
        }
    }
}

gboolean GLModelTreeView::TreeModelForeachFunc(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter, gpointer data)
{
    return TRUE;
}
