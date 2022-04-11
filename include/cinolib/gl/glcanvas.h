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
#ifndef CINO_GLCANVAS_H
#define CINO_GLCANVAS_H

#include <cinolib/gl/gl_glfw.h>
#include <cinolib/drawable_object.h>
#include <cinolib/gl/side_bar_item.h>
#include <cinolib/gl/FreeCamera.hpp>
#include <cinolib/min_max_inf.h>
#include <cinolib/color.h>
#include <vector>
#include <functional>
#include <chrono>

namespace cinolib
{

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

struct Marker
{
    vec2d       pos_2d      = vec2d(inf_double); // first choice to position the marker
    vec3d       pos_3d      = vec3d(inf_double); // used to position a 3D marker IFF pos_2d is INF
    std::string text        = "";                // text to render. Set to the empty string to not render the text
    Color       color       = Color::BLUE();     // color, for both the text and the disk
    unsigned int        disk_radius =  1;                // disk radius (in pixels). Set to zero to not render the disk
    unsigned int        font_size   = 12;                // font size;
};

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

struct Trackball
{
    vec2d last_cursor_pos = vec2d(inf_double);
    std::chrono::high_resolution_clock::time_point t_last_click;
};

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

class GLcanvas
{

    private:

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void notify_camera_change() const;
        double get_camera_speed_modifier() const;
        void handle_zoom(double amount);
        void handle_rotation(const vec2d& amount);
        void handle_translation(const vec3d& amount);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        // internal event handlers
        static void window_size_event(GLFWwindow* w, int    width, int    height);
        static void key_event(GLFWwindow* w, int    key, int    unused, int action, int modif);
        static void mouse_button_event(GLFWwindow* w, int    butt, int    action, int modif);
        static void cursor_event(GLFWwindow* w, double x_pos, double y_pos);
        static void scroll_event(GLFWwindow* w, double x_off, double y_off);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    public:

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        struct KeyBindings final
        {
            static const int none;

            int toggle_ortho{ GLFW_KEY_O };
            int camera_faster{ GLFW_KEY_LEFT_SHIFT };
            int camera_slower{ GLFW_KEY_LEFT_CONTROL };
            int camera_inplace_zoom{ GLFW_KEY_LEFT_ALT };
            int camera_inplace_rotation{ GLFW_KEY_LEFT_ALT };
            int reset_camera{ GLFW_KEY_R };
            int look_at_center{ GLFW_KEY_L };
            int toggle_axes{ GLFW_KEY_A };
            int toggle_sidebar{ GLFW_KEY_TAB };
            int store_camera{ GLFW_KEY_C };
            int restore_camera{ GLFW_KEY_V };
            bool pan_with_arrow_keys{ true };
            bool pan_and_zoom_with_numpad_keys{ true };

            void print() const;

        } key_bindings;

        struct MouseBindings final
        {
            static const int none;

            int camera_pan{ GLFW_MOUSE_BUTTON_RIGHT };
            int camera_zoom{ GLFW_MOUSE_BUTTON_MIDDLE };
            int camera_rotate{ GLFW_MOUSE_BUTTON_LEFT };
            bool zoom_with_wheel{ true };

            void print() const;

        } mouse_bindings;

        struct CameraSettings final
        {
            double zoom_scroll_speed{ 1 };
            double zoom_drag_speed{ 1 };
            double translate_key_speed{ 1 };
            double pan_drag_speed{ 1 };
            double rotate_drag_speed{ 1 };
            double faster_factor{ 2 };
            double slower_factor{ 0.5 };
            double far_scene_radius_factor{ 10 };
            double near_scene_radius_factor{ 1.0 / 100 };
            double max_persp_fov{ 120 };
            double min_persp_fov{ 20 };
            double max_ortho_fov_scene_radius_factor{ 2 };
            double min_ortho_fov_scene_radius_factor{ 1.0 / 100 };
            double max_up_angle_diff{ 10 };
            double camera_distance_scene_radius_factor{ 2 };
        } camera_settings;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        vec3d scene_center{};
        float scene_radius{};
        int width, height;
        double DPI_factor;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        GLFWwindow                        *window;
        std::vector<const DrawableObject*> drawlist;
        std::vector<Marker>                markers;
        std::vector<SideBarItem*>          side_bar_items;
        bool                               show_side_bar      = false;
        float                              side_bar_width     = 0.2f;
        float                              side_bar_alpha     = 1.f;
        int                                font_size          = 13;
        bool                               show_axis          = false;
        bool                               depth_cull_markers = true; // skip occluded 3D markers, testing their depth with the Z-buffer

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        // ImGui and GLFW do not yet handle multi windows properly.
        // This variable ensures that only one window will create
        // and handle the ImGui context. As a result, only that
        // window will benefit from the functionalities implemented
        // with ImGui, such as the side bar with visual controls and
        // the visual markers
        bool owns_ImGui = false;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        FreeCamera<double> camera{};
        Trackball trackball{};

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

