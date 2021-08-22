#include "wx/wxprec.h"
#include <wx/wx.h>
#include <wx/log.h>
#include "gtkglareawidget.h"
#include "wx/gtk/private.h"
#include "wx/gtk/private/eventsdisabler.h"
#include "wx/gtk/private/list.h"
#include <opencv2/opencv.hpp>
#include <epoxy/gl.h>
#include <GL/glu.h>
#include <boost/filesystem.hpp>
#include <boost/dll/runtime_symbol_info.hpp>

glm::mat4 camera(float Translate, glm::vec2 const& Rotate)
{
    glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.f);
    glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Translate));
    View = glm::rotate(View, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
    View = glm::rotate(View, Rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
    return Projection * View * Model;
}

extern "C" {
static void gtk_switchbutton_clicked_callback(GtkWidget *WXUNUSED(widget), GParamSpec *WXUNUSED(pspec), wxGLAreaWidget *cb)
{
    const glm::mat4 m = camera(1.f, glm::vec2(1.f, 0.5f));
    std::cout << "matrix diagonal: " << m[0][0] << ", "
        << m[1][1] << ", " << m[2][2] << ", " << m[3][3] << "\n";
}
}


static const char *vertex_shader_code_330 =
"#version 330\n" \
"\n" \
"layout(location = 0) in vec3 aPos;\n" \
"\n" \
"uniform mat4 modelview;\n" \
"uniform mat4 projection;\n" \
"\n" \
"void main() {\n" \
"  gl_Position = projection * modelview * vec4(aPos, 1.0f);\n" \
"}";

static const char *fragment_shader_code_330 =
"#version 330\n" \
"\n" \
"uniform vec4 cColor;\n" \
"out vec4 outputColor;\n" \
"void main() {\n" \
"  float lerpVal = gl_FragCoord.y / 400.0f;\n" \
"  outputColor = cColor;\n" \
"}";

static const char *bk_vs =
"#version 330 core\n" \
"layout(location = 0) in vec3 aPos;\n" \
"layout(location = 1) in vec2 aTexCoord;\n" \
"\n" \
"out vec2 TexCoord;\n" \
"\n" \
"void main()\n" \
"{\n" \
"    gl_Position = vec4(aPos, 1.0);\n" \
"    TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n" \
"}";

static const char *bk_fs =
"#version 330 core\n" \
"out vec4 FragColor;\n" \
"\n" \
"in vec2 TexCoord;\n" \
"\n" \
"// texture sampler\n" \
"uniform sampler2D texture1;\n" \
"\n" \
"void main()\n" \
"{\n" \
"    FragColor = texture(texture1, TexCoord);\n" \
"}";

wxIMPLEMENT_DYNAMIC_CLASS(wxGLAreaWidget, wxControl);

bool wxGLAreaWidget::Create(wxWindow *parent, wxWindowID id,
                            const wxPoint &pos,
                            const wxSize &size, long style,
                            const wxValidator& validator,
                            const wxString &name)
{
    if (!PreCreation(parent, pos, size) ||
        !CreateBase(parent, id, pos, size, style, validator, name ))
    {
        wxFAIL_MSG(wxT("wxGLAreaWidget creation failed"));
        return false;
    }

    m_widget = gtk_gl_area_new();
    g_object_ref_sink(m_widget);
    gtk_widget_add_events(m_widget, GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK);

    g_signal_connect(m_widget, "realize", G_CALLBACK(realize_cb), this);
    g_signal_connect(m_widget, "unrealize", G_CALLBACK(unrealize_cb), this);
    g_signal_connect(m_widget, "render", G_CALLBACK(render_cb), this);

    m_parent->DoAddChild(this);

    PostCreation(size);

    Bind(wxEVT_SIZE,        &wxGLAreaWidget::OnSize,            this, wxID_ANY);
    Bind(wxEVT_MOTION,      &wxGLAreaWidget::OnMouseMotion,     this, wxID_ANY);
    Bind(wxEVT_LEFT_DCLICK, &wxGLAreaWidget::OnMiddleUp,        this, wxID_ANY);
    Bind(wxEVT_MIDDLE_DOWN, &wxGLAreaWidget::OnMiddleDown,      this, wxID_ANY);
    Bind(wxEVT_LEFT_DOWN,   &wxGLAreaWidget::OnLeftMouseDown,   this, wxID_ANY);
    Bind(wxEVT_LEFT_UP,     &wxGLAreaWidget::OnLeftMouseUp,     this, wxID_ANY);
    Bind(wxEVT_RIGHT_DOWN,  &wxGLAreaWidget::OnRightMouseDown,  this, wxID_ANY);
    Bind(wxEVT_RIGHT_UP,    &wxGLAreaWidget::OnRightMouseUp,    this, wxID_ANY);
    Bind(wxEVT_SET_FOCUS,   &wxGLAreaWidget::OnSetFocus,        this, wxID_ANY);
    Bind(wxEVT_KILL_FOCUS,  &wxGLAreaWidget::OnKillFocus,       this, wxID_ANY);
    Bind(wxEVT_MOUSEWHEEL,  &wxGLAreaWidget::OnMouseWheel,      this, wxID_ANY);

    norm_ = glm::mat4(1.f);
    norm_ = glm::rotate(norm_, glm::radians(45.f), glm::vec3(-1.0f, 0.0f, 0.0f));
    norm_ = glm::rotate(norm_, glm::radians(45.f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelview_ = norm_;
    projection_ = glm::mat4(1.0f);

    return true;
}

void wxGLAreaWidget::GTKDisableEvents()
{
    g_signal_handlers_block_by_func(m_widget,
                                (gpointer)gtk_switchbutton_clicked_callback, this);
}

void wxGLAreaWidget::GTKEnableEvents()
{
    g_signal_handlers_unblock_by_func(m_widget,
                                (gpointer)gtk_switchbutton_clicked_callback, this);
}

void wxGLAreaWidget::SetAxisAngleValue(const int axisIndex, const int angleVal)
{
    wxCHECK_RET(m_widget != NULL, wxT("invalid switch button"));
    Refresh(false);
}

bool wxGLAreaWidget::GetValue() const
{
    wxCHECK_MSG(m_widget != NULL, false, wxT("invalid switch button"));

    return gtk_switch_get_active(GTK_SWITCH(m_widget)) != 0;
}

void wxGLAreaWidget::OnSize(wxSizeEvent &e)
{
    const int w = e.GetSize().GetWidth();
    const int h = e.GetSize().GetHeight();
    glViewport(0, 0, w, h);

    top_ = 1.f;
    bottom_ = -1.f;
    left_ = -(float)w / (float)h;
    right_ = -left_;

    projection_ = glm::ortho(left_, right_, bottom_, top_, zNear_, zFar_);

}

void wxGLAreaWidget::OnLeftMouseDown(wxMouseEvent &e)
{
    if (e.LeftIsDown() && e.RightIsDown())
    {
        modelview_ = norm_;
        Refresh(false);
    }

    lastPos_ = e.GetPosition();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    pos(&dragPosX_, &dragPosY_, &dragPosZ_, lastPos_.x, lastPos_.y, viewport);
}

void wxGLAreaWidget::OnLeftMouseUp(wxMouseEvent &e)
{
    lastPos_ = e.GetPosition();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    pos(&dragPosX_, &dragPosY_, &dragPosZ_, lastPos_.x, lastPos_.y, viewport);
}

void wxGLAreaWidget::OnRightMouseDown(wxMouseEvent &e)
{
    if (e.LeftIsDown() && e.RightIsDown())
    {
        modelview_ = norm_;
        Refresh(false);
    }

    anchorPos_ = e.GetPosition();
    lastPos_ = e.GetPosition();
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    pos(&dragPosX_, &dragPosY_, &dragPosZ_, lastPos_.x, lastPos_.y, viewport);
}

void wxGLAreaWidget::OnRightMouseUp(wxMouseEvent &e)
{
    anchorPos_ = wxPoint();
    lastPos_ = wxPoint();
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    pos(&dragPosX_, &dragPosY_, &dragPosZ_, lastPos_.x, lastPos_.y, viewport);
}

void wxGLAreaWidget::OnMiddleDown(wxMouseEvent &e)
{
    anchorPos_ = e.GetPosition();
    lastPos_ = e.GetPosition();
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    pos(&dragPosX_, &dragPosY_, &dragPosZ_, lastPos_.x, lastPos_.y, viewport);
}

void wxGLAreaWidget::OnMiddleUp(wxMouseEvent &e)
{
    anchorPos_ = wxPoint();
    lastPos_ = wxPoint();
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    pos(&dragPosX_, &dragPosY_, &dragPosZ_, lastPos_.x, lastPos_.y, viewport);
}

void wxGLAreaWidget::OnMouseMotion(wxMouseEvent &e)
{
    const int x = e.GetPosition().x;
    const int y = e.GetPosition().y;
    const int dx = x - lastPos_.x;
    const int dy = y - lastPos_.y;
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    if (e.Dragging() && e.RightIsDown())
    {
        float px, py, pz;
        pos(&px, &py, &pz, x, y, viewport);
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, glm::vec3(px - dragPosX_, py - dragPosY_, pz - dragPosZ_));
        modelview_ = m * modelview_;

        dragPosX_ = px;
        dragPosY_ = py;
        dragPosZ_ = pz;

        if (dx || dx)
        {
            Refresh(false);
        }
    }

    if (e.Dragging() && e.MiddleIsDown())
    {
        const float ax = dy;
        const float ay = dx;
        const float az = 0.0;
        const float angle = vlen(ax, ay, az) / (double)(viewport[2] + 1) * 360.f;

        glm::mat4 invm = glm::inverse(modelview_);
        const float bx = invm[0][0] * ax + invm[1][0] * ay + invm[2][0] * az;
        const float by = invm[0][1] * ax + invm[1][1] * ay + invm[2][1] * az;
        const float bz = invm[0][2] * ax + invm[1][2] * ay + invm[2][2] * az;

        modelview_ = glm::translate(modelview_, glm::vec3(0.f, 0.f, 0.f));
        modelview_ = glm::rotate(modelview_, glm::radians(angle), glm::vec3(bx, by, bz));
        modelview_ = glm::translate(modelview_, glm::vec3(0.f, 0.f, 0.f));

        if (dx || dx)
        {
            Refresh(false);
        }
    }

    lastPos_ = e.GetPosition();
}

void wxGLAreaWidget::OnMouseWheel(wxMouseEvent &e)
{
    const float s = e.GetWheelRotation() > 0 ? 1.2f : 0.8f;
    modelview_ = glm::scale(modelview_, glm::vec3(s, s, s));
    Refresh(false);
}

void wxGLAreaWidget::OnSetFocus(wxFocusEvent &e)
{
    wxLogMessage(wxT("wxGLAreaWidget::OnSetFocus."));
}

void wxGLAreaWidget::OnKillFocus(wxFocusEvent &e)
{
    wxLogMessage(wxT("wxGLAreaWidget::OnKillFocus."));
}

void wxGLAreaWidget::DoApplyWidgetStyle(GtkRcStyle *style)
{
    GTKApplyStyle(m_widget, style);
    GtkWidget* child = gtk_bin_get_child(GTK_BIN(m_widget));
    GTKApplyStyle(child, style);

#ifndef __WXGTK4__
    wxGCC_WARNING_SUPPRESS(deprecated-declarations)
    // for buttons with images, the path to the label is (at least in 2.12)
    // GtkButton -> GtkAlignment -> GtkHBox -> GtkLabel
    if ( GTK_IS_ALIGNMENT(child) )
    {
        GtkWidget* box = gtk_bin_get_child(GTK_BIN(child));
        if ( GTK_IS_BOX(box) )
        {
            wxGtkList list(gtk_container_get_children(GTK_CONTAINER(box)));
            for (GList* item = list; item; item = item->next)
            {
                GTKApplyStyle(GTK_WIDGET(item->data), style);
            }
        }
    }
    wxGCC_WARNING_RESTORE(0)
#endif
}

wxSize wxGLAreaWidget::DoGetBestSize() const
{
    return wxSize(64, 48);
}

GLuint wxGLAreaWidget::LoadTexture()
{
    boost::system::error_code ec;
    boost::filesystem::path p = boost::dll::program_location(ec);
    p = p.parent_path();
    p.append(wxT("res")).append(wxT("Apex.png"));

    bk_texture = 0;
    if (boost::filesystem::exists(p, ec) && boost::filesystem::is_regular_file(p, ec))
    {
        cv::Mat srcMat = cv::imread(cv::String(wxString(p.native())), cv::IMREAD_COLOR), fImg, bkImg;
        if (!srcMat.empty() && CV_8U == srcMat.depth())
        {
            cv::cvtColor(srcMat, fImg, cv::COLOR_BGR2RGB);
            cv::flip(fImg, bkImg, 0);

            glGenTextures(1, &bk_texture);
            glBindTexture(GL_TEXTURE_2D, bk_texture);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

            //even better quality, but this will do for now.
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            //to the edge of our shape. 
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            //Generate the texture
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bkImg.cols, bkImg.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, bkImg.data);
        }
    }

    return bk_texture;

}

void wxGLAreaWidget::StartOrthogonal()
{
    GtkAllocation rcAlloc;
    gtk_widget_get_allocation(m_widget, &rcAlloc);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-rcAlloc.width / 2, rcAlloc.width / 2, -rcAlloc.height / 2, rcAlloc.height / 2);
    glMatrixMode(GL_MODELVIEW);
}

