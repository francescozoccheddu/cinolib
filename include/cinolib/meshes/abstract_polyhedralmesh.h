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
#ifndef CINO_ABSTRACT_POLYHEDRAL_MESH_H
#define CINO_ABSTRACT_POLYHEDRAL_MESH_H

#include <cinolib/meshes/abstract_mesh.h>
#include <cinolib/meshes/mesh_attributes.h>
#include <cinolib/ipair.h>

namespace cinolib
{

template<class M = Mesh_std_attributes,
         class V = Vert_std_attributes,
         class E = Edge_std_attributes,
         class F = Polygon_std_attributes,
         class P = Polyhedron_std_attributes>
class AbstractPolyhedralMesh : public AbstractMesh<M,V,E,P>
{
    protected:

        std::vector<std::vector<unsigned int>> faces;              // list of faces (assumed CCW)
        std::vector<std::vector<bool>> polys_face_winding; // true if the face is CCW, false if it is CW

        std::vector<F> f_data;

        std::vector<std::vector<unsigned int>> v2f; // vert to face adjacency
        std::vector<std::vector<unsigned int>> e2f; // edge to face adjacency
        std::vector<std::vector<unsigned int>> f2e; // face to edge adjacency
        std::vector<std::vector<unsigned int>> f2f; // face to face adjacency (through edges)
        std::vector<std::vector<unsigned int>> f2p; // face to poly adjacency
        std::vector<std::vector<unsigned int>> p2v; // poly to vert adjacency

        std::vector<std::vector<unsigned int>> face_triangles; // per face serialized triangulation (e.g., for rendering)

    public:

        typedef F F_type;

        explicit AbstractPolyhedralMesh() : AbstractMesh<M,V,E,P>() {}
        ~AbstractPolyhedralMesh() {}

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void clear() override;

        void init(const std::vector<vec3d>             & verts,
                  const std::vector<std::vector<unsigned int>> & faces,
                  const std::vector<std::vector<unsigned int>> & polys,
                  const std::vector<std::vector<bool>> & polys_face_winding);

        void init(const std::vector<vec3d>             & verts,
                  const std::vector<std::vector<unsigned int>> & polys);

        void init(const std::vector<vec3d>             & verts,
                  const std::vector<std::vector<unsigned int>> & polys,
                  const std::vector<int>               & vert_labels,
                  const std::vector<int>               & poly_labels);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        double mesh_srf_area() const;
        double mesh_volume()   const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void normalize_volume();

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        int Euler_characteristic() const override;
        int genus() const override;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

