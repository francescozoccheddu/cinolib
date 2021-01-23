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
#ifndef CINO_MARECHAL_HEX_SCHEME_H
#define CINO_MARECHAL_HEX_SCHEME_H

#include <cinolib/meshes/abstract_polyhedralmesh.h>
#include <cinolib/meshes/polyhedralmesh.h>

namespace cinolib
{

/* Implementation of the convertion schemes to transform an adaptively refined grid
 * with hanging nodes into a conformin hexahedral mesh. This code is based on
 *
 *     Advances in Octree-Based All-Hexahedral Mesh Generation: Handling Sharp Features
 *     Loic Marechal
 *     Proceedings of the 18th International Meshing Roundtable, 2009
 *
 * The algorithm works in the primal mesh, modifying the connectivity to adjust the
 * valence of the vertices, and achieving valence 6 everywhere. 
 *
 * The code takes as input:
 *
 *  - a generic voluemtric mesh (both a non conforming hexmesh and a polyhedral mesh are good)
 *  - a bottom 5x5 grid of mesh vertices
 *  - a top    3x3 grid of mesh vertices
 *
 * Input grids must be aligned, meaning that bot[0][0] should have top[0][0] above it in the mesh,
 * bot[4][0] should have top[2][0] above it, bot[4][4] should have top[2][2] above it, and so on...
 *
 * The output is a set of polyhedral cells that stay in between the two grids, and realize the
 * necessary grading to connect them in a conforming way. Dualizing these elements only hexahedra
 * are generated (see cinolib::dual_mesh for dualization).
*/

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
void marechal(const AbstractPolyhedralMesh<M,V,E,F,P> & m,
                    Polyhedralmesh<>                  & m_out,
              uint                                      bot[5][5],
              uint                                      top[3][3]);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
void marechal(const AbstractPolyhedralMesh<M,V,E,F,P> & m,
              const uint                                bot[5][5],
              const uint                                top[3][3],
                    std::vector<vec3d>                & verts,
                    std::vector<std::vector<uint>>    & faces,
                    std::vector<std::vector<uint>>    & polys);

}


#ifndef  CINO_STATIC_LIB
#include "marechal_hex_scheme.cpp"
#endif

#endif // CINO_MARECHAL_HEX_SCHEME_H
