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
#ifndef CINO_QUADMESH_H
#define CINO_QUADMESH_H

#include <vector>
#include <sys/types.h>
#include <cinolib/meshes/mesh_attributes.h>
#include <cinolib/meshes/abstract_polygonmesh.h>

namespace cinolib
{

template<class M = Mesh_std_attributes, // default template arguments
         class V = Vert_std_attributes,
         class E = Edge_std_attributes,
         class P = Polygon_std_attributes>
class Quadmesh : public AbstractPolygonMesh<M,V,E,P>
{
    public:

        explicit Quadmesh(){}

        explicit Quadmesh(const char * filename);

        explicit Quadmesh(const std::vector<vec3d> & verts,
                          const std::vector<unsigned int>  & polys);

        explicit Quadmesh(const std::vector<double> & coords,
                          const std::vector<unsigned int>   & polys);

        explicit Quadmesh(const std::vector<vec3d>             & verts,
                          const std::vector<std::vector<unsigned int>> & polys);

        explicit Quadmesh(const std::vector<double>            & coords,
                          const std::vector<std::vector<unsigned int>> & polys);

        ~Quadmesh(){}

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        MeshType mesh_type() const override { return QUADMESH; }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        unsigned int verts_per_poly(const unsigned int) const override { return 4; }
        unsigned int verts_per_poly()           const          { return 4; }
        unsigned int edges_per_poly(const unsigned int) const override { return 4; }
        unsigned int edges_per_poly()           const          { return 4; }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        bool              vert_is_singular     (const unsigned int vid) const;
        bool              vert_is_regular      (const unsigned int vid) const;
        int               vert_next_along_chain(const unsigned int curr, const unsigned int prev) const;
        std::vector<unsigned int> vert_chain           (const unsigned int start, const unsigned int next) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        int                            edge_next_along_chain(const unsigned int eid, const unsigned int vid) const;
        unsigned int                           edge_opposite_to(const unsigned int pid, const unsigned int eid) const;
        std::vector<unsigned int>              edges_opposite_to(const unsigned int eid) const;
        std::vector<unsigned int>              edge_chain(const unsigned int eid, const unsigned int vid) const; // chain of ADJACENT edges
        std::vector<unsigned int>              edge_parallel_chain(const unsigned int eid) const;        // chain of PARALLEL edges
        std::vector<std::vector<unsigned int>> edge_parallel_chains() const;                     // chain of PARALLEL edges

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        unsigned int poly_vert_opposite_to(const unsigned int pid, const unsigned int vid) const;
};

}

#ifndef  CINO_STATIC_LIB
#include "quadmesh.cpp"
#endif

#endif // CINO_QUADMESH_H
