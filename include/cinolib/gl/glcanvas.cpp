/********************************************************************************
*  This file is part of CinoLib                                                 *
*  Copyright(C) 2021: Marco Livesu                                              *
*                                                                               *
*  The MIT License                                                              *
*                                                                               *
*  Permission is hereby granted, free of charge, to any person obtaining a      *
*  copy of this software and associated documentation files (the "Software"),   *
*  to deal in the Software without restriction, including without limitation    *
*  the rights to use, copy, modify, merge, publish, distribute, sublicense,     *
*  and/or sell copies of the Software, and to permit persons to whom the        *
*  Software is furnished to do so, subject to the following conditions:         *
*                                                                               *
*  The above copyright notice and this permission notice shall be included in   *
*  all copies or substantial portions of the Software.                          *
*                                                                               *
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR   *
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,     *
*  FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT. IN NO EVENT SHALL THE *
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER       *
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING      *
*  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS *
*  IN THE SOFTWARE.                                                             *
*                                                                               *
*  Author(s):                                                                   *
*                                                                               *
*     Marco Livesu (marco.livesu@gmail.com)                                     *
*     http://pers.ge.imati.cnr.it/livesu/                                       *
*                                                                               *
*     Italian National Research Council (CNR)                                   *
*     Institute for Applied Mathematics and Information Technologies (IMATI)    *
*     Via de Marini, 6                                                          *
*     16149 Genoa,                                                              *
*     Italy                                                                     *
*********************************************************************************/
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/trackball.h>
#include <cinolib/how_many_seconds.h>
#include <cinolib/fonts/droid_sans.hpp>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_glfw.h>
#include <cinolib/gl/draw_arrow.h>
#include <cinolib/gl/draw_sphere.h>
#include <cinolib/gl/glproject.h>
#include <cinolib/gl/glunproject.h>
#include <cinolib/clamp.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <iomanip>
#include <cctype>
#include <algorithm>