void wxGLAreaWidget::EndOrthogonal()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void wxGLAreaWidget::DrawBackground()
{
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, bk_texture);
    glUseProgram(bk_program);
    glBindVertexArray(bk_position_buffer);

    StartOrthogonal();

    // texture width/height
    glPushMatrix();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glPopMatrix();

    EndOrthogonal();
    gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void wxGLAreaWidget::pos(float *px, float *py, float *pz, const int x, const int y, const int *viewport) const
{
    *px = (double)(x - viewport[0]) / (double)(viewport[2]);
    *py = (double)(y - viewport[1]) / (double)(viewport[3]);

    *px = left_ + (*px)*(right_ - left_);
    *py = top_ + (*py)*(bottom_ - top_);
    *pz = zNear_;
}

void wxGLAreaWidget::init_buffers(GLuint *vao_out, GLuint *buffer_out)
{
    GLuint vao, buffer, EBO;

    float vertices[] = {
        -0.2f, -0.2f, -0.2f,
         0.2f, -0.2f, -0.2f,
         0.2f,  0.2f, -0.2f,
         -0.2f,  0.2f, -0.2f,

        -0.2f, -0.2f, 0.2f,
         0.2f, -0.2f, 0.2f,
         0.2f,  0.2f, 0.2f,
         -0.2f,  0.2f, 0.2f
    };

    unsigned int indices[] = {
        0, 1, 2, 0, 2, 3,
        0, 1, 5, 0, 5, 4,
        1, 2, 6, 1, 6, 5,
        2, 6, 7, 2, 7, 3,
        3, 7, 4, 3, 4, 0,
        4, 5, 6, 4, 6, 7,
        0, 1, 1, 2, 2, 3, 3, 0,
        0, 4, 1, 5, 2, 6, 3, 7,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 1, 2, 3, 4, 5, 6, 7
    };

    /* we only use one VAO, so we always keep it bound */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if (vao_out != NULL)
        *vao_out = vao;

    if (buffer_out != NULL)
        *buffer_out = buffer;
}

