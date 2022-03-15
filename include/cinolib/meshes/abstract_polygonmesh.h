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
#ifndef CINO_ABSTRACT_POLYGON_MESH_H
#define CINO_ABSTRACT_POLYGON_MESH_H

#include <cinolib/meshes/abstract_mesh.h>
#include <cinolib/meshes/mesh_attributes.h>
#include <cinolib/ipair.h>
#include <cinolib/symbols.h>

namespace cinolib
{

template<class M = Mesh_std_attributes,
         class V = Vert_std_attributes,
         class E = Edge_std_attributes,
         class P = Polygon_std_attributes>
class AbstractPolygonMesh : public AbstractMesh<M,V,E,P>
{
    protected:

        std::vector<std::vector<unsigned int>> poly_triangles; // triangles covering each quad. Useful for
                                                       // robust normal estimation and rendering

    public:

        explicit AbstractPolygonMesh() : AbstractMesh<M,V,E,P>() {}
        ~AbstractPolygonMesh() {}

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void load(const char * filename) override;
        void save(const char * filename) const override;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void clear() override;
        void init(const std::vector<vec3d>             & verts,
                  const std::vector<std::vector<unsigned int>> & polys);
        void init(      std::vector<vec3d>             & pos,       // vertex xyz positions
                        std::vector<vec3d>             & tex,       // vertex uv(w) texture coordinates
                        std::vector<vec3d>             & nor,       // vertex normals
                        std::vector<std::vector<unsigned int>> & poly_pos,  // polygons with references to pos
                  const std::vector<std::vector<unsigned int>> & poly_tex,  // polygons with references to tex
                  const std::vector<std::vector<unsigned int>> & poly_nor,  // polygons with references to nor
                  const std::vector<Color>             & poly_col,  // per polygon colors
                  const std::vector<int>               & poly_lab); // per polygon labels

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

                void update_normals() override;
                void update_p_tessellation(const unsigned int pid);
        virtual void update_p_normal(const unsigned int pid);
                void update_v_normal(const unsigned int vid);
                void update_p_tessellations();
                void update_p_normals();
                void update_v_normals();

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        int Euler_characteristic() const override;
        int genus() const override;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        unsigned int verts_per_poly(const unsigned int pid) const override { return this->polys.at(pid).size(); }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        const std::vector<unsigned int> & adj_p2v(const unsigned int pid) const override { return this->polys.at(pid); }
              std::vector<unsigned int> & adj_p2v(const unsigned int pid)       override { return this->polys.at(pid); }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void operator+=(const AbstractPolygonMesh<M,V,E,P> & m);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        std::vector<unsigned int>  get_boundary_vertices()         const;
        std::vector<unsigned int>  get_ordered_boundary_vertices() const;
        std::vector<ipair> get_boundary_edges()            const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        double mesh_volume() const;
        double mesh_area()   const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void normalize_area();

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        bool              vert_is_saddle          (const unsigned int vid, const int tex_coord = U_param) const;
        bool              vert_is_critical_p      (const unsigned int vid, const int tex_coord = U_param) const;
        double            vert_area               (const unsigned int vid) const;
        double            vert_mass               (const unsigned int vid) const override;
        bool              vert_is_boundary        (const unsigned int vid) const;
        bool              vert_is_manifold        (const unsigned int vid) const;
        void              vert_switch_id          (const unsigned int vid0, const unsigned int vid1);
        void              vert_remove             (const unsigned int vid);
        void              vert_remove_unreferenced(const unsigned int vid);
        unsigned int              vert_add                (const vec3d & pos);
        bool              vert_merge              (const unsigned int vid0, const unsigned int vid1);
        void              vert_cluster_one_ring   (const unsigned int vid, std::vector<std::vector<unsigned int>> & clusters, const bool marked_edges_are_borders);
        bool              vert_is_visible         (const unsigned int vid) const;
        std::vector<unsigned int> vert_adj_visible_polys  (const unsigned int vid, const vec3d dir, const double ang_thresh = 60.0);
        std::vector<unsigned int> vert_boundary_edges     (const unsigned int vid) const;
        std::vector<unsigned int> vert_verts_link         (const unsigned int vid) const; // see https://en.wikipedia.org/wiki/Simplicial_complex#Closure,_star,_and_link for adefinition of link and star
        std::vector<unsigned int> vert_edges_link         (const unsigned int vid) const;
        std::vector<unsigned int> vert_ordered_verts_link (const unsigned int vid) const;
        std::vector<unsigned int> vert_ordered_polys_star (const unsigned int vid) const;
        std::vector<unsigned int> vert_ordered_edges_star (const unsigned int vid) const;
        std::vector<unsigned int> vert_ordered_edges_link (const unsigned int vid) const;
        void              vert_ordered_one_ring   (const unsigned int          vid,
                                                   std::vector<unsigned int> & v_link,        // sorted list of adjacent vertices
                                                   std::vector<unsigned int> & f_star,        // sorted list of adjacent triangles
                                                   std::vector<unsigned int> & e_star,        // sorted list of edges incident to vid
                                                   std::vector<unsigned int> & e_link) const; // sorted list of edges opposite to vid

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        bool   edge_is_manifold               (const unsigned int eid) const;
        bool   edge_is_boundary               (const unsigned int eid) const;
        bool   edge_is_incident_to_boundary   (const unsigned int eid) const;
        double edge_dihedral_angle              (const unsigned int eid) const override;
        bool   edges_share_poly               (const unsigned int eid1, const unsigned int eid2) const;
        unsigned int   edge_shared                    (const unsigned int pid0, const unsigned int pid1) const;
        void   edge_switch_id                 (const unsigned int eid0, const unsigned int eid1);
        unsigned int   edge_add                       (const unsigned int vid0, const unsigned int vid1);
        void   edge_remove                    (const unsigned int eid);
        void   edge_remove_unreferenced       (const unsigned int eid);
        void   edge_mark_labeling_boundaries  ();
        void   edge_mark_color_discontinuities();
        void   edge_mark_boundaries           ();
        bool   edge_is_CCW                    (const unsigned int eid, const unsigned int pid) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

