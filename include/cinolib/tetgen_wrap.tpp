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
#include <cinolib/tetgen_wrap.h>
#include <cinolib/vector_serialization.h>

#ifdef CINOLIB_USES_TETGEN
#include <tetgen.h>
#endif

namespace cinolib
{

template<class M, class V, class E, class F, class P>
CINO_INLINE
void tetgen_wrap(const std::vector<vec3d>  & verts_in,
                 const std::vector<unsigned int>   & tris_in,
                 const std::vector<unsigned int>   & edges_in,
                 const std::string         & flags,
                       Tetmesh<M,V,E,F,P>  & m)
{
    std::vector<vec3d> verts;
    std::vector<unsigned int>  tets;
    tetgen_wrap(verts_in, tris_in, edges_in, flags, verts, tets);
    m = Tetmesh<M,V,E,F,P>(verts,tets);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
void tetgen_wrap(const std::vector<vec3d>             & verts_in,
                 const std::vector<std::vector<unsigned int>> & polys_in,
                 const std::vector<unsigned int>              & edges_in,
                 const std::string                    & flags,
                       Tetmesh<M,V,E,F,P>             & m)
{
    std::vector<vec3d> verts;
    std::vector<unsigned int>  tets;
    tetgen_wrap(verts_in, polys_in, edges_in, flags, verts, tets);
    m = Tetmesh<M,V,E,F,P>(verts,tets);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
void tetgen_wrap(const AbstractPolygonMesh<M,V,E,F> & m_srf,
                 const std::string                  & flags,
                       Tetmesh<M,V,E,F,P>           & m)
{
    tetgen_wrap(m_srf.vector_verts(), m_srf.vector_polys(), {}, flags, m);
}

}
