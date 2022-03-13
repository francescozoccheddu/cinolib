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
#ifndef CINO_TETMESH_H
#define CINO_TETMESH_H

#include <sys/types.h>
#include <vector>
#include <cinolib/meshes/abstract_polyhedralmesh.h>
#include <cinolib/meshes/mesh_attributes.h>
#include <cinolib/meshes/trimesh.h>
#include <cinolib/geometry/vec_mat.h>

namespace cinolib
{

template<class M = Mesh_std_attributes, // default template arguments
         class V = Vert_std_attributes,
         class E = Edge_std_attributes,
         class F = Polygon_std_attributes,
         class P = Polyhedron_std_attributes>
class Tetmesh : public AbstractPolyhedralMesh<M,V,E,F,P>
{
    public:

        explicit Tetmesh(){}

        explicit Tetmesh(const char * filename);

        explicit Tetmesh(const std::vector<double> & coords,
                         const std::vector<unsigned int>   & polys);

        explicit Tetmesh(const std::vector<vec3d> & verts,
                         const std::vector<unsigned int>  & polys);

        explicit Tetmesh(const std::vector<vec3d>             & verts,
                         const std::vector<std::vector<unsigned int>> & polys);

        ~Tetmesh() {}

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        MeshType mesh_type() const override { return TETMESH; }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void load(const char * filename) override;
        void save(const char * filename) const override;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void update_f_normal(const unsigned int fid)  override;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        unsigned int verts_per_poly(const unsigned int) const override { return  4; }
        unsigned int verts_per_poly()           const          { return  4; }
        unsigned int edges_per_poly(const unsigned int) const override { return  6; }
        unsigned int edges_per_poly()           const          { return  6; }
        unsigned int faces_per_poly(const unsigned int) const override { return  4; }
        unsigned int faces_per_poly()           const          { return  4; }
        unsigned int verts_per_face(const unsigned int) const override { return  3; }
        unsigned int verts_per_face()           const          { return  3; }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void vert_weights          (const unsigned int vid, const int type, std::vector<std::pair<unsigned int,double>> & wgts) const override;
        void vert_weights_cotangent(const unsigned int vid, std::vector<std::pair<unsigned int,double>> & wgts) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        bool edge_flip                        (const unsigned int eid); // 3-to-2 flip
        unsigned int edge_split                       (const unsigned int eid, const vec3d & p);
        unsigned int edge_split                       (const unsigned int eid, const double lambda = 0.5); // use linear interpolation: e0*(1-lambda) + e1*lambda
        int  edge_collapse                    (const unsigned int eid, const double lambda = 0.5, const double topologic_check = true, const double geometric_check = true);
        int  edge_collapse                    (const unsigned int eid, const vec3d & p, const double topologic_check = true, const double geometric_check = true);
        bool edge_is_collapsible              (const unsigned int eid, const vec3d & p) const;
        bool edge_is_collapsible              (const unsigned int eid, const double lambda) const;
        bool edge_is_geometrically_collapsible(const unsigned int eid, const vec3d & p) const;
        bool edge_is_topologically_collapsible(const unsigned int eid) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        bool   face_flip            (const unsigned int fid, const bool geometric_check = true); // 2-to-3 flip
        double face_area            (const unsigned int fid) const;
        unsigned int   face_edge_opposite_to(const unsigned int fid, const unsigned int vid) const;
        unsigned int   face_vert_opposite_to(const unsigned int fid, const unsigned int eid) const;
        unsigned int   face_split           (const unsigned int fid, const vec3d & p);
        unsigned int   face_split           (const unsigned int fid, const std::vector<double> & bc = { 1./3., 1./3., 1./3. });

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        int               poly_id              (const unsigned int fid, const unsigned int vid) const;
        int               poly_id_from_vids    (const std::vector<unsigned int> & vids) const;
        double            poly_dihedral_angle  (const unsigned int pid, const unsigned int fid0, const unsigned int fid1) const;
        unsigned int              poly_vert_opposite_to(const unsigned int pid, const unsigned int fid) const;
        unsigned int              poly_edge_opposite_to(const unsigned int pid, const unsigned int eid) const;
        unsigned int              poly_edge_opposite_to(const unsigned int pid, const unsigned int vid0, const unsigned int vid1) const;
        unsigned int              poly_face_opposite_to(const unsigned int pid, const unsigned int vid) const;
        std::vector<unsigned int> poly_faces_opposite_to(const unsigned int pid, const unsigned int eid) const;
        int               poly_shared_vert      (const unsigned int pid, const std::vector<unsigned int> & incident_edges) const; // TODO: move to global ids!!!!!!
        void              poly_bary_coords      (const unsigned int pid, const vec3d & p, double bc[]) const;
        double            poly_volume           (const unsigned int pid) const override;
        unsigned int              poly_split            (const unsigned int pid, const vec3d & p);
        unsigned int              poly_split            (const unsigned int pid, const std::vector<double> & bc = { 0.25, 0.25, 0.25, 0.25 });
        unsigned int              poly_split            (const unsigned int pid, const unsigned int vid);
        void              polys_split           (const std::vector<unsigned int> & pids);

        using AbstractPolyhedralMesh<M,V,E,F,P>::poly_id;  // avoid hiding poly_id(flist)

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
};

}

#ifndef  CINO_STATIC_LIB
#include "tetmesh.cpp"
#endif

#endif // CINO_TETMESH_H
