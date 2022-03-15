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
#ifndef CINO_TRIMESH_H
#define CINO_TRIMESH_H

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
class Trimesh : public AbstractPolygonMesh<M,V,E,P>
{
    public:

        explicit Trimesh(){}

        explicit Trimesh(const char * filename);

        explicit Trimesh(const std::vector<vec3d>             & coords,
                         const std::vector<std::vector<unsigned int>> & polys);

        explicit Trimesh(const std::vector<double>            & coords,
                         const std::vector<std::vector<unsigned int>> & polys);

        explicit Trimesh(const std::vector<vec3d> & coords,
                         const std::vector<unsigned int>  & polys);

        explicit Trimesh(const std::vector<double> & coords,
                         const std::vector<unsigned int>   & polys);

        ~Trimesh(){}

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        MeshType mesh_type() const override { return TRIMESH; }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void update_p_normal(const unsigned int pid) override;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        unsigned int verts_per_poly(const unsigned int) const override { return 3; }
        unsigned int verts_per_poly()           const          { return 3; }
        unsigned int edges_per_poly(const unsigned int) const override { return 3; }
        unsigned int edges_per_poly()           const          { return 3; }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        unsigned int              vert_split            (const unsigned int eid0, const unsigned int eid1);
        void              vert_weights          (const unsigned int vid, const int type, std::vector<std::pair<unsigned int,double>> & wgts) const override;
        void              vert_weights_cotangent(const unsigned int vid, std::vector<std::pair<unsigned int,double>> & wgts) const;
        unsigned int              vert_opposite_to      (const unsigned int pid, const unsigned int vid0, const unsigned int vid1) const;
        std::vector<unsigned int> vert_link_edges       (const unsigned int vid) const;
        std::vector<unsigned int> verts_opposite_to     (const unsigned int eid) const;

        using  AbstractMesh<M,V,E,P>::vert_opposite_to; // avoid hiding vert_opposite_to(eid,vid)

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        unsigned int              edge_opposite_to                 (const unsigned int pid, const unsigned int vid) const;
        int               edge_collapse                    (const unsigned int eid, const double lambda = 0.5, const bool topologic_check = true, const bool geometric_check = true);
        bool              edge_is_collapsible              (const unsigned int eid, const double lambda) const;
        bool              edge_is_geometrically_collapsible(const unsigned int eid, const double lambda) const;
        bool              edge_is_topologically_collapsible(const unsigned int eid) const;
        unsigned int              edge_split                       (const unsigned int eid, const double lambda = 0.5);
        unsigned int              edge_split                       (const unsigned int eid, const vec3d & p);
        bool              edge_is_flippable                (const unsigned int eid);
        double            edge_cotangent_weight            (const unsigned int eid) const;
        int               edge_flip                        (const unsigned int eid, const bool geometric_check = true);
        std::vector<unsigned int> edge_verts_link                  (const unsigned int eid) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        int                 poly_id            (const unsigned int eid0, const unsigned int eid1) const;
        double              poly_area          (const unsigned int pid) const;
        bool                poly_is_cap        (const unsigned int pid, const double angle_thresh_deg = 177.0) const;
        bool                poly_is_needle     (const unsigned int pid, const double angle_thresh_deg = 3.0) const;
        void                poly_bary_coords   (const unsigned int pid, const vec3d & p, double bc[]) const;
        unsigned int                poly_add           (const unsigned int vid0, const unsigned int vid1, const unsigned int vid2);
        unsigned int                poly_edge_id       (const unsigned int pid, const unsigned int offset) const;
        unsigned int                poly_split         (const unsigned int pid); // uses centroid as default split point
        unsigned int                poly_split         (const unsigned int pid, const vec3d & p);

        using AbstractMesh<M,V,E,P>::poly_edge_id;    // avoid hiding poly_ege_id(pid,vid0,vid1)
        using AbstractPolygonMesh<M,V,E,P>::poly_add; // avoid hiding poly_add(p)
        using AbstractPolygonMesh<M,V,E,P>::poly_id;  // avoid hiding poly_id(vlist);
};

}

#include "trimesh.tpp"

#endif // CINO_TRIMESH_H
