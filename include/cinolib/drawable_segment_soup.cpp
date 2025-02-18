/********************************************************************************
*  This file is part of CinoLib                                                 *
*  Copyright(C) 2016: Marco Livesu                                              *
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
#include <cinolib/drawable_segment_soup.h>
#include <cinolib/cino_inline.h>
#include <cinolib/gl/draw_sphere.h>
#include <cinolib/gl/draw_cylinder.h>
#include <cassert>

namespace cinolib
{

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
DrawableSegmentSoup::DrawableSegmentSoup() : bb{}
{
    color_first   = Color::RED();
    color_second  = Color::RED();
    thickness     = 1.0;
    use_gl_lines  = false;
    no_depth_test = false;
    show          = true;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
vec3d DrawableSegmentSoup::scene_center() const 
{ 
    return bb.center(); 
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
float DrawableSegmentSoup::scene_radius() const 
{ 
    return static_cast<float>(bb.diag()); 
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void DrawableSegmentSoup::draw(const float scene_size) const
{
    if (!show) return;

    if(no_depth_test) glDisable(GL_DEPTH_TEST);

    if(!use_gl_lines)
    {
        double cylind_rad = scene_size*0.002*thickness;

        for(unsigned int i=0; i<size()/2; ++i)
        {
            draw_sphere(at(2*i+0), cylind_rad, color_first);
            draw_sphere(at(2*i+1), cylind_rad, color_second);
            draw_cylinder(at(2*i+0), at(2*i+1), cylind_rad, cylind_rad, color_first);
        }
    }
    else
    {
        glLineWidth(thickness);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT,  GL_NICEST);
        glDisable(GL_LIGHTING);
        for(unsigned int i=0; i<size()/2; ++i)
        {
            vec3d a = at(2*i+0);
            vec3d b = at(2*i+1);
            glBegin(GL_LINES);
                glColor3fv(color_first.rgba);
                glVertex3d(a.x(), a.y(), a.z());
                glColor3fv(color_second.rgba);
                glVertex3d(b.x(), b.y(), b.z());
            glEnd();
        }
        glColor3f(0,0,0);
        glEnable(GL_LIGHTING);
        glDisable(GL_LINE_SMOOTH);
    }

    if(no_depth_test) glEnable(GL_DEPTH_TEST);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void DrawableSegmentSoup::push_seg(const vec3d v0, const vec3d v1)
{
    push_back(v0);
    push_back(v1);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void DrawableSegmentSoup::pop_seg()
{
    assert(size()>1);
    pop_back();
    pop_back();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void DrawableSegmentSoup::update_bbox()
{
    bb.reset();
    bb.push(*this);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void DrawableSegmentSoup::set_cheap_rendering(const bool b)
{
    use_gl_lines = b;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void DrawableSegmentSoup::set_always_in_front(const bool b)
{
    no_depth_test = b;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void DrawableSegmentSoup::set_color(const Color & c)
{
    color_first = color_second = c;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void DrawableSegmentSoup::set_color(const Color& first, const Color& second)
{
    color_first = first;
    color_second = second;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void DrawableSegmentSoup::set_thickness(float t)
{
    thickness = t;
}

}