              unsigned int                 poly_vert_offset        (const unsigned int pid, const unsigned int vid) const;
              double               poly_angle_at_vert      (const unsigned int pid, const unsigned int vid, const int unit = RAD) const;
              double               poly_area               (const unsigned int pid) const;
              double               poly_perimeter          (const unsigned int pid) const;
              double               poly_mass               (const unsigned int pid) const override;
              int                  poly_id                 (const std::vector<unsigned int> & vlist) const;
              int                  poly_shared             (const unsigned int eid0, const unsigned int eid1) const;
              bool                 polys_are_adjacent      (const unsigned int pid0, const unsigned int pid1) const;
              std::vector<unsigned int>    polys_adjacent_along    (const unsigned int pid, const unsigned int vid0, const unsigned int vid1) const;
              std::vector<unsigned int>    polys_adjacent_along    (const unsigned int pid, const unsigned int eid) const;
              void                 poly_flip_winding_order (const unsigned int pid);
              void                 poly_switch_id          (const unsigned int pid0, const unsigned int pid1);
              bool                 poly_is_boundary        (const unsigned int pid) const;
              unsigned int                 poly_add                (const std::vector<unsigned int> & vlist);
              void                 poly_remove_unreferenced(const unsigned int pid);
              void                 poly_remove             (const unsigned int pid);
              void                 polys_remove            (const std::vector<unsigned int> & pids);
              int                  poly_opposite_to        (const unsigned int eid, const unsigned int pid) const;
              bool                 poly_verts_are_CCW      (const unsigned int pid, const unsigned int curr, const unsigned int prev) const;
              std::vector<vec3d>   poly_vlist              (const unsigned int pid) const;
        const std::vector<unsigned int>  & poly_tessellation       (const unsigned int pid) const;
              void                 poly_export_element     (const unsigned int pid, std::vector<vec3d> & verts, std::vector<std::vector<unsigned int>> & faces) const override;
};

}

#include "abstract_polygonmesh.tpp"

#endif //CINO_ABSTRACT_POLYGON_MESH_H
