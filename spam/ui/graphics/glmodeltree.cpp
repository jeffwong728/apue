#include "glmodeltree.h"
#include <wx/wxprec.h>
#include <wx/wx.h>

SPGLModelTreeView GLModelTreeView::MakeNew()
{
    return std::make_shared<GLModelTreeView>(this_is_private{ 0 });
}

GLModelTreeView::GLModelTreeView(const this_is_private&)
    : treeView_(nullptr), model_(nullptr), mainView_(nullptr)
{
    GtkTreeIter iter, child_iter;
    model_ = gtk_tree_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING);

    gtk_tree_store_append(model_, &iter, NULL);
    gtk_tree_store_set(model_, &iter, ENTITY_NAME, "part1", -1);
    gtk_tree_store_append(model_, &child_iter, &iter);
    gtk_tree_store_set(model_, &child_iter, ENTITY_NAME, "body1", ENTITY_VISIBILITY, TRUE, ENTITY_DISPLAY_MODE, "filled", ENTITY_COLOR, "red", -1);
    gtk_tree_store_append(model_, &child_iter, &iter);
    gtk_tree_store_set(model_, &child_iter, ENTITY_NAME, "body2", ENTITY_VISIBILITY, FALSE, ENTITY_DISPLAY_MODE, "wireframe", ENTITY_COLOR, "green", -1);
    gtk_tree_store_append(model_, &child_iter, &iter);
    gtk_tree_store_set(model_, &child_iter, ENTITY_NAME, "body3", ENTITY_VISIBILITY, TRUE, ENTITY_DISPLAY_MODE, "wireframe", ENTITY_COLOR, "yellow", -1);
    gtk_tree_store_append(model_, &child_iter, &iter);
    gtk_tree_store_set(model_, &child_iter, ENTITY_NAME, "body4", ENTITY_VISIBILITY, FALSE, ENTITY_DISPLAY_MODE, "filled", ENTITY_COLOR, "blue", -1);
    gtk_tree_store_append(model_, &child_iter, &iter);
    gtk_tree_store_set(model_, &child_iter, ENTITY_NAME, "body5", ENTITY_VISIBILITY, FALSE, ENTITY_DISPLAY_MODE, "wireframe", ENTITY_COLOR, "purple", -1);

    for (int i=6; i<200; ++i)
    {
        std::string strName("body");
        strName += std::to_string(i);
        gtk_tree_store_append(model_, &child_iter, &iter);
        gtk_tree_store_set(model_, &child_iter, ENTITY_NAME, strName.c_str(), ENTITY_VISIBILITY, FALSE, ENTITY_DISPLAY_MODE, "wireframe", ENTITY_COLOR, "purple", -1);
    }

    treeView_ = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model_));
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

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeView_), -1, "Name", renderer, "text", ENTITY_COLOR, NULL);

    mainView_ = gtk_scrolled_window_new(NULL, NULL);
    gtk_window_set_resizable(GTK_WINDOW(mainView_), TRUE);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(mainView_), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(mainView_), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(mainView_), treeView_);
    gtk_widget_set_size_request(mainView_, 380, 600);
}

GLModelTreeView::~GLModelTreeView()
{
}
