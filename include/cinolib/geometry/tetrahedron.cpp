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
#include <cinolib/geometry/tetrahedron.h>
#include <cinolib/geometry/tetrahedron_utils.h>

namespace cinolib
{

CINO_INLINE
ItemType Tetrahedron::item_type() const
{
    return TETRAHEDRON;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
Bbox Tetrahedron::aabb() const
{
    return Bbox({v0, v1, v2, v3});
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
vec3d Tetrahedron::point_closest_to(const vec3d & p) const
{
    return tetrahedron_closest_point(p,v0,v1,v2,v3);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
bool Tetrahedron::intersects_ray(const vec3d & p, const vec3d & dir, double & t, vec3d & pos) const
{
    bool   backside;
    bool   coplanar;
    vec3d  bary;
    double face_t[4];
    if(!Moller_Trumbore_intersection(p, dir, v0, v2, v1, backside, coplanar, face_t[0], bary)) face_t[0] = inf_double;
    if(!Moller_Trumbore_intersection(p, dir, v0, v1, v3, backside, coplanar, face_t[1], bary)) face_t[1] = inf_double;
    if(!Moller_Trumbore_intersection(p, dir, v0, v3, v2, backside, coplanar, face_t[2], bary)) face_t[2] = inf_double;
    if(!Moller_Trumbore_intersection(p, dir, v1, v2, v3, backside, coplanar, face_t[3], bary)) face_t[3] = inf_double;
    double min_t = *std::min_element(face_t, face_t+4);
    if(min_t!=inf_double)
    {
        t   = min_t;
        pos = p + t*dir;
        return true;
    }
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void Tetrahedron::barycentric_coordinates(const vec3d &p, double bc[]) const
{
    std::vector<double> wgts;
    tet_barycentric_coords(v0, v1, v2, v3, p, wgts, 0);
    bc[0] = wgts[0];
    bc[1] = wgts[1];
    bc[2] = wgts[2];
    bc[3] = wgts[3];
}

}