void wxGLAreaWidget::init_bk_buffers(GLuint *vao_out, GLuint *buffer_out)
{
    float vertices[] = {
        // positions          // texture coords
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f, // top right
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };

    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if (vao_out != NULL)
        *vao_out = VAO;

    if (buffer_out != NULL)
        *buffer_out = VBO;
}

GLuint wxGLAreaWidget::create_shader(int type, const char *src)
{
    GLuint shader;
    int status;

    shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        int log_len;
        std::vector<char> buffer;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

        buffer.resize(log_len + 1);
        glGetShaderInfoLog(shader, log_len, NULL, buffer.data());

        g_warning("Compile failure in %s shader:\n%s",
            type == GL_VERTEX_SHADER ? "vertex" : "fragment",
            buffer.data());

        glDeleteShader(shader);

        return 0;
    }

    return shader;
}

void wxGLAreaWidget::init_shaders(const char *vertex_shader_code, const char *fragment_shader_code, GLuint *program_out, GLuint *mvp_out)
{
    GLuint vertex, fragment;
    GLuint program = 0;
    GLuint mvp = 0;
    int status;

    vertex = create_shader(GL_VERTEX_SHADER, vertex_shader_code);
    if (vertex == 0)
    {
        *program_out = 0;
        return;
    }

    fragment = create_shader(GL_FRAGMENT_SHADER, fragment_shader_code);
    if (fragment == 0)
    {
        glDeleteShader(vertex);
        *program_out = 0;
        return;
    }

    program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        int log_len;
        std::vector<char> buffer;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);

        buffer.resize(log_len + 1);
        glGetProgramInfoLog(program, log_len, NULL, buffer.data());

        g_warning("Linking failure:\n%s", buffer.data());

        glDeleteProgram(program);
        program = 0;

        goto out;
    }

    mvp = glGetUniformLocation(program, "mvp");

    glDetachShader(program, vertex);
    glDetachShader(program, fragment);

