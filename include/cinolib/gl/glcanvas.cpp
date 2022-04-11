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
#include <cinolib/fonts/droid_sans.h>
#include <../external/imgui/imgui_impl_opengl2.h>
#include <../external/imgui/imgui_impl_glfw.h>
#include <cinolib/gl/draw_arrow.h>
#include <cinolib/gl/draw_sphere.h>
#include <cinolib/gl/glproject.h>
#include <cinolib/gl/glunproject.h>
#include <cinolib/clamp.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cmath>

namespace cinolib
{

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

const int GLcanvas::KeyBindings::none{ 0 };
const int GLcanvas::MouseBindings::none{ 0 };

CINO_INLINE
void GLcanvas::KeyBindings::print() const
{
    auto binding = [](const char* binding, const char* desc)
    {
        std::cout << binding << ": " << desc << '\n';
    };
    auto key = [&binding](int key, const char* desc)
    {
        if (key != none)
        {
            const char* key_name{ glfwGetKeyName(key, 0) }; // TODO (francescozoccheddu) not working for non-character keys
            if (key_name)
            {
                binding(key_name, desc);
            }
        }
    };
    key(toggle_sidebar, "toggle sidebar");
    key(toggle_axes, "toggle axes");
    key(toggle_ortho, "toggle perspective/orthographic camera");
    key(reset_camera, "reset camera");
    key(look_at_center, "look at center");
    key(store_camera, "copy camera to clipboard");
    key(restore_camera, "restore camera from clipboard");
    key(camera_faster, "move camera faster (hold down)");
    key(camera_slower, "move camera slower (hold down)");
    key(camera_inplace_zoom, "change fov instead of moving forward when zooming (hold down)");
    key(camera_inplace_rotation, "rotate camera around itself instead of the center (hold down)");
    if (pan_with_arrow_keys)
    {
        binding("arrows", "pan");
    }
    if (pan_and_zoom_with_numpad_keys)
    {
        binding("numpad", "pan and zoom");
    }
}

void GLcanvas::MouseBindings::print() const
{
    auto binding = [](const char* binding, const char* desc)
    {
        std::cout << binding << ": " << desc << '\n';
    };
    auto button = [&binding](int button, const char* desc)
    {
        switch (button)
        {
            case GLFW_MOUSE_BUTTON_LEFT:
                binding("left mouse button", desc);
                break;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                binding("middle mouse button", desc);
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                binding("right mouse button", desc);
                break;
        }
    };
    button(camera_pan, "pan (drag)");
    button(camera_zoom, "zoom (drag)");
    button(camera_rotate, "rotate camera (drag)");
    if (zoom_with_wheel)
    {
        binding("scroll wheel", "zoom");
    }
}

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
void GLcanvas::handle_zoom(double _amount)
{
    _amount *= get_camera_speed_modifier();
    if (camera.projection.perspective && glfwGetKey(window, key_bindings.camera_inplace_zoom) != GLFW_PRESS)
    {
        camera.view.eye += camera.view.normForward() * _amount * scene_radius;
        const double min_pivot_distance{ camera_settings.min_camera_pivot_distance_scene_radius_factor * scene_radius };
        if ((camera_pivot - camera.view.eye).dot(camera.view.normForward()) < min_pivot_distance)
        {
            camera_pivot = camera.view.centerAt(min_pivot_distance);
        }
        camera.updateView();
        update_GL_view();
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
            const double camera_scene_radius{ scene_radius ? scene_radius : 1 };
            min = camera_settings.min_ortho_fov_scene_radius_factor * camera_scene_radius;
            max = camera_settings.max_ortho_fov_scene_radius_factor * camera_scene_radius;
        }
        double fov{ camera.projection.verticalFieldOfView + _amount * (max - min) };
        camera.projection.verticalFieldOfView = clamp(fov, min, max);
        camera.updateProjection();
        update_GL_projection();
    }
    draw();
    notify_camera_change();
}