                void update_normals() override;
                void update_f_normals();
        virtual void update_f_normal(const unsigned int fid) = 0;
                void update_f_tessellation();
                void update_f_tessellation(const unsigned int fid);
                void update_v_normals();
                void update_v_normal(const unsigned int vid);
                void update_quality();
                void update_p_quality(const unsigned int pid);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void operator+=(const AbstractPolyhedralMesh<M,V,E,F,P> & m);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        virtual unsigned int verts_per_poly(const unsigned int pid) const override { return this->p2v.at(pid).size();   }
        virtual unsigned int faces_per_poly(const unsigned int pid) const          { return this->polys.at(pid).size(); }
        virtual unsigned int verts_per_face(const unsigned int fid) const          { return this->faces.at(fid).size(); }
        virtual unsigned int edges_per_face(const unsigned int fid) const          { return this->faces.at(fid).size(); }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        unsigned int num_srf_verts() const;
        unsigned int num_srf_edges() const;
        unsigned int num_srf_faces() const;
        unsigned int num_srf_polys() const;
        unsigned int num_faces()     const { return faces.size(); }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        const std::vector<std::vector<unsigned int>> & vector_faces() const { return faces; }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        std::vector<unsigned int> get_surface_verts() const;
        std::vector<unsigned int> get_surface_edges() const;
        std::vector<unsigned int> get_surface_faces() const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        const std::vector<unsigned int> & adj_v2f(const unsigned int vid) const          { return v2f.at(vid);         }
              std::vector<unsigned int> & adj_v2f(const unsigned int vid)                { return v2f.at(vid);         }
        const std::vector<unsigned int> & adj_e2f(const unsigned int eid) const          { return e2f.at(eid);         }
              std::vector<unsigned int> & adj_e2f(const unsigned int eid)                { return e2f.at(eid);         }
        const std::vector<unsigned int> & adj_f2v(const unsigned int fid) const          { return this->faces.at(fid); }
              std::vector<unsigned int> & adj_f2v(const unsigned int fid)                { return this->faces.at(fid); }
        const std::vector<unsigned int> & adj_f2e(const unsigned int fid) const          { return f2e.at(fid);         }
              std::vector<unsigned int> & adj_f2e(const unsigned int fid)                { return f2e.at(fid);         }
        const std::vector<unsigned int> & adj_f2f(const unsigned int fid) const          { return f2f.at(fid);         }
              std::vector<unsigned int> & adj_f2f(const unsigned int fid)                { return f2f.at(fid);         }
        const std::vector<unsigned int> & adj_f2p(const unsigned int fid) const          { return f2p.at(fid);         }
              std::vector<unsigned int> & adj_f2p(const unsigned int fid)                { return f2p.at(fid);         }
        const std::vector<unsigned int> & adj_p2f(const unsigned int pid) const          { return this->polys.at(pid); }
              std::vector<unsigned int> & adj_p2f(const unsigned int pid)                { return this->polys.at(pid); }
        const std::vector<unsigned int> & adj_p2v(const unsigned int pid) const override { return p2v.at(pid);         }
              std::vector<unsigned int> & adj_p2v(const unsigned int pid)       override { return p2v.at(pid);         }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        const F & face_data(const unsigned int fid) const { return f_data.at(fid); }
              F & face_data(const unsigned int fid)       { return f_data.at(fid); }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        // useful for GUIs with mouse picking
        unsigned int pick_poly(const vec3d& p, bool include_hidden = false, bool include_inner = false) const;
        unsigned int pick_face(const vec3d& p, bool include_hidden = false, bool include_inner = false) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void               vert_switch_id             (const unsigned int vid0, const unsigned int vid1);
        void               vert_remove                (const unsigned int vid);
        void               vert_remove_unreferenced   (const unsigned int vid);
        unsigned int               vert_add                   (const vec3d & pos);
        bool               vert_is_on_srf             (const unsigned int vid) const;
        double             vert_mass                  (const unsigned int vid) const override;
        double             vert_volume                (const unsigned int vid) const;
        bool               vert_is_manifold           (const unsigned int vid) const;
        bool               vert_is_visible            (const unsigned int vid) const;
        int                vert_shared_between_faces  (const std::vector<unsigned int> & fids) const;
        std::vector<unsigned int>  vert_verts_link            (const unsigned int vid) const; // see https://en.wikipedia.org/wiki/Simplicial_complex#Closure,_star,_and_link for adefinition of link and star
        std::vector<unsigned int>  vert_edges_link            (const unsigned int vid) const;
        std::vector<unsigned int>  vert_faces_link            (const unsigned int vid) const;
        std::vector<ipair> vert_adj_visible_faces     (const unsigned int vid, const vec3d dir, const double ang_thresh = 60.0);
        std::vector<unsigned int>  vert_adj_srf_verts         (const unsigned int vid) const;
        std::vector<unsigned int>  vert_adj_srf_edges         (const unsigned int vid) const;
        std::vector<unsigned int>  vert_adj_srf_faces         (const unsigned int vid) const;
        std::vector<unsigned int>  vert_ordered_srf_vert_ring (const unsigned int vid, const bool CCW = false) const;
        std::vector<unsigned int>  vert_ordered_srf_edge_ring (const unsigned int vid, const bool CCW = false) const;
        std::vector<unsigned int>  vert_ordered_srf_face_ring (const unsigned int vid, const bool CCW = false) const;
        void               vert_ordered_srf_one_ring  (const unsigned int vid,
                                                       std::vector<unsigned int> & v_ring,
                                                       std::vector<unsigned int> & e_ring,
                                                       std::vector<unsigned int> & f_ring,
                                                       const bool          CCW = false) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void              edge_switch_id             (const unsigned int eid0, const unsigned int eid1);
        unsigned int              edge_add                   (const unsigned int vid0, const unsigned int vid1);
        bool              edge_is_manifold           (const unsigned int eid) const;
        void              edge_remove                (const unsigned int eid);
        void              edge_remove_unreferenced   (const unsigned int eid);
        bool              edge_is_on_srf             (const unsigned int eid) const;
        bool              edge_is_incident_to_srf    (const unsigned int eid) const;
        bool              edge_has_border_on_srf     (const unsigned int eid) const;
        std::vector<unsigned int> edge_ordered_poly_ring     (const unsigned int eid) const;
        std::vector<unsigned int> edge_adj_srf_faces         (const unsigned int eid) const;
        std::vector<unsigned int> edge_verts_link            (const unsigned int eid) const;
        std::vector<unsigned int> edge_edges_link            (const unsigned int eid) const;
        std::vector<unsigned int> edge_faces_link            (const unsigned int eid) const;
        unsigned int              edge_split                 (const unsigned int eid, const vec3d & p);
        double            edge_dihedral_angle        (const unsigned int eid) const override;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

