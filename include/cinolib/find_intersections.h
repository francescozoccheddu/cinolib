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
#ifndef CINO_FIND_INTERSECTIONS_H
#define CINO_FIND_INTERSECTIONS_H

#include <cinolib/geometry/vec_mat.h>
#include <cinolib/meshes/trimesh.h>
#include <cinolib/ipair.h>
#include <set>

namespace cinolib
{

/* This method puts all the input polygons into an octree, then
 * performs pairwise intersection tests within each leaf, retunrning
 * a set of pairs of intersecting triangles.
 *
 * The output must be a set and not a vector because triangles will
 * appear in all the leaves that have non empty overlap with it. Therefore,
 * the same intersection can be detected multiple times.
 *
 * IMPORTANT: intersections tests are based on the orient predicates contained
 * in cinolib/predicates.h. These predicates are exact if the symbol
 * CINOLIB_USES_EXACT_PREDICATES, and are approximated otherwise.
*/

template<class M, class V, class E, class P>
CINO_INLINE
void find_intersections(const Trimesh<M,V,E,P> & m,
                        std::set<ipair>        & intersections);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void find_intersections(const std::vector<vec3d> & verts,
                        const std::vector<unsigned int>  & tris,
                              std::set<ipair>    & intersections);

}

#include "find_intersections.tpp"
#ifndef  CINO_STATIC_LIB
#include "find_intersections.cpp"
#endif

#endif // CINO_FIND_INTERSECTIONS_H
