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
#include <cinolib/triangle_wrap.h>
#include <cinolib/geometry/vec_mat.h>
#include <cinolib/vector_serialization.h>

#ifdef CINOLIB_USES_TRIANGLE

// define symbols used by triangle (with a backup of previous symbols, if any)
#ifdef ANSI_DECLARATORS
#  define PREVIOUSLY_DEFINED_ANSI_DECLARATORS ANSI_DECLARATORS
#  undef ANSI_DECLARATORS
#endif
#ifdef REAL
#  define PREVIOUSLY_DEFINED_REAL REAL
#  undef REAL
#endif
#ifdef VOID
#  define PREVIOUSLY_DEFINED_VOID VOID
#  undef VOID
#endif
#define ANSI_DECLARATORS
#define REAL double
#define VOID int

#include <triangle.h>

// restore previously existing symbols
#undef ANSI_DECLARATORS
#ifdef PREVIOUSLY_DEFINED_ANSI_DECLARATORS
#  define ANSI_DECLARATORS PREVIOUSLY_DEFINED_ANSI_DECLARATORS
#endif
#undef REAL
#ifdef PREVIOUSLY_DEFINED_REAL
#  define REAL PREVIOUSLY_DEFINED_REAL
#endif
#undef VOID
#ifdef PREVIOUSLY_DEFINED_VOID
#  define VOID PREVIOUSLY_DEFINED_VOID
#endif
#endif

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

namespace cinolib
{

template<class M, class V, class E, class P>
CINO_INLINE
void triangle_wrap(const std::vector<double> & coords_in, // serialized input xy coordinates
                   const std::vector<unsigned int>   & segs_in,   // serialized segments
                   const std::vector<double> & holes_in,  // serialized xy holes
                   const double                z_coord,   // lift triangulation to z_coord
                   const std::string         & flags,     // https://www.cs.cmu.edu/~quake/triangle.switch.html
                         Trimesh<M,V,E,P>    & m)

{
    std::vector<double> v;
    std::vector<unsigned int>   t;
    triangle_wrap(coords_in, segs_in, holes_in, flags, v, t);
    m = Trimesh<M,V,E,P>(vec3d_from_serialized_xy(v,z_coord), t);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void triangle_wrap(const std::vector<vec2d> & verts_in,
                   const std::vector<unsigned int>  & segs_in,   // serialized segments
                   const std::vector<vec2d> & holes_in,
                   const double               z_coord,   // lift triangulation to z_coord
                   const std::string        & flags,     // https://www.cs.cmu.edu/~quake/triangle.switch.html
                         Trimesh<M,V,E,P>   & m)
{
    std::vector<double> v;
    std::vector<unsigned int>   t;
    triangle_wrap(serialized_xy_from_vec2d(verts_in), segs_in,
                  serialized_xy_from_vec2d(holes_in), flags, v, t);
    m = Trimesh<M,V,E,P>(vec3d_from_serialized_xy(v,z_coord), t);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void triangle_wrap(const std::vector<vec3d> & verts_in,
                   const std::vector<unsigned int>  & segs_in,   // serialized segments
                   const std::vector<vec3d> & holes_in,
                   const double               z_coord,   // lift triangulation to z_coord
                   const std::string        & flags,     // https://www.cs.cmu.edu/~quake/triangle.switch.html
                         Trimesh<M,V,E,P>   & m)
{
    std::vector<double> v;
    std::vector<unsigned int>   t;
    triangle_wrap(serialized_xy_from_vec3d(verts_in), segs_in,
                  serialized_xy_from_vec3d(holes_in), flags, v, t);
    m = Trimesh<M,V,E,P>(vec3d_from_serialized_xy(v,z_coord), t);
}

}

#ifdef CINOLIB_USES_TRIANGLE
    #undef ANSI_DECLARATORS
    #undef REAL
    #undef VOID
#endif