out:
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if (program_out != NULL)
        *program_out = program;

    if (mvp_out != NULL)
        *mvp_out = mvp;
}

void wxGLAreaWidget::draw_triangle(wxGLAreaWidget *glArea)
{
    g_assert(glArea->position_buffer != 0);
    g_assert(glArea->program != 0);

    glUseProgram(glArea->program);

    // create transformations
    glm::mat4 modelview = glArea->modelview_;
    glm::mat4 projection = glArea->projection_;

    // retrieve the matrix uniform locations
    GLuint modelViewLoc = glGetUniformLocation(glArea->program, "modelview");
    GLuint projLoc = glGetUniformLocation(glArea->program, "projection");
    GLuint colorLoc = glGetUniformLocation(glArea->program, "cColor");

    glUniformMatrix4fv(modelViewLoc, 1, GL_FALSE, glm::value_ptr(modelview));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    GLfloat faceColor[] = { 1.0f, 0.85f, 0.35f, 1.0f };
    GLfloat edgeColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat vertColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };

    glBindVertexArray(glArea->position_buffer);
    //glUniform4fv(colorLoc, 1, faceColor);
    //glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glUniform4fv(colorLoc, 1, faceColor);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, (void*)(36*sizeof(unsigned int)));
    glUniform4fv(colorLoc, 1, vertColor);
    glDrawElements(GL_POINTS, 8, GL_UNSIGNED_INT, (void*)(60 * sizeof(unsigned int)));

    glUseProgram(0);
}

