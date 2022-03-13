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
#ifndef CINO_HEXMESH_H
#define CINO_HEXMESH_H

#include <sys/types.h>
#include <vector>
#include <cinolib/geometry/vec_mat.h>
#include <cinolib/meshes/quadmesh.h>
#include <cinolib/meshes/mesh_attributes.h>
#include <cinolib/meshes/abstract_polyhedralmesh.h>

namespace cinolib
{

template<class M = Mesh_std_attributes, // default template arguments
         class V = Vert_std_attributes,
         class E = Edge_std_attributes,
         class F = Polygon_std_attributes,
         class P = Polyhedron_std_attributes>
class Hexmesh : public AbstractPolyhedralMesh<M,V,E,F,P>
{
    public:

        explicit Hexmesh(){}

        explicit Hexmesh(const char * filename);

        explicit Hexmesh(const std::vector<double> & coords,
                         const std::vector<unsigned int>   & polys);

        explicit Hexmesh(const std::vector<vec3d> & verts,
                         const std::vector<unsigned int>  & polys);

        explicit Hexmesh(const std::vector<vec3d>             & verts,
                         const std::vector<std::vector<unsigned int>> & polys);

        ~Hexmesh(){}

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        MeshType mesh_type() const override { return HEXMESH; }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void load(const char * filename) override;
        void save(const char * filename) const override;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void update_f_normal(const unsigned int fid) override;
        void print_quality(const bool list_folded_elements = false);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        unsigned int verts_per_poly(const unsigned int) const override { return  8; }
        unsigned int verts_per_poly()           const          { return  8; }
        unsigned int edges_per_poly(const unsigned int) const override { return 12; }
        unsigned int edges_per_poly()           const          { return 12; }
        unsigned int faces_per_poly(const unsigned int) const override { return  6; }
        unsigned int faces_per_poly()           const          { return  6; }
        unsigned int verts_per_face(const unsigned int) const override { return  4; }
        unsigned int verts_per_face()           const          { return  4; }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        bool  vert_is_singular(const unsigned int vid) const;
        bool  vert_is_regular (const unsigned int vid) const;
        vec3d verts_average   (const std::vector<unsigned int> & vids) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        bool  edge_is_singular(const unsigned int eid) const;
        bool  edge_is_regular (const unsigned int eid) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        std::vector<unsigned int> face_sheet(const unsigned int fid) const; // stop at singular edges

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        unsigned int   poly_face_opposite_to(const unsigned int pid, const unsigned int fid) const;
        unsigned int   poly_vert_opposite_to(const unsigned int pid, const unsigned int fid, const unsigned int vid) const;
        void   poly_subdivide       (const std::vector<std::vector<std::vector<unsigned int>>> & split_scheme);
        double poly_volume          (const unsigned int pid) const override;
        bool   poly_fix_orientation ();
        void   poly_local_frame     (const unsigned int pid, vec3d & x, vec3d & y, vec3d & z);
        void   poly_local_frame     (const unsigned int pid, mat3d & xyz);
};

}

#ifndef  CINO_STATIC_LIB
#include "hexmesh.cpp"
#endif

#endif // CINO_HEXMESH_H
