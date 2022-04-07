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
#include <cinolib/tetrahedralization.h>
#include <cinolib/ipair.h>
#include <cinolib/standard_elements_tables.h>
#include <set>
#include <cassert>
#include <algorithm>
#include <cinolib/stl_container_utilities.h>

namespace cinolib
{

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

// Transforms a hexahedral mesh into a conforming tetrahedral
// mesh, splitting all hexahedra into five or six tetrahedra
template<class M, class V, class E, class F, class P>
CINO_INLINE
void hex_to_tets(const Hexmesh<M,V,E,F,P> & hm,
                       Tetmesh<M,V,E,F,P> & tm)
{
    for(unsigned int vid=0; vid<hm.num_verts(); ++vid)
    {
        tm.vert_add(hm.vert(vid));
    }

    for(unsigned int pid=0; pid<hm.num_polys(); ++pid)
    {
        std::vector<unsigned int> tets;
        hex_to_tets(hm.poly_verts_id(pid),tets);

        auto t = polys_from_serialized_vids(tets,4);
        for(auto tet : t)
        {
            unsigned int id = tm.poly_add(tet);
            tm.poly_data(id).label = pid;
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
void hex_to_corner_tets(const Hexmesh<M,V,E,F,P> & hm,
                              Tetmesh<M,V,E,F,P> & tm)
{
    for(unsigned int vid=0; vid<hm.num_verts(); ++vid)
    {
        tm.vert_add(hm.vert(vid));
    }

    for(unsigned int pid=0; pid<hm.num_polys(); ++pid)
    {
        std::vector<unsigned int> tets;
        hex_to_corner_tets(hm.poly_verts_id(pid),tets);

        auto t = polys_from_serialized_vids(tets,4);
        for(auto tet : t) tm.poly_add(tet);
    }
}

}