                vec3d              face_vert                  (const unsigned int fid, const unsigned int off) const;
                std::vector<vec3d> face_verts                 (const unsigned int fid) const;
                std::vector<unsigned int>  face_verts_id              (const unsigned int fid, const bool sort_by_vid = false) const;
                unsigned int               face_vert_id               (const unsigned int fid, const unsigned int off) const;
                unsigned int               face_vert_offset           (const unsigned int fid, const unsigned int vid) const;
                unsigned int               face_edge_id               (const unsigned int fid, const unsigned int vid0, const unsigned int vid1) const;
                unsigned int               face_edge_id               (const unsigned int fid, const unsigned int off) const;
                std::vector<unsigned int>  face_v2e                   (const unsigned int fid, const unsigned int vid) const;
                std::vector<unsigned int>  face_v2v                   (const unsigned int fid, const unsigned int vid) const;
                bool               face_is_on_srf             (const unsigned int fid) const;
                bool               face_contains_vert         (const unsigned int fid, const unsigned int vid) const;
                bool               face_contains_edge         (const unsigned int fid, const unsigned int eid) const;
                bool               face_winding_agrees_with   (const unsigned int fid, const unsigned int vid0, const unsigned int vid1) const;
                ipair              face_edges_from_vert       (const unsigned int fid, const unsigned int vid) const;
                unsigned int               face_adj_srf_edge          (const unsigned int fid, const unsigned int eid, const unsigned int vid) const;
                unsigned int               face_opp_to_srf_edge       (const unsigned int fid, const unsigned int eid) const;
                unsigned int               face_shared_edge           (const unsigned int fid0, const unsigned int fid1) const;
                vec3d              face_centroid              (const unsigned int fid) const;
                double             face_mass                  (const unsigned int fid) const;
                double             face_area                  (const unsigned int fid) const;
                bool               faces_are_disjoint         (const unsigned int fid0, const unsigned int fid1) const;
                bool               faces_are_adjacent         (const unsigned int fid0, const unsigned int fid1) const;
                bool               faces_share_poly           (const unsigned int fid0, const unsigned int fid1) const;
                int                face_id                    (const std::vector<unsigned int> & f) const;
                bool               face_is_tri                (const unsigned int fid) const;
                bool               face_is_quad               (const unsigned int fid) const;
        virtual void               face_set_color             (const Color & c);
        virtual void               face_set_alpha             (const float alpha);
                void               face_switch_id             (const unsigned int fid0, const unsigned int fid1);
                unsigned int               face_add                   (const std::vector<unsigned int> & f);
                void               face_remove                (const unsigned int fid);
                void               face_remove_unreferenced   (const unsigned int fid);
                std::vector<unsigned int>  face_tessellation          (const unsigned int fid) const;
                bool               face_is_visible            (const unsigned int fid, unsigned int & pid_beneath) const;
                void               face_apply_labels          (const std::vector<int> & labels);
                void               face_apply_label           (const int label);
                bool               face_verts_are_CCW         (const unsigned int fid, const unsigned int curr, const unsigned int prev) const;
                unsigned int               face_split_along_new_edge  (const unsigned int fid, const unsigned int vid0, const unsigned int vid1);
                unsigned int               face_split_in_triangles    (const unsigned int fid, const vec3d & p);
                bool               face_has_no_duplicate_verts(const unsigned int fid) const;
                void               face_set_flag              (const int flag, const bool b);
                void               face_set_flag              (const int flag, const bool b, const std::vector<unsigned int> & fids);
                bool               face_has_border_on_srf     (const unsigned int fid) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        virtual double             poly_volume                 (const unsigned int pid) const = 0;
                double             poly_mass                   (const unsigned int pid) const override;
                bool               poly_contains_face          (const unsigned int pid, const unsigned int fid) const;
                bool               poly_is_on_surf             (const unsigned int pid) const;
                int                poly_id                     (const std::vector<unsigned int> & flist) const;
                int                poly_shared_face            (const unsigned int pid0, const unsigned int pid1) const;
                unsigned int               poly_face_id                (const unsigned int pid, const unsigned int off) const;
                void               poly_face_flip_winding      (const unsigned int pid, const unsigned int fid);
                bool               poly_face_winding           (const unsigned int pid, const unsigned int fid) const;
                void               poly_flip_winding           (const unsigned int pid);
                bool               poly_face_is_CCW            (const unsigned int pid, const unsigned int fid) const;
                bool               poly_face_is_CW             (const unsigned int pid, const unsigned int fid) const;
                unsigned int               poly_face_offset            (const unsigned int pid, const unsigned int fid) const;
                vec3d              poly_face_normal            (const unsigned int pid, const unsigned int fid) const;
                int                poly_adj_through_face       (const unsigned int pid, const unsigned int fid) const;
                std::vector<unsigned int>  poly_v2f                    (const unsigned int pid, const unsigned int vid) const;
                std::vector<unsigned int>  poly_e2f                    (const unsigned int pid, const unsigned int eid) const;
                std::vector<unsigned int>  poly_f2f                    (const unsigned int pid, const unsigned int fid) const;
                void               poly_switch_id              (const unsigned int pid0, const unsigned int pid1);
                unsigned int               poly_add                    (const std::vector<unsigned int> & flist, const std::vector<bool> & fwinding);
                unsigned int               poly_add                    (const std::vector<unsigned int> & vlist);
                void               poly_remove_unreferenced    (const unsigned int pid);
                void               poly_remove                 (const unsigned int pid, const bool delete_dangling_elements = true);
                std::vector<unsigned int>  poly_dangling_vids  (const unsigned int pid) const;
                std::vector<unsigned int>  poly_dangling_eids  (const unsigned int pid) const;
                std::vector<unsigned int>  poly_dangling_fids  (const unsigned int pid) const;
                void               poly_dangling_ids           (const unsigned int pid, std::vector<unsigned int>& vids, std::vector<unsigned int>& eids, std::vector<unsigned int>& fids) const;
                void               poly_disconnect             (const unsigned int pid, const std::vector<unsigned int>& dangling_vids, const std::vector<unsigned int>& dangling_eids, const std::vector<unsigned int>& dangling_fids);
                void               polys_remove                (const std::vector<unsigned int> & pids);
                unsigned int               poly_face_adj_through_edge  (const unsigned int pid, const unsigned int fid, const unsigned int eid) const;
                bool               poly_faces_share_orientation(const unsigned int pid, const unsigned int fid0, const unsigned int fid1) const;
                bool               poly_fix_orientation        (const unsigned int pid, const unsigned int fid);
                bool               poly_fix_orientation        ();
                int                poly_Euler_characteristic   (const unsigned int pid) const;
                int                poly_genus                  (const unsigned int pid) const;
                bool               poly_is_spherical           (const unsigned int pid) const;
                void               poly_export_element         (const unsigned int pid, std::vector<vec3d> & verts, std::vector<std::vector<unsigned int>> & faces) const override;
                std::vector<unsigned int>  poly_faces_id               (const unsigned int pid, const bool sort_by_fid = false) const;
                std::vector<bool>  poly_faces_winding          (const unsigned int pid) const;
                unsigned int               poly_split_along_new_face   (const unsigned int pid, const std::vector<unsigned int> & f);
                void               poly_reorder_p2v            (const unsigned int pid);
                bool               poly_is_hexahedron          (const unsigned int pid) const;
                bool               poly_is_tetrahedron         (const unsigned int pid) const;
                bool               poly_is_prism               (const unsigned int pid) const;
                bool               poly_is_prism               (const unsigned int pid, const unsigned int fid) const; // check if it is a prism using fid as base
                bool               poly_is_hexable_w_midpoint  (const unsigned int pid) const; // check if this element can be hexed with midpoint subdivision

};

}

#include "abstract_polyhedralmesh.tpp"

#endif // CINO_ABSTRACT_POLYHEDRAL_MESH_H