CINO_INLINE
void GLcanvas::handle_rotation(const vec2d& amount)
{
    const vec2d angles(amount * get_camera_speed_modifier());
    const double distance{ camera.view.eye.dist(camera_pivot) };
    if (glfwGetKey(window, key_bindings.camera_inplace_rotation) == GLFW_PRESS)
    {
        camera.view.rotateFps(world_up, world_forward, angles.x(), angles.y());
    }
    else
    {
        camera.view.rotateTps(world_up, world_forward, distance, angles.x(), angles.y());
    }
    camera_pivot = camera.view.centerAt(distance);
    camera.updateView();
    update_GL_view();
    draw();
    notify_camera_change();
}

CINO_INLINE
void GLcanvas::handle_pan(const vec2d& amount)
{
    vec3d translation{ 0,0,0 };
    translation += amount.x() * camera.view.normRight();
    translation += amount.y() * camera.view.normUp();
    translation *= scene_radius * get_camera_speed_modifier();
    camera_pivot += translation;
    camera.view.eye += translation;
    camera.updateView();
    update_GL_view();
    draw();
    notify_camera_change();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

const vec3d GLcanvas::world_right{ 1,0,0 }, GLcanvas::world_up{ 0,1,0 }, GLcanvas::world_forward{ 0,0,-1 };

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
GLcanvas::GLcanvas(const int width, const int height)
{
    // make sure that only the first window
    // in the app creates a ImGui context
    static int window_count = 0;
    owns_ImGui = (window_count==0);
    window_count++;

    glfwInit();
    window = glfwCreateWindow(width, height, "", NULL, NULL);
    if(!window) glfwTerminate();

    this->width = width;
    this->height = height;

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
        ImGui_ImplGlfw_InitForOpenGL(window, false);
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

    std::cout << "--------------\n";
    key_bindings.print();
    mouse_bindings.print();
    std::cout << "--------------\n";

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

    if(reset_camera && obj->scene_radius()>0)
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
void GLcanvas::push(const Marker & m)
{
    // ImGui uses 16 bits to index items. It cannot handle more than this
    //if(markers.size() >= (1 << 16)) return;
    markers.push_back(m);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::push_marker(const vec2d       & p,
                           const std::string & text,
                           const Color         color,
                           const unsigned int          disk_radius,
                           const unsigned int          font_size)
{
    Marker m;
    m.pos_2d      = p;
    m.text        = text;
    m.color       = color;
    m.disk_radius = disk_radius;
    m.font_size   = font_size;
    push(m);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::push_marker(const vec3d       & p,
                           const std::string & text,
                           const Color         color,
                           const unsigned int          disk_radius,
                           const unsigned int          font_size)
{
    Marker m;
    m.pos_3d      = p;
    m.text        = text;
    m.color       = color;
    m.disk_radius = disk_radius;
    m.font_size   = font_size;
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
    markers.clear();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::refit_scene(bool update_gl)
{
    scene_center = vec3d{0, 0, 0};
    camera_pivot = scene_center;
    scene_radius = 0;
    unsigned int count = 0;
    for (auto obj : drawlist)
    {
        if (obj->scene_radius() > 0)
        {
            scene_center += obj->scene_center();
            scene_radius += obj->scene_radius();
            ++count;
        }
    }
    if (count)
    {
        scene_center /= static_cast<double>(count);
        scene_radius /= static_cast<double>(count);
    }
    const double camera_scene_radius{ scene_radius ? scene_radius : 1 };
    camera.projection.nearZ = camera_scene_radius * camera_settings.near_scene_radius_factor;
    camera.projection.farZ = camera_scene_radius * camera_settings.far_scene_radius_factor;
    if (update_gl)
    {
        camera.updateProjection();
        update_GL_projection();
        draw();
        notify_camera_change();
    }
}

CINO_INLINE
void GLcanvas::reset_camera(bool update_gl)
{
    refit_scene(false);
    camera.projection.setAspect(width, height);
    camera.projection.perspective = true;
    camera.projection.verticalFieldOfView = (camera_settings.min_persp_fov + camera_settings.max_persp_fov) / 2.0;
    const double camera_scene_radius{ scene_radius ? scene_radius : 1 };
    const double distance{ camera_scene_radius * camera_settings.camera_distance_scene_radius_factor };
    camera.view = FreeCamera<double>::View::tps(world_up, world_forward, scene_center, distance, 0, 0);

    if (update_gl)
    {
        camera.updateProjectionAndView();
        update_GL_matrices();
        draw();
        notify_camera_change();
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
    glfwMakeContextCurrent(window);
    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // draw your 3D scene
    for(auto obj : drawlist) obj->draw();

    if(show_axis) draw_axis();

    if(owns_ImGui)
    {
        // draw markers and visual controls
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        draw_markers();
        if(show_side_bar) draw_side_bar();
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    }

    glfwSwapBuffers(window);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::draw_axis() const
{
    vec3d  O = scene_center;
    vec3d  X = O + world_right * scene_radius;
    vec3d  Y = O + world_up * scene_radius;
    vec3d  Z = O + world_forward * scene_radius;
    double r = scene_radius*0.02;
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

    if(markers.empty()) return;

    ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    bool visible = true;
    ImGui::Begin("Markers", &visible, ImGuiWindowFlags_NoTitleBar            |
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
    GLint    W     = static_cast<GLint>(width  * DPI_factor);
    GLint    H     = static_cast<GLint>(height * DPI_factor);
    GLfloat *z_buf = (depth_cull_markers) ? new GLfloat[W*H] : nullptr;
    if(depth_cull_markers) whole_Z_buffer(z_buf);

    for(const Marker & m : markers)
    {
        vec2d pos = m.pos_2d;
        if(pos.is_inf()) // 3D marker
        {
            assert(!m.pos_3d.is_inf());
            GLdouble z;
            project(m.pos_3d, pos, z);
            int x  = static_cast<int>(pos.x()*DPI_factor);
            int y  = static_cast<int>(pos.y()*DPI_factor);
            // marker is outside the frustum
            if(z<0 || z>=1 || x<=0 || x>=W || y<=0 || y>=H) continue;
            // marker is occluded in the current view
            assert((x+W*(H-y-1)<W*H));
            if(depth_cull_markers && fabs(z-z_buf[x+W*(H-y-1)])>0.01) continue;
        }

        //
        if(m.disk_radius>0)
        {
            // ImGui uses 16bits to index vertices, hence it is likely to overflow if there are too many markers.
            //  - I noticed that the automatic segment count immediately triggers overflow with large amounts of zoom,
            //    so I am always approximating circles with 20-gons
            //  - Besides, I should probably use something like AddSquare or AddRect to save limit polygon vertices....
            drawList->AddCircleFilled(ImVec2(pos.x(),pos.y()),
                                      m.disk_radius,
                                      ImGui::GetColorU32(ImVec4(m.color.r, m.color.g, m.color.b, m.color.a)), 20);
        }
        if(m.font_size>0 && m.text.length()>0)
        {
            drawList->AddText(ImGui::GetFont(),
                              m.font_size,
                              ImVec2(pos.x()+m.disk_radius, pos.y()-2*m.disk_radius),
                              ImGui::GetColorU32(ImVec4(m.color.r, m.color.g, m.color.b, m.color.a)),
                              &m.text[0],
                              &m.text[0] + m.text.size());
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
    ImGui::SetNextWindowSize({width*side_bar_width, height*1.f}, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(side_bar_alpha);
    ImGui::Begin("MENU");
    if(callback_app_controls!=nullptr)
    {
        ImGui::SetNextItemOpen(true,ImGuiCond_Once);
        if(ImGui::TreeNode("APP CONTROLS"))
        {
            callback_app_controls();
            ImGui::TreePop();
        }
    }
    for(auto item : side_bar_items)
    {
        ImGui::SetNextItemOpen(item->show_open,ImGuiCond_Once);
        if(ImGui::TreeNode(item->name.c_str()))
        {
            item->draw();
            ImGui::TreePop();
        }
    }    
    // this allows the user to interactively resize the width of the side bar
    side_bar_width = ImGui::GetWindowWidth() / width;
    ImGui::End();        
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
    DPI_factor = (float)fb_width/width;
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
    GLint x = p.x()         * DPI_factor;
    GLint y = p.y()         * DPI_factor;
    GLint H = height * DPI_factor;
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
    mat2i viewport({ 0,      height,
                     width, -height});

    return gl_unproject(p2d, depth, camera.viewMatrix(), camera.projectionMatrix(), viewport, p3d);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void GLcanvas::project(const vec3d & p3d, vec2d & p2d, GLdouble & depth) const
{
    mat2i viewport({0,             height,
                    width, -height});

    gl_project(p3d, camera.viewMatrix(), camera.projectionMatrix(), viewport, p2d, depth);
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
    glViewport(0, 0, width, height);
    GLcanvas* v = static_cast<GLcanvas*>(glfwGetWindowUserPointer(window));
    v->height           = height;
    v->width            = width;
    v->camera.projection.setAspect(width, height);  // update the camera frustum
    v->camera.updateProjection();
    v->update_GL_projection();                      // update OpenGL's projection matrix
    v->draw();                                      // refresh canvas while resizing
    v->notify_camera_change();
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
                    v->handle_zoom(amount.z());
                    v->handle_pan(vec2d{amount.x(), amount.y()});
                }
            }
        }

        if (action == GLFW_PRESS && !modifiers)
        {
            static const std::string camera_clipboard_token{ "cinolib_gl_glcanvas_camera\n" };
            if (key == v->key_bindings.store_camera)
            {
                constexpr char sep{ ' ' };
                std::stringstream stream{};
                stream 
                    << camera_clipboard_token
                    << v->camera << sep
                    << v->camera_pivot;
                glfwSetClipboardString(window, stream.str().c_str());
            }
            if (key == v->key_bindings.restore_camera)
            {
                const char* const clipboard{ glfwGetClipboardString(window) };
                if (clipboard && std::strstr(clipboard, camera_clipboard_token.c_str()) == clipboard)
                {
                    std::stringstream{ clipboard + camera_clipboard_token.length() }
                        >> v->camera
                        >> v->camera_pivot;
                    v->width = static_cast<int>(std::round(v->height * v->camera.projection.aspectRatio));
                    glfwSetWindowSize(window, v->width, v->height);
                    v->camera.updateProjectionAndView();
                    v->update_GL_matrices();
                    v->draw();
                    v->notify_camera_change();
                }
            }
            if (key == v->key_bindings.reset_camera)
            {
                v->reset_camera();
            }
            if (key == v->key_bindings.look_at_center)
            {
                v->camera.view.lookAt(v->scene_center);
                // get rid of roll
                v->camera.view.rotateFps(world_up, world_forward, 0,0); // TODO (francescozoccheddu) this is ugly
                v->camera.updateView();
                v->update_GL_view();
                v->draw();
                v->notify_camera_change();
            }
            if (key == v->key_bindings.toggle_axes)
            {
                v->show_axis ^= true;
                v->draw();
            }
            if (key == v->key_bindings.toggle_ortho)
            {
                v->camera.projection.perspective ^= true;
                v->camera.projection.verticalFieldOfView = v->camera.projection.perspective ? 67 : v->scene_radius;
                v->camera.updateProjection();
                v->update_GL_projection();
                v->draw();
                v->notify_camera_change();
            }
            if (key == v->key_bindings.toggle_sidebar)
            {
                v->show_side_bar ^= true;
                v->draw();
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
            double dt = how_many_seconds(v->trackball.t_last_click, t);
            v->trackball.t_last_click = t;
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
    const vec2d delta{ cursor_pos - v->trackball.last_cursor_pos };
    v->trackball.last_cursor_pos = cursor_pos;

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