void wxGLAreaWidget::realize_cb(GtkWidget *widget, gpointer user_data)
{
    const char *fragment, *vertex;
    GdkGLContext *context;

    gtk_gl_area_make_current(GTK_GL_AREA(widget));
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(widget), true);

    if (gtk_gl_area_get_error(GTK_GL_AREA(widget)) != NULL)
        return;

    context = gtk_gl_area_get_context(GTK_GL_AREA(widget));
    if (gdk_gl_context_get_use_es(context))
    {
        return;
    }
    else
    {
        if (!gdk_gl_context_is_legacy(context))
        {
            vertex = vertex_shader_code_330;
            fragment = fragment_shader_code_330;
        }
        else
        {
            return;
        }
    }

    wxGLAreaWidget *glArea = reinterpret_cast<wxGLAreaWidget *>(user_data);
    init_buffers(&glArea->position_buffer, NULL);
    init_bk_buffers(&glArea->bk_position_buffer, NULL);
    init_shaders(vertex, fragment, &glArea->program, &glArea->mvp_location);
    init_shaders(bk_vs, bk_fs, &glArea->bk_program, NULL);
    
    glArea->bk_texture = glArea->LoadTexture();
    glEnable(GL_DEPTH_TEST);
    glPointSize(5.0f);
    glLineWidth(1.0f);
}

void wxGLAreaWidget::unrealize_cb(GtkWidget *widget, gpointer user_data)
{
    gtk_gl_area_make_current(GTK_GL_AREA(widget));

    if (gtk_gl_area_get_error(GTK_GL_AREA(widget)) != NULL)
        return;

    wxGLAreaWidget *glArea = reinterpret_cast<wxGLAreaWidget *>(user_data);
    glDeleteBuffers(1, &glArea->position_buffer);
    glDeleteProgram(glArea->program);
}

gboolean wxGLAreaWidget::render_cb(GtkGLArea *area, GdkGLContext *context, gpointer user_data)
{
    wxGLAreaWidget *glArea = reinterpret_cast<wxGLAreaWidget *>(user_data);

    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (glArea->bk_texture)
    {
        glDepthMask(GL_FALSE);
        glArea->DrawBackground();
        glDepthMask(GL_TRUE);
    }

    draw_triangle(reinterpret_cast<wxGLAreaWidget *>(user_data));
    return TRUE;
}