namespace cinolib
{

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::notify_camera_change() const
{
    if (callback_camera_changed)
    {
        callback_camera_changed();
    }
}

CINO_INLINE
double GLcanvas::get_camera_speed_modifier() const
{
    const bool fast{ glfwGetKey(window, key_bindings.camera_faster) == GLFW_PRESS };
    const bool slow{ glfwGetKey(window, key_bindings.camera_slower) == GLFW_PRESS };
    return slow ? camera_settings.slower_factor : fast ? camera_settings.faster_factor : 1.0;
}

CINO_INLINE
void GLcanvas::clamp_camera_pivot()
{
    const double min_depth{ camera_settings.min_camera_pivot_distance_scene_radius_factor * m_sceneRadius };
    m_cameraPivotDepth = std::max(m_cameraPivotDepth, min_depth);
}

CINO_INLINE
void GLcanvas::handle_zoom(double amount, bool update_gl)
{
    amount *= get_camera_speed_modifier();
    if (camera.projection.perspective && glfwGetKey(window, key_bindings.camera_inplace_zoom) != GLFW_PRESS)
    {
        camera.view.eye += camera.view.normForward() * amount * m_sceneRadius;
        m_cameraPivotDepth -= amount * m_sceneRadius;
        clamp_camera_pivot();
        if (update_gl)
        {
            camera.updateView();
            update_GL_view();
        }
    }
    else
    {
        double min, max;
        if (camera.projection.perspective)
        {
            min = camera_settings.min_persp_fov;
            max = camera_settings.max_persp_fov;
        }
        else
        {
            const double camera_scene_radius{ m_sceneRadius ? m_sceneRadius : 1 };
            min = camera_settings.min_ortho_fov_scene_radius_factor * camera_scene_radius;
            max = camera_settings.max_ortho_fov_scene_radius_factor * camera_scene_radius;
        }
        double fov{ camera.projection.verticalFieldOfView - amount * (max - min) };
        camera.projection.verticalFieldOfView = clamp(fov, min, max);
        if (update_gl)
        {
            camera.updateProjection();
            update_GL_projection();
        }
    }
    if (update_gl)
    {
        draw();
        notify_camera_change();
    }
}

CINO_INLINE
void GLcanvas::handle_rotation(const vec2d& amount, bool update_gl)
{
    const vec2d angles{amount * get_camera_speed_modifier()};
    if (glfwGetKey(window, key_bindings.camera_inplace_rotation) == GLFW_PRESS)
    {
        camera.view.rotateFps(world_up, world_forward, angles.x(), angles.y());
    }
    else
    {
        camera.view.rotateTps(world_up, world_forward, m_cameraPivotDepth, angles.x(), angles.y());
    }
    if (update_gl)
    {
        camera.updateView();
        update_GL_view();
        draw();
        notify_camera_change();
    }
}

CINO_INLINE
void GLcanvas::handle_pan(const vec2d& amount, bool update_gl)
{
    vec3d translation{ 0,0,0 };
    translation += amount.x() * camera.view.normRight();
    translation += amount.y() * camera.view.normUp();
    translation *= m_sceneRadius * get_camera_speed_modifier();
    camera.view.eye += translation;
    if (update_gl)
    {
        camera.updateView();
        update_GL_view();
        draw();
        notify_camera_change();
    }
}

CINO_INLINE
void GLcanvas::handle_pan_and_zoom(const vec3d& amount, bool update_gl)
{
    handle_zoom(amount.z(), false);
    handle_pan(vec2d{amount.x(), amount.y()}, false);
    if (update_gl)
    {
        camera.updateProjectionAndView();
        update_GL_matrices();
        draw();
        notify_camera_change();
    }
}

CINO_INLINE
void GLcanvas::update_viewport(bool update_gl, bool redraw)
{
    camera.projection.setAspect(canvas_width(), m_height);
    if (update_gl)
    {
        glViewport(current_sidebar_width(), 0, canvas_width(), m_height);
        camera.updateProjection();
        update_GL_projection();
        if (redraw)
        {
            draw();
            notify_camera_change();
        }
    }
}

CINO_INLINE
int& GLcanvas::windowCount()
{
    static int windowCount{ 0 };
    return windowCount;
}

CINO_INLINE
GLFWwindow* GLcanvas::createWindow(int width, int height)
{
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow* const window{ glfwCreateWindow(width, height, "Cinolib", nullptr, nullptr) };
    if (!window) glfwTerminate();
    windowCount()++;
    return window;
}

CINO_INLINE
vec3d GLcanvas::scene_center() const
{
    return m_sceneCenter;
}
CINO_INLINE
float GLcanvas::scene_radius() const
{
    return m_sceneRadius;
}

CINO_INLINE
int GLcanvas::width() const
{
    return m_width;
}

CINO_INLINE
int GLcanvas::canvas_width() const
{
    return m_width - current_sidebar_width();
}

CINO_INLINE
int GLcanvas::height() const
{
    return m_height;
}

CINO_INLINE
int GLcanvas::sidebar_width() const
{
    return static_cast<int>(std::round(m_sidebarRelativeWidth * m_width));
}

CINO_INLINE
int GLcanvas::current_sidebar_width() const
{
    return m_showSidebar ? sidebar_width() : 0;
}

CINO_INLINE
void GLcanvas::sidebar_width(int width, bool update_gl, bool redraw)
{
    sidebar_relative_width(width * m_width, update_gl, redraw);
}

CINO_INLINE
float GLcanvas::sidebar_relative_width() const
{
    return m_sidebarRelativeWidth;
}

CINO_INLINE
void GLcanvas::sidebar_relative_width(float width, bool update_gl, bool redraw)
{
    const int old_width{ sidebar_width() };
    m_sidebarRelativeWidth = clamp(width, 0.2f, 0.8f);
    if (old_width != sidebar_width())
    {
        update_viewport(update_gl, redraw);
    }
}

CINO_INLINE
bool GLcanvas::show_sidebar() const
{
    return m_showSidebar;
}

CINO_INLINE
void GLcanvas::show_sidebar(bool show, bool update_gl, bool redraw)
{
    if (m_showSidebar != show)
    {
        m_showSidebar = show;
        update_viewport(update_gl, redraw);
    }
}

CINO_INLINE
double GLcanvas::dpi_factor() const
{
    return m_dpiFactor;
}

CINO_INLINE
double GLcanvas::camera_pivot_depth() const
{
    return m_cameraPivotDepth;
}

CINO_INLINE
void GLcanvas::camera_pivot_depth(double depth)
{
    m_cameraPivotDepth = depth;
    clamp_camera_pivot();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
GLcanvas::GLcanvas(const int width, const int height, const int font_size)
    : owns_ImGui{windowCount() == 0}, window{createWindow(width, height)}, m_width{width}, m_height{height}, font_size{font_size}
{

    glfwSwapInterval(1); // enable vsync

    // pass the window handler (i.e. this whole class) to the GLFW window,
    // so that I can access the methods of GLcanvas2from within the static
    // callbakcs defined within the class itself....
    // Despite ugly, I do not foresee better ways to interact with the
    // GLFW event handling system!
    // See:
    //      https://github.com/glfw/glfw/issues/815
    //      https://stackoverflow.com/questions/55145966/what-does-glfwgetwindowuserpointer-do
    glfwSetWindowUserPointer(window, (void*)this);

    // register GLFW callbacks
    glfwSetWindowSizeCallback (window, window_size_event );
    glfwSetKeyCallback        (window, key_event         );
    glfwSetMouseButtonCallback(window, mouse_button_event);
    glfwSetCursorPosCallback  (window, cursor_event      );
    glfwSetScrollCallback     (window, scroll_event      );

    // intialize OpenGL environment
    glfwMakeContextCurrent(window);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // to handle high DPI displays
    update_DPI_factor();

    if(owns_ImGui)
    {
        // initialize ImGui backend for visual controls...
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL2_Init();
        // NOTE: since ImGui does not support dynamic font sizing, I am using oversized fonts
        // (10x) to account for zoom factor (downscaling is visually better than upscaling)
        // https://github.com/ocornut/imgui/issues/797
        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = NULL;
        io.Fonts->Clear();
        io.Fonts->AddFontFromMemoryCompressedTTF(droid_sans_data, droid_sans_size, font_size*10.f);
        io.FontGlobalScale = 0.1f; // compensate for high-res fonts
    }

    std::cout << "---- GLcanvas key bindings ----\n";
    key_bindings.print();
    mouse_bindings.print();
    std::cout << "-------------------------------\n";

    reset_camera();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
GLcanvas::~GLcanvas()
{
    if(owns_ImGui)
    {
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::push(const DrawableObject *obj, const bool reset_camera)
{
    drawlist.push_back(obj);

    if(reset_camera && obj->scene_radius_transformed()>0)
    {
        this->reset_camera();
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::push(SideBarItem *item)
{
    side_bar_items.push_back(item);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::push(CanvasGuiItem* item)
{
    canvas_gui_items.push_back(item);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::push(const Marker & m)
{
    // ImGui uses 16 bits to index items. It cannot handle more than this
    //if(markers.size() >= (1 << 16)) return;
    if (marker_sets.empty())
    {
        marker_sets.push_back({});
    }
    marker_sets[0].push_back(m);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::push_marker(const vec2d       &  p,
                           const std::string &  text,
                           const Color          color,
                           const unsigned int   shape_radius,
                           const unsigned int   font_size)
{
    Marker m;
    m.pos_2d        = p;
    m.text          = text;
    m.color         = color;
    m.shape_radius  = shape_radius;
    m.font_size     = font_size;
    push(m);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::push_marker(const vec3d       &  p,
                           const std::string &  text,
                           const Color          color,
                           const unsigned int   shape_radius,
                           const unsigned int   font_size)
{
    Marker m;
    m.pos_3d        = p;
    m.text          = text;
    m.color         = color;
    m.shape_radius  = shape_radius;
    m.font_size     = font_size;
    push(m);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
bool GLcanvas::pop(const DrawableObject *obj)
{
    // pops the first occurrence of obj (if found)
    for(std::vector<const DrawableObject*>::iterator it=drawlist.begin(); it!=drawlist.end(); ++it)
    {
        if(obj==*it)
        {
            drawlist.erase(it);
            return true;
        }
    }
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::pop_all_markers()
{
    if (!marker_sets.empty())
    {
        marker_sets[0].clear();
        marker_sets.resize(1);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::refit_scene(bool update_gl, bool redraw)
{
    m_sceneCenter = vec3d{0, 0, 0};
    m_sceneRadius = 0;
    clamp_camera_pivot();
    unsigned int count = 0;
    for (auto obj : drawlist)
    {
        const double radius{ obj->scene_radius_transformed() };
        if (radius > 0)
        {
            m_sceneCenter += obj->scene_center_transformed();
            if (radius > m_sceneRadius)
            {
                m_sceneRadius = radius;
            }
            ++count;
        }
    }
    if (count)
    {
        m_sceneCenter /= static_cast<double>(count);
    }
    const double camera_scene_radius{ m_sceneRadius ? m_sceneRadius : 1 };
    camera.projection.nearZ = camera_scene_radius * camera_settings.near_scene_radius_factor;
    camera.projection.farZ = camera_scene_radius * camera_settings.far_scene_radius_factor;
    if (update_gl)
    {
        camera.updateProjection();
        update_GL_projection();
        if (redraw)
        {
            draw();
            notify_camera_change();
        }
    }
}

CINO_INLINE
void GLcanvas::reset_camera(bool update_gl, bool redraw)
{
    refit_scene(false);
    update_viewport(false);
    camera.projection.perspective = true;
    camera.projection.verticalFieldOfView = (camera_settings.min_persp_fov + camera_settings.max_persp_fov) / 2.0;
    const double camera_scene_radius{ m_sceneRadius ? m_sceneRadius : 1 };
    const double distance{ camera_scene_radius * camera_settings.camera_distance_scene_radius_factor };
    camera.view = FreeCamera<double>::View::tps(world_up, world_forward, m_sceneCenter, distance, 180, 0);
    camera_pivot_depth(m_sceneCenter.dist(camera.view.eye));
    if (update_gl)
    {
        camera.updateProjectionAndView();
        update_GL_matrices();
        if (redraw)
        {
            draw();
            notify_camera_change();
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::update_GL_matrices() const
{
    update_GL_view();
    update_GL_projection();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::update_GL_view() const
{
    glfwMakeContextCurrent(window);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd((camera.viewMatrix()).transpose().ptr());
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::update_GL_projection() const
{
    glfwMakeContextCurrent(window);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(camera.projectionMatrix().transpose().ptr());
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::draw()
{
    if (m_drawing)
    {
        return;
    }
    m_drawing = true;
    glfwMakeContextCurrent(window);
    glClearColor(background.r(), background.g(), background.b(), 1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // draw your 3D scene
    for(auto obj : drawlist) obj->draw_transformed();

    if(show_axis) draw_axis();

    if(owns_ImGui)
    {
        // draw markers and visual controls
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        draw_markers();
        draw_custom_gui();
        if(m_showSidebar) draw_side_bar();
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    }

    glfwSwapBuffers(window);
    m_drawing = false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::draw_axis() const
{
    vec3d  O = m_sceneCenter;
    vec3d  X = O + world_right * m_sceneRadius;
    vec3d  Y = O + world_up * m_sceneRadius;
    vec3d  Z = O + world_forward * m_sceneRadius;
    double r = m_sceneRadius * 0.02;
    glfwMakeContextCurrent(window);
    glDisable(GL_DEPTH_TEST);
    draw_arrow(O, X, r, Color::RED(),   0.9, 0.5, 8);
    draw_arrow(O, Y, r, Color::GREEN(), 0.9, 0.5, 8);
    draw_arrow(O, Z, r, Color::BLUE(),  0.9, 0.5, 8);
    draw_sphere(O, r, Color::WHITE(), 1);
    glEnable(GL_DEPTH_TEST);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

// WARNING: depth culling 3D markers seems buggy and may crash!
// There is some weird memory leak involving buffer allocation
// and Z-buffer access. I could not entirely fix it...
//
CINO_INLINE
void GLcanvas::draw_markers() const
{
    assert(owns_ImGui && "Only the first canvas created handles the ImGui context");

    if (std::all_of(marker_sets.begin(), marker_sets.end(), [](auto& ms) { return ms.empty(); }))
    {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    bool shown = true;
    ImGui::Begin("Markers", &shown, ImGuiWindowFlags_NoTitleBar            |
                                      ImGuiWindowFlags_NoResize              |
                                      ImGuiWindowFlags_NoMove                |
                                      ImGuiWindowFlags_NoScrollbar           |
                                      ImGuiWindowFlags_NoBringToFrontOnFocus |
                                      ImGuiWindowFlags_NoScrollWithMouse     |
                                      ImGuiWindowFlags_NoCollapse            |
                                      ImGuiWindowFlags_NoSavedSettings       |
                                      ImGuiWindowFlags_NoInputs);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // if marker culling is enabled, read the Z buffer to depth-test 3D markers
    GLint    W     = static_cast<GLint>(m_width  * m_dpiFactor);
    GLint    H     = static_cast<GLint>(m_height * m_dpiFactor);
    GLfloat *z_buf = (depth_cull_markers) ? new GLfloat[W*H] : nullptr;
    if(depth_cull_markers) whole_Z_buffer(z_buf);

    for (const std::vector<Marker>& marker_set : marker_sets)
    {
        for (const Marker& m : marker_set)
        {
            if (!m.enabled)
            {
                continue;
            }
            vec2d pos = m.pos_2d;
            if (pos.is_inf()) // 3D marker
            {
                assert(!m.pos_3d.is_inf());
                GLdouble z;
                project(m.pos_3d, pos, z);
                int x = static_cast<int>(pos.x() * m_dpiFactor);
                int y = static_cast<int>(pos.y() * m_dpiFactor);
                // marker is outside the frustum
                if (z < 0 || z >= 1 || x <= 0 || x >= W || y <= 0 || y >= H) continue;
                // marker is occluded in the current view
                assert((x + W * (H - y - 1) < W * H));
                if (depth_cull_markers && fabs(z - z_buf[x + W * (H - y - 1)]) > 0.01) continue;
            }

            const ImU32 color{ ImGui::GetColorU32(ImVec4(m.color.r(), m.color.g(), m.color.b(), m.color.a())) };
            const bool drawShape{m.shape_radius > 0 && (m.shape == Marker::EShape::CircleFilled || m.line_thickness > 0)};
            if (drawShape)
            {
                static constexpr int segCount{ 6 };
                const ImVec2 center{ static_cast<float>(pos.x()), static_cast<float>(pos.y()) };
                switch (m.shape)
                {
                    case Marker::EShape::CircleFilled:
                        drawList->AddCircleFilled(center, m.shape_radius, color, segCount);
                        break;
                    case Marker::EShape::CircleOutline:
                        drawList->AddCircle(center, m.shape_radius, color, segCount, m.line_thickness);
                        break;
                    case Marker::EShape::Cross90:
                        drawList->AddLine({center.x - m.shape_radius, center.y}, { center.x + m.shape_radius, center.y }, color, m.line_thickness);
                        drawList->AddLine({center.x, center.y - m.shape_radius}, { center.x, center.y + m.shape_radius }, color, m.line_thickness);
                        break;
                    case Marker::EShape::Cross45:
                        drawList->AddLine({center.x - m.shape_radius, center.y - m.shape_radius}, { center.x + m.shape_radius, center.y + m.shape_radius }, color, m.line_thickness);
                        drawList->AddLine({center.x - m.shape_radius, center.y + m.shape_radius}, { center.x + m.shape_radius, center.y - m.shape_radius }, color, m.line_thickness);
                        break;
                }
            }
            if (m.font_size > 0 && m.text.length() > 0)
            {
                ImVec2 offset;
                if (drawShape)
                {
                    offset = { static_cast<float>(m.shape_radius), static_cast<float>(m.shape_radius) };
                }
                else
                {
                    const ImVec2 normTextSize{ ImGui::CalcTextSize(&m.text[0], &m.text[0] + m.text.size()) };
                    const ImVec2 textSize{ normTextSize.x * m.font_size / ImGui::GetFontSize(), normTextSize.y * m.font_size / ImGui::GetFontSize() };
                    offset = { -textSize.x / 2, -textSize.y / 2 };
                }
                drawList->AddText(ImGui::GetFont(),
                    m.font_size,
                    ImVec2(pos.x() + offset.x, pos.y() + offset.y),
                    color,
                    &m.text[0],
                    &m.text[0] + m.text.size());
            }
        }
    }
    if(depth_cull_markers) delete[] z_buf;
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::draw_side_bar()
{
    if(callback_app_controls==nullptr && side_bar_items.empty()) return;

    assert(owns_ImGui && "Only the first canvas created handles the ImGui context");

    ImGui::SetNextWindowPos({0,0}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({ m_sidebarRelativeWidth * m_width, m_height*1.f}, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(1.0f);
    bool shown{ true };
    ImGui::Begin("Sidebar", &shown,  
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse 
    );
    if(callback_app_controls!=nullptr)
    {
        if(ImGui::TreeNode("App controls"))
        {
            callback_app_controls();
            ImGui::TreePop();
        }
    }
    for(auto item : side_bar_items)
    {
        ImGui::PushID(item);
        ImGui::SetNextItemOpen(item->show_open, ImGuiCond_Always);
        if((item->show_open = ImGui::TreeNode("%s##cinolib_glcanvas_sidebar_item_title", item->name.c_str())))
        {
            item->draw();
            ImGui::TreePop();
        }
        ImGui::PopID();
    }    
    // this allows the user to interactively resize the width of the side bar
    const float relativeWidth = ImGui::GetWindowWidth() / m_width;
    if (relativeWidth < 0.05f)
    {
        show_sidebar(false, true, false);
    }
    sidebar_relative_width(relativeWidth, true, false);
    ImGui::End();        
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::draw_custom_gui() const
{
    if (callback_custom_gui == nullptr && canvas_gui_items.empty()) return;

    assert(owns_ImGui && "Only the first canvas created handles the ImGui context");

    ImGui::SetNextWindowPos({ current_sidebar_width() * 1.f, 0 }, ImGuiCond_Always);
    ImGui::SetNextWindowSize({ canvas_width() * 1.f, m_height * 1.f}, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

    bool shown = true;
    ImGui::Begin("Custom GUI", &shown, 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoInputs);

    if (callback_custom_gui)
    {
        callback_custom_gui();
    }

    for (CanvasGuiItem* item : canvas_gui_items)
    {
        ImGui::PushID(item);
        item->draw();
        ImGui::PopID();
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
int GLcanvas::launch(std::initializer_list<GLcanvas*> additional_windows)
{
    while(true)
    {
        glfwMakeContextCurrent(window);
        draw();
        if(glfwWindowShouldClose(window)) return EXIT_SUCCESS;

        for(auto it=additional_windows.begin(); it!=additional_windows.end(); ++it)
        {
            glfwMakeContextCurrent((*it)->window);
            (*it)->draw();
            if(glfwWindowShouldClose((*it)->window)) return EXIT_SUCCESS;
        }

        glfwPollEvents();
    }
    return EXIT_SUCCESS;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::update_DPI_factor()
{
    // https://www.glfw.org/docs/latest/intro_guide.html#coordinate_systems
    // https://www.glfw.org/docs/latest/window_guide.html#window_fbsize
    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    m_dpiFactor = (float)fb_width/m_width;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
vec2d GLcanvas::cursor_pos() const
{
    vec2d pos;
    glfwGetCursorPos(window, &pos.x(), &pos.y());
    return pos;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::whole_Z_buffer(GLfloat * buf) const
{
    glfwMakeContextCurrent(window);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glReadPixels(viewport[0], viewport[1], viewport[2], viewport[3], GL_DEPTH_COMPONENT, GL_FLOAT, buf);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
GLfloat GLcanvas::query_Z_buffer(const vec2d & p) const
{
    glfwMakeContextCurrent(window);
    GLint x = p.x()         * m_dpiFactor;
    GLint y = p.y()         * m_dpiFactor;
    GLint H = m_height * m_dpiFactor;
    GLfloat depth;
    glReadPixels(x, H-1-y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    return depth;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
bool GLcanvas::unproject(const vec2d & p2d, vec3d & p3d) const
{
    // retrieve the missing z coordinate from the Z buffer, then unproject
    return unproject(p2d, (double)query_Z_buffer(p2d), p3d);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

// this is equivalent to gluUnproject
// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluUnProject.xml
//
CINO_INLINE
bool GLcanvas::unproject(const vec2d & p2d, const GLdouble & depth, vec3d & p3d) const
{
    mat2i viewport({current_sidebar_width(),    m_height,
                    canvas_width(),            -m_height });

    return gl_unproject(p2d, depth, camera.viewMatrix(), camera.projectionMatrix(), viewport, p3d);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::project(const vec3d & p3d, vec2d & p2d, GLdouble & depth) const
{
    mat2i viewport({current_sidebar_width(),    m_height,
                    canvas_width(),            -m_height});

    gl_project(p3d, camera.viewMatrix(), camera.projectionMatrix(), viewport, p2d, depth);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
Ray GLcanvas::eye_to_screen_ray(const vec2d& p2d) const
{
    const vec4d clipHomPos{ 
        (p2d.x() - static_cast<double>(current_sidebar_width())) / static_cast<double>(canvas_width()) * 2.0 - 1.0,
        1.0 - p2d.y() / static_cast<double>(m_height) * 2.0,
        0.0, 
        1.0 
    };
    const vec4d nearHomPos{ camera.projectionViewMatrix().inverse() * clipHomPos };
    const vec3d nearPos{ (nearHomPos / nearHomPos[3]).rem_coord()};
    if (camera.projection.perspective)
    {
        return Ray{ camera.view.eye, (nearPos - camera.view.eye).normalized() };
    }
    else
    {
        return Ray{ nearPos, camera.view.normForward() };
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
Ray GLcanvas::eye_to_mouse_ray() const
{
    vec2d pos;
    glfwGetCursorPos(window, &pos.x(), &pos.y());
    return eye_to_screen_ray(pos);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::::::::::::::::::::::: STATIC GLFW CALLBACKS :::::::::::::::::::::::::
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::window_size_event(GLFWwindow *window, int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        return;
    }
    GLcanvas* v = static_cast<GLcanvas*>(glfwGetWindowUserPointer(window));
    v->m_height           = height;
    v->m_width            = width;
    v->update_DPI_factor();
    v->update_viewport();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::key_event(GLFWwindow* window, int key, int /*scancode*/, int action, int modifiers)
{
    // if visual controls claim the event, let them handle it!
    if (ImGui::GetIO().WantCaptureKeyboard) return;


    // handle repeated keys as if they were a sequence of single key press events
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        GLcanvas* v = static_cast<GLcanvas*>(glfwGetWindowUserPointer(window));

        if (v->callback_key_pressed && v->callback_key_pressed(key, modifiers))
        {
            return;
        }

        {
            vec3d amount{ 0,0,0 };
            if (v->key_bindings.pan_with_arrow_keys)
            {
                switch (key)
                {
                    case GLFW_KEY_LEFT:     amount += vec3d{-1, 0, 0 }; break;
                    case GLFW_KEY_RIGHT:    amount += vec3d{ 1, 0, 0 }; break;
                    case GLFW_KEY_UP:       amount += vec3d{ 0, 1, 0 }; break;
                    case GLFW_KEY_DOWN:     amount += vec3d{ 0,-1, 0 }; break;
                }
            }
            if (v->key_bindings.pan_and_zoom_with_numpad_keys)
            {
                switch (key)
                {
                    case GLFW_KEY_KP_0:     amount += vec3d{ 0, 0,-1 }; break;
                    case GLFW_KEY_KP_1:     amount += vec3d{-1,-1, 0 }; break;
                    case GLFW_KEY_KP_2:     amount += vec3d{ 0,-1, 0 }; break;
                    case GLFW_KEY_KP_3:     amount += vec3d{ 1,-1, 0 }; break;
                    case GLFW_KEY_KP_4:     amount += vec3d{-1, 0, 0 }; break;
                    case GLFW_KEY_KP_5:     amount += vec3d{ 0, 0, 1 }; break;
                    case GLFW_KEY_KP_6:     amount += vec3d{ 1, 0, 0 }; break;
                    case GLFW_KEY_KP_7:     amount += vec3d{-1, 1, 0 }; break;
                    case GLFW_KEY_KP_8:     amount += vec3d{ 0, 1, 0 }; break;
                    case GLFW_KEY_KP_9:     amount += vec3d{ 1, 1, 0 }; break;
                }
                if (!amount.is_null())
                {
                    amount.normalize();
                    amount *= v->camera_settings.translate_key_speed / 50;
                    v->handle_pan_and_zoom(amount);
                }
            }
        }

        if (action == GLFW_PRESS)
        {
            const KeyBinding binding{ key, modifiers };
            static const std::string camera_clipboard_token{ "cinolib_gl_glcanvas_camera\n" };
            if (binding == v->key_bindings.store_camera)
            {
                constexpr char sep{ ' ' };
                std::stringstream stream{};
                stream 
                    << camera_clipboard_token
                    << v->camera << sep
                    << v->m_cameraPivotDepth;
                glfwSetClipboardString(window, stream.str().c_str());
            }
            if (binding == v->key_bindings.restore_camera)
            {
                const char* const clipboard{ glfwGetClipboardString(window) };
                if (clipboard && std::strstr(clipboard, camera_clipboard_token.c_str()) == clipboard)
                {
                    std::stringstream{ clipboard + camera_clipboard_token.length() }
                        >> v->camera
                        >> v->m_cameraPivotDepth;
                    v->clamp_camera_pivot();
                    v->update_viewport(false);
                    v->camera.updateProjectionAndView();
                    v->update_GL_matrices();
                    v->draw();
                    v->notify_camera_change();
                }
            }
            if (binding == v->key_bindings.reset_camera)
            {
                v->reset_camera();
            }
            if (binding == v->key_bindings.look_at_center)
            {
                v->camera.view.lookAt(v->m_sceneCenter);
                // get rid of roll
                v->camera.view.rotateFps(world_up, world_forward, 0,0); // TODO (francescozoccheddu) this is ugly
                v->camera_pivot_depth(v->m_sceneCenter.dist(v->camera.view.eye));
                v->camera.updateView();
                v->update_GL_view();
                v->draw();
                v->notify_camera_change();
            }
            {
                bool cameraUpdated{ true };
                double yaw{}, pitch{};
                static constexpr double maxPitch{ 89 }; // neither gimbal lock nor dutch angle, please
                if (binding == v->key_bindings.camera_look_at_minus_x)
                {
                    yaw = 90;
                }
                else if (binding == v->key_bindings.camera_look_at_minus_y)
                {
                    pitch = -maxPitch;
                }
                else if (binding == v->key_bindings.camera_look_at_minus_z)
                {
                    yaw = 0;
                }
                else if (binding == v->key_bindings.camera_look_at_plus_x)
                {
                    yaw = -90;
                }
                else if (binding == v->key_bindings.camera_look_at_plus_y)
                {
                    pitch = maxPitch;
                }
                else if (binding == v->key_bindings.camera_look_at_plus_z)
                {
                    yaw = 180;
                }
                else
                {
                    cameraUpdated = false;
                }
                if (cameraUpdated)
                {
                    v->camera.view = FreeCamera<double>::View::tps(world_up, world_forward, v->m_sceneCenter, v->m_cameraPivotDepth, yaw, pitch);
                    v->camera.updateView();
                    v->update_GL_view();
                    v->draw();
                    v->notify_camera_change();
                }
            }
            if (binding == v->key_bindings.toggle_axes)
            {
                v->show_axis ^= true;
                v->draw();
            }
            if (binding == v->key_bindings.toggle_ortho)
            {
                v->camera.projection.perspective ^= true;
                v->camera.projection.verticalFieldOfView = (v->camera.projection.perspective 
                    ? v->camera_settings.min_persp_fov + v->camera_settings.max_persp_fov
                    : v->m_sceneRadius * (v->camera_settings.min_ortho_fov_scene_radius_factor + v->camera_settings.max_ortho_fov_scene_radius_factor)) 
                    / 2;
                v->camera.updateProjection();
                v->update_GL_projection();
                v->draw();
                v->notify_camera_change();
            }
            if (binding == v->key_bindings.toggle_sidebar)
            {
                v->show_sidebar(!v->m_showSidebar);
            }
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::mouse_button_event(GLFWwindow* window, int button, int action, int modifiers)
{
    // if visual controls claim the event, let them handle it
    if (ImGui::GetIO().WantCaptureMouse) return;

    GLcanvas* v = static_cast<GLcanvas*>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS)
    {

        // thanks GLFW for asking me to handle the single/double click burden...
        auto double_click = [&]() -> bool
        {
            auto   t = std::chrono::high_resolution_clock::now();
            double dt = how_many_seconds(v->m_trackball.t_last_click, t);
            v->m_trackball.t_last_click = t;
            return (dt < 0.2);
        };


        if (double_click())
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT)
            {
                if (v->callback_mouse_left_click2 && v->callback_mouse_left_click2(modifiers))
                {
                    return;
                }
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT)
            {
                if (v->callback_mouse_right_click2 && v->callback_mouse_right_click2(modifiers))
                {
                    return;
                }
            }
        }
        else // single click
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT)
            {
                if (v->callback_mouse_left_click && v->callback_mouse_left_click(modifiers))
                {
                    return;
                }
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT)
            {
                if (v->callback_mouse_right_click && v->callback_mouse_right_click(modifiers))
                {
                    return;
                }
            }
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::cursor_event(GLFWwindow* window, double x_pos, double y_pos)
{

    // if visual controls claim the event, let them handle it
    if (ImGui::GetIO().WantCaptureMouse) return;

    GLcanvas* v = static_cast<GLcanvas*>(glfwGetWindowUserPointer(window));
    const vec2d cursor_pos{ v->cursor_pos() };
    const vec2d delta{ cursor_pos - v->m_trackball.last_cursor_pos };
    v->m_trackball.last_cursor_pos = cursor_pos;

    if (v->callback_mouse_moved && v->callback_mouse_moved(x_pos, y_pos))
    {
        return;
    }

    if (!delta.is_inf())
    {
        if (glfwGetMouseButton(window, v->mouse_bindings.camera_rotate) == GLFW_PRESS)
        {
            const vec2d amount{ -delta * v->camera_settings.rotate_drag_speed / 10 };
            v->handle_rotation(amount);
        }

        if (glfwGetMouseButton(window, v->mouse_bindings.camera_zoom) == GLFW_PRESS)
        {
            const double amount{ delta.y() * v->camera_settings.zoom_drag_speed / 300 };
            v->handle_zoom(-amount);
        }

        if (glfwGetMouseButton(window, v->mouse_bindings.camera_pan) == GLFW_PRESS)
        {
            const vec2d amount{ delta * v->camera_settings.pan_drag_speed / 200 };
            v->handle_pan(vec2d{ -amount.x(), amount.y() });
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::scroll_event(GLFWwindow* window, double x_offset, double y_offset)
{
    GLcanvas* v = static_cast<GLcanvas*>(glfwGetWindowUserPointer(window));

    // if visual controls claim the event, let them handle it
    if (ImGui::GetIO().WantCaptureMouse) return;

    if (v->callback_mouse_scroll && v->callback_mouse_scroll(x_offset, y_offset))
    {
        return;
    }

    if (v->mouse_bindings.zoom_with_wheel)
    {
        const double amount{ y_offset * v->camera_settings.zoom_scroll_speed / 20 };
        v->handle_zoom(amount);
    }
}

}