         GLcanvas(const int width = 700, const int height = 700);
        ~GLcanvas();

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        // Main application loop. This is the typical return statement in
        // the main of your application. It consists of an infinite loop
        // that interleaves rendering and event handling.
        // In case the application has multiple windows, secondary windows
        // can be passed as additional parameters, such that rendering and
        // event polling are all controlled from a single place. Closing
        // *any* of the windows will cause a termination of the program.
        int launch(std::initializer_list<GLcanvas*> additional_windows = {});

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void draw();                 // single render pass
        void draw_side_bar();        // render side bar with visual controls (if any)
        void draw_markers()   const; // render text labels with ImGui (3d markers are depth tested if culling is enabled)
        void draw_axis()      const; // render the global frame XYZ

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void refit_scene(bool update_gl = true);
        void reset_camera(bool update_gl = true);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void push(const DrawableObject * obj, const bool reset_camera = true);
        void push(const Marker         & m);
        void push(SideBarItem          * item);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void push_marker(const vec2d       & p,
                         const std::string & text        = "",
                         const Color         color       = Color::BLUE(),
                         const unsigned int          disk_radius =  5,
                         const unsigned int          font_size   = 10);

        void push_marker(const vec3d       & p,
                         const std::string & text        = "",
                         const Color         color       = Color::BLUE(),
                         const unsigned int          disk_radius =  5,
                         const unsigned int          font_size   = 10);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        bool pop(const DrawableObject *obj);
        void pop_all_markers();

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void update_GL_matrices()   const;
        void update_GL_view()  const;
        void update_GL_projection() const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void update_DPI_factor();

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        vec2d cursor_pos() const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        // TODO:
        //   https://stackoverflow.com/questions/18006299/quickly-read-values-from-the-depth-buffer
        //   (GLFW uses 24 bits by default)
        //
        void    whole_Z_buffer(      GLfloat * buf) const;
        GLfloat query_Z_buffer(const vec2d   & p2d) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void project  (const vec3d & p3d, vec2d & p2d, GLdouble & depth)      const;
        bool unproject(const vec2d & p2d, vec3d & p3d)                        const;
        bool unproject(const vec2d & p2d, const GLdouble & depth, vec3d & p3d) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        // these callbacks can be used to execute external code when mouse and keyboard
        // events occurr (e.g. for picking, drawing). If defined, they will be called
        // **before** the internal event handlers
        std::function<bool(int    key,      int    modifiers)> callback_key_pressed        = nullptr;
        std::function<bool(int    modifiers                 )> callback_mouse_left_click   = nullptr;
        std::function<bool(int    modifiers                 )> callback_mouse_left_click2  = nullptr; // double click
        std::function<bool(int    modifiers                 )> callback_mouse_right_click  = nullptr;
        std::function<bool(int    modifiers                 )> callback_mouse_right_click2 = nullptr; // double click
        std::function<bool(double x_pos,    double y_pos    )> callback_mouse_moved        = nullptr;
        std::function<bool(double x_offset, double y_offset )> callback_mouse_scroll       = nullptr;
        std::function<void(void                             )> callback_app_controls       = nullptr; // useful to insert app-dependent visual controls (with ImGui)
        std::function<void(void                             )> callback_camera_changed     = nullptr; 

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
};

}

#ifndef  CINO_STATIC_LIB
#include "glcanvas.cpp"
#endif

#endif // CINO_GLCANVAS_H
