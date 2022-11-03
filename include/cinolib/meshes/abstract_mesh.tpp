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
#include <cinolib/meshes/abstract_mesh.h>
#include <cinolib/meshes/mesh_attributes.h>
#include <cinolib/stl_container_utilities.h>
#include <cinolib/min_max_inf.h>
#include <map>
#include <unordered_set>
#include <unordered_map>

namespace cinolib
{

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractMesh<M,V,E,P>::mesh_is_surface() const
{
    switch (mesh_type())
    {
        case TRIMESH       : return true;
        case QUADMESH      : return true;
        case POLYGONMESH   : return true;
        case TETMESH       : return false;
        case HEXMESH       : return false;
        case POLYHEDRALMESH: return false;
        default : assert(false);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractMesh<M,V,E,P>::mesh_is_volumetric() const
{
    return !mesh_is_surface();
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::clear()
{
    bb.reset();
    //
    verts.clear();
    edges.clear();
    polys.clear();
    //
    M std_M_data;
    m_data = std_M_data;
    v_data.clear();
    e_data.clear();
    p_data.clear();
    //
    v2v.clear();
    v2e.clear();
    v2p.clear();
    e2p.clear();
    p2e.clear();
    p2p.clear();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
vec3d AbstractMesh<M,V,E,P>::centroid() const
{
    vec3d bary{0,0,0};
    for(auto p : verts) bary += p;
    if (num_verts() > 0) bary/=static_cast<double>(num_verts());
    return bary;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::translate(const vec3d & delta)
{
    for(unsigned int vid=0; vid<num_verts(); ++vid) vert(vid) += delta;
    bb.min += delta;
    bb.max += delta;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::rotate(const vec3d & axis, const double angle)
{
    vec3d  c = centroid();
    mat3d R = mat3d::ROT_3D(axis, angle);

    for(unsigned int vid=0; vid<num_verts(); ++vid)
    {
        vert(vid) -= c;
        vert(vid)  = R*vert(vid);
        vert(vid) += c;
    }
    //
    if(m_data.update_bbox)    update_bbox();
    if(m_data.update_normals) update_normals();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::scale(const double scale_factor)
{
    vec3d c = centroid();
    translate(-c);
    for(unsigned int vid=0; vid<num_verts(); ++vid) vert(vid) *= scale_factor;
    translate(c);
    if(m_data.update_bbox) update_bbox();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::normalize_bbox()
{
    double s = 1.0/bbox().diag();
    for(unsigned int vid=0; vid<num_verts(); ++vid) vert(vid) *= s;
    if(m_data.update_bbox) update_bbox();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::update_bbox()
{
    bb.reset();
    bb.push(this->verts);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<vec3d> AbstractMesh<M,V,E,P>::vector_vert_normals() const
{
    std::vector<vec3d> normals;
    normals.reserve(num_verts());
    for(unsigned int vid=0; vid<num_verts(); ++vid)
    {
        normals.push_back(vert_data(vid).normal);
    }
    return normals;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<Color> AbstractMesh<M,V,E,P>::vector_vert_colors() const
{
    std::vector<Color> colors;
    colors.reserve(num_verts());
    for(unsigned int vid=0; vid<num_verts(); ++vid)
    {
        colors.push_back(vert_data(vid).color);
    }
    return colors;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<int> AbstractMesh<M,V,E,P>::vector_vert_labels() const
{
    std::vector<int> labels;
    labels.reserve(num_verts());
    for(unsigned int vid=0; vid<num_verts(); ++vid)
    {
        labels.push_back(vert_data(vid).label);
    }
    return labels;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<Color> AbstractMesh<M,V,E,P>::vector_edge_colors() const
{
    std::vector<Color> colors;
    colors.reserve(num_edges());
    for(unsigned int eid=0; eid<num_edges(); ++eid)
    {
        colors.push_back(edge_data(eid).color);
    }
    return colors;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<int> AbstractMesh<M,V,E,P>::vector_edge_labels() const
{
    std::vector<int> labels;
    labels.reserve(num_edges());
    for(unsigned int eid=0; eid<num_edges(); ++eid)
    {
        labels.push_back(edge_data(eid).label);
    }
    return labels;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<vec3d> AbstractMesh<M,V,E,P>::vector_poly_normals() const
{
    std::vector<vec3d> normals;
    normals.reserve(num_polys());
    for(unsigned int pid=0; pid<num_polys(); ++pid)
    {
        normals.push_back(poly_data(pid).normal);
    }
    return normals;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<Color> AbstractMesh<M,V,E,P>::vector_poly_colors() const
{
    std::vector<Color> colors;
    colors.reserve(num_polys());
    for(unsigned int pid=0; pid<num_polys(); ++pid)
    {
        colors.push_back(poly_data(pid).color);
    }
    return colors;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<int> AbstractMesh<M,V,E,P>::vector_poly_labels() const
{
    std::vector<int> labels;
    labels.reserve(num_polys());
    for(unsigned int pid=0; pid<num_polys(); ++pid)
    {
        labels.push_back(poly_data(pid).label);
    }
    return labels;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<Color> AbstractMesh<M,V,E,P>::vector_poly_unique_colors() const
{
    std::vector<Color> colors = vector_poly_colors();
    REMOVE_DUPLICATES_FROM_VEC(colors);
    return colors;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<int> AbstractMesh<M,V,E,P>::vector_poly_unique_labels() const
{
    std::vector<int> labels = vector_poly_labels();
    REMOVE_DUPLICATES_FROM_VEC(labels);
    return labels;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<double> AbstractMesh<M,V,E,P>::serialize_uvw(const int mode) const
{
    std::vector<double> uvw;
    uvw.reserve(num_verts());
    for(unsigned int vid=0; vid<num_verts(); ++vid)
    {
        switch (mode)
        {
            case U_param  : uvw.push_back(vert_data(vid).uvw[0]); break;
            case V_param  : uvw.push_back(vert_data(vid).uvw[1]); break;
            case W_param  : uvw.push_back(vert_data(vid).uvw[2]); break;
            case UV_param : uvw.push_back(vert_data(vid).uvw[0]);
                            uvw.push_back(vert_data(vid).uvw[1]); break;
            case UW_param : uvw.push_back(vert_data(vid).uvw[0]);
                            uvw.push_back(vert_data(vid).uvw[2]); break;
            case VW_param : uvw.push_back(vert_data(vid).uvw[1]);
                            uvw.push_back(vert_data(vid).uvw[2]); break;
            case UVW_param: uvw.push_back(vert_data(vid).uvw[0]);
                            uvw.push_back(vert_data(vid).uvw[1]);
                            uvw.push_back(vert_data(vid).uvw[2]); break;
            default: assert(false);
        }
    }
    return uvw;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::deserialize_uvw(const std::vector<vec3d> & uvw)
{
    assert(uvw.size()==num_verts());
    for(unsigned int vid=0; vid<num_verts(); ++vid)
    {
        vert_data(vid).uvw = uvw.at(vid);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::copy_xyz_to_uvw(const int mode)
{
    for(unsigned int vid=0; vid<num_verts(); ++vid)
    {
        switch (mode)
        {
            case U_param  : vert_data(vid).uvw[0] = vert(vid).x(); break;
            case V_param  : vert_data(vid).uvw[1] = vert(vid).y(); break;
            case W_param  : vert_data(vid).uvw[2] = vert(vid).z(); break;
            case UV_param : vert_data(vid).uvw[0] = vert(vid).x();
                            vert_data(vid).uvw[1] = vert(vid).y(); break;
            case UW_param : vert_data(vid).uvw[0] = vert(vid).x();
                            vert_data(vid).uvw[2] = vert(vid).z(); break;
            case VW_param : vert_data(vid).uvw[1] = vert(vid).y();
                            vert_data(vid).uvw[2] = vert(vid).z(); break;
            case UVW_param: vert_data(vid).uvw[0] = vert(vid).x();
                            vert_data(vid).uvw[1] = vert(vid).y();
                            vert_data(vid).uvw[2] = vert(vid).z(); break;
            default: assert(false);
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::copy_uvw_to_xyz(const int mode)
{
    for(unsigned int vid=0; vid<num_verts(); ++vid)
    {
        switch (mode)
        {
            case U_param  : vert(vid).x() = vert_data(vid).uvw[0]; break;
            case V_param  : vert(vid).y() = vert_data(vid).uvw[1]; break;
            case W_param  : vert(vid).z() = vert_data(vid).uvw[2]; break;
            case UV_param : vert(vid).x() = vert_data(vid).uvw[0];
                            vert(vid).y() = vert_data(vid).uvw[1]; break;
            case UW_param : vert(vid).x() = vert_data(vid).uvw[0];
                            vert(vid).z() = vert_data(vid).uvw[2]; break;
            case VW_param : vert(vid).y() = vert_data(vid).uvw[1];
                            vert(vid).z() = vert_data(vid).uvw[2]; break;
            case UVW_param: vert(vid).x() = vert_data(vid).uvw[0];
                            vert(vid).y() = vert_data(vid).uvw[1];
                            vert(vid).z() = vert_data(vid).uvw[2]; break;
            default: assert(false);
        }
    }
    if(m_data.update_bbox) update_bbox();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::swap_xyz_uvw(const bool normals, const bool bbox)
{
    for(unsigned int vid=0; vid<num_verts(); ++vid)
    {
        std::swap(vert(vid),vert_data(vid).uvw);
    }
    if(normals) update_normals();
    if(bbox)    update_bbox();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractMesh<M,V,E,P>::adj_e2v(const unsigned int eid) const
{
    return { this->edge_vert_id(eid,0),
             this->edge_vert_id(eid,1) };
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractMesh<M,V,E,P>::adj_e2e(const unsigned int eid) const
{
    std::unordered_set<unsigned int> unique_e_list;
    unsigned int v0 = this->edge_vert_id(eid,0);
    unsigned int v1 = this->edge_vert_id(eid,1);
    for(unsigned int nbr : this->adj_v2e(v0)) if(nbr != eid) unique_e_list.insert(nbr);
    for(unsigned int nbr : this->adj_v2e(v1)) if(nbr != eid) unique_e_list.insert(nbr);
    std::vector<unsigned int> e_list(unique_e_list.begin(), unique_e_list.end());
    return e_list;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::set<unsigned int> AbstractMesh<M,V,E,P>::vert_n_ring(const unsigned int vid, const unsigned int n) const
{
    std::set<unsigned int> active_set;
    std::set<unsigned int> ring;

    active_set.insert(vid);
    for(unsigned int i=0; i<n; ++i)
    {
        std::set<unsigned int> next_active_set;

        for(unsigned int curr : active_set)
        for(unsigned int nbr  : adj_v2v(curr))
        {
            if (DOES_NOT_CONTAIN(ring,nbr) && nbr != vid) next_active_set.insert(nbr);
            ring.insert(nbr);
        }

        active_set = next_active_set;
    }
    return ring;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractMesh<M,V,E,P>::verts_are_adjacent(const unsigned int vid0, const unsigned int vid1) const
{
    for(unsigned int nbr : adj_v2v(vid0)) if (vid1==nbr) return true;
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractMesh<M,V,E,P>::vert_opposite_to(const unsigned int eid, const unsigned int vid) const
{
    assert(this->edge_contains_vert(eid, vid));
    if (this->edge_vert_id(eid,0) != vid) return this->edge_vert_id(eid,0);
    else                                  return this->edge_vert_id(eid,1);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::vert_weights(const unsigned int vid, const int type, std::vector<std::pair<unsigned int,double>> & wgts) const
{
    switch (type)
    {
        case UNIFORM : vert_weights_uniform(vid, wgts); return;
        default      : assert(false && "Vert weights not supported at this level of the hierarchy!");
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::vert_weights_uniform(const unsigned int vid, std::vector<std::pair<unsigned int,double>> & wgts) const
{
    wgts.clear();
    double w = 1.0; // / (double)nbrs.size(); // <= WARNING: makes the matrix non-symmetric!!!!!
    for(unsigned int nbr : adj_v2v(vid))
    {
        wgts.push_back(std::make_pair(nbr,w));
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractMesh<M,V,E,P>::vert_is_local_min(const unsigned int vid, const int tex_coord) const
{
    for(unsigned int nbr : adj_v2v(vid))
    {
        switch (tex_coord)
        {
            case U_param : if (vert_data(nbr).uvw[0] < vert_data(vid).uvw[0]) return false; break;
            case V_param : if (vert_data(nbr).uvw[1] < vert_data(vid).uvw[1]) return false; break;
            case W_param : if (vert_data(nbr).uvw[2] < vert_data(vid).uvw[2]) return false; break;
            default: assert(false);
        }
    }
    return true;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractMesh<M,V,E,P>::vert_is_local_max(const unsigned int vid, const int tex_coord) const
{
    for(unsigned int nbr : adj_v2v(vid))
    {
        switch (tex_coord)
        {
            case U_param : if (vert_data(nbr).uvw[0] > vert_data(vid).uvw[0]) return false; break;
            case V_param : if (vert_data(nbr).uvw[1] > vert_data(vid).uvw[1]) return false; break;
            case W_param : if (vert_data(nbr).uvw[2] > vert_data(vid).uvw[2]) return false; break;
            default: assert(false);
        }
    }
    return true;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractMesh<M,V,E,P>::vert_valence(const unsigned int vid) const
{
    assert(adj_v2v(vid).size() == adj_v2e(vid).size());
    return adj_v2v(vid).size();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractMesh<M,V,E,P>::vert_shared(const unsigned int eid0, const unsigned int eid1) const
{
    unsigned int e00 = edge_vert_id(eid0,0);
    unsigned int e01 = edge_vert_id(eid0,1);
    unsigned int e10 = edge_vert_id(eid1,0);
    unsigned int e11 = edge_vert_id(eid1,1);
    if (e00 == e10 || e00 == e11) return e00;
    if (e01 == e10 || e01 == e11) return e01;
    return -1;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
int AbstractMesh<M,V,E,P>::vert_shared_between_polys(const std::vector<unsigned int> & pids) const
{
    for(unsigned int vid : this->adj_p2v(pids.front()))
    {
        bool shared = true;
        for(unsigned int i=1; i<pids.size(); ++i)
        {
            if(!this->poly_contains_vert(pids.at(i),vid))
            {
                shared = false;
                break;
            }
        }
        if(shared) return vid;
    }
    return -1;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractMesh<M,V,E,P>::vert_min_uvw_value(const int tex_coord) const
{
    double min = inf_double;
    for(unsigned int vid=0; vid<num_verts(); ++vid)
    {
        switch (tex_coord)
        {
            case U_param : min = std::min(min, vert_data(vid).uvw[0]); break;
            case V_param : min = std::min(min, vert_data(vid).uvw[1]); break;
            case W_param : min = std::min(min, vert_data(vid).uvw[2]); break;
            default: assert(false);
        }
    }
    return min;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractMesh<M,V,E,P>::vert_max_uvw_value(const int tex_coord) const
{
    double max = -inf_double;
    for(unsigned int vid=0; vid<num_verts(); ++vid)
    {
        switch (tex_coord)
        {
            case U_param : max = std::max(max, vert_data(vid).uvw[0]); break;
            case V_param : max = std::max(max, vert_data(vid).uvw[1]); break;
            case W_param : max = std::max(max, vert_data(vid).uvw[2]); break;
            default: assert(false);
        }
    }
    return max;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::vert_set_color(const Color & c)
{
    for(unsigned int vid=0; vid<num_verts(); ++vid)
    {
        vert_data(vid).color = c;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::vert_set_alpha(const float alpha)
{
    for(unsigned int vid=0; vid<num_verts(); ++vid)
    {
        vert_data(vid).color.a() = alpha;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractMesh<M,V,E,P>::edge_vert_id(const unsigned int eid, const unsigned int offset) const
{
    unsigned int   eid_ptr = eid * 2;
    return edges.at(eid_ptr + offset);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractMesh<M,V,E,P>::edge_vert_ids(const unsigned int eid) const
{
    return { this->edge_vert_id(eid,0), this->edge_vert_id(eid,1) };
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<vec3d> AbstractMesh<M,V,E,P>::edge_verts(const unsigned int eid) const
{
    return {this->edge_vert(eid,0), this->edge_vert(eid,1)};
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
int AbstractMesh<M,V,E,P>::edge_id(const unsigned int vid0, const unsigned int vid1) const
{
    assert(vid0 != vid1);
    for(unsigned int eid : adj_v2e(vid0))
    {
        if(edge_contains_vert(eid,vid0) && edge_contains_vert(eid,vid1))
        {
            return eid;
        }
    }
    return -1;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
int AbstractMesh<M,V,E,P>::edge_id(const ipair & vids) const
{
    return edge_id(vids.first, vids.second);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
int AbstractMesh<M,V,E,P>::edge_id(const std::vector<unsigned int> & vids) const
{
    assert(vids.size()==2);
    return edge_id(vids.front(), vids.back());
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractMesh<M,V,E,P>::edge_valence(const unsigned int eid) const
{
    return this->adj_e2p(eid).size();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
vec3d AbstractMesh<M,V,E,P>::edge_vert(const unsigned int eid, const unsigned int offset) const
{
    return vert(edge_vert_id(eid,offset));
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractMesh<M,V,E,P>::edge_contains_vert(const unsigned int eid, const unsigned int vid) const
{
    if (edge_vert_id(eid,0) == vid) return true;
    if (edge_vert_id(eid,1) == vid) return true;
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractMesh<M,V,E,P>::edges_are_adjacent(const unsigned int eid0, const unsigned int eid1) const
{
    if (edge_vert_id(eid0,0)==edge_vert_id(eid1,0)) return true;
    if (edge_vert_id(eid0,0)==edge_vert_id(eid1,1)) return true;
    if (edge_vert_id(eid0,1)==edge_vert_id(eid1,0)) return true;
    if (edge_vert_id(eid0,1)==edge_vert_id(eid1,1)) return true;
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
vec3d AbstractMesh<M,V,E,P>::edge_sample_at(const unsigned int eid, const double lambda) const
{
    return ((1.0-lambda)*edge_vert(eid,0) + lambda*edge_vert(eid,1));
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractMesh<M,V,E,P>::edge_length(const unsigned int eid) const
{
    return edge_vec(eid).norm();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
vec3d AbstractMesh<M,V,E,P>::edge_vec(const unsigned int eid, const bool normalized) const
{
    vec3d e{edge_vert(eid,1) - edge_vert(eid,0)};
    if(normalized) e.normalize();
    return e;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractMesh<M,V,E,P>::edge_avg_length() const
{
    double avg = 0;
    for(unsigned int eid=0; eid<num_edges(); ++eid) avg += edge_length(eid);
    if (num_edges() > 0) avg/=static_cast<double>(num_edges());
    return avg;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractMesh<M,V,E,P>::edge_avg_length(const unsigned int vid) const
{
    double avg = 0;
    for(unsigned int eid : this->adj_v2e(vid)) avg += edge_length(eid);
    if(num_edges() > 0) avg/=static_cast<double>(this->adj_v2e(vid).size());
    return avg;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractMesh<M,V,E,P>::edge_max_length() const
{
    double max = 0;
    for(unsigned int eid=0; eid<num_edges(); ++eid) max = std::max(max, edge_length(eid));
    return max;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractMesh<M,V,E,P>::edge_min_length() const
{
    double min = inf_double;
    for(unsigned int eid=0; eid<num_edges(); ++eid) min = std::min(min,edge_length(eid));
    return min;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::edge_set_color(const Color & c)
{
    for(unsigned int eid=0; eid<num_edges(); ++eid)
    {
        edge_data(eid).color = c;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::edge_set_alpha(const float alpha)
{
    for(unsigned int eid=0; eid<num_edges(); ++eid)
    {
        edge_data(eid).color.a() = alpha;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractMesh<M,V,E,P>::poly_vert_id(const unsigned int pid, const unsigned int offset) const
{
    return adj_p2v(pid).at(offset);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractMesh<M,V,E,P>::poly_vert_offset(const unsigned int pid, const unsigned int vid) const
{
    assert(poly_contains_vert(pid,vid));
    for(unsigned int off=0; off<verts_per_poly(pid); ++off)
    {
        if(poly_vert_id(pid,off) == vid) return off;
    }
    assert(false);
    return 0; // warning killer
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
vec3d AbstractMesh<M,V,E,P>::poly_centroid(const unsigned int pid) const
{
    vec3d c{0,0,0};
    for(unsigned int vid : adj_p2v(pid)) c += vert(vid);
    c /= static_cast<double>(verts_per_poly(pid));
    return c;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
vec3d AbstractMesh<M,V,E,P>::poly_sample_at(const unsigned int pid, const double bc[]) const
{
    vec3d p{0,0,0};
    for(unsigned int off=0; off<verts_per_poly(pid); ++off)
    {
        p += bc[off] * poly_vert(pid,off);
    }
    return p;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractMesh<M,V,E,P>::poly_sample_param_at(const unsigned int pid, const double bc[], const int tex_coord) const
{
    double val = 0;
    for(unsigned int off=0; off<verts_per_poly(pid); ++off)
    {
        switch(tex_coord)
        {
            case U_param : val += bc[off] * this->vert_data(this->poly_vert_id(pid,off)).uvw[0]; break;
            case V_param : val += bc[off] * this->vert_data(this->poly_vert_id(pid,off)).uvw[1]; break;
            case W_param : val += bc[off] * this->vert_data(this->poly_vert_id(pid,off)).uvw[2]; break;
            default: assert(false);
        }
    }
    return val;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
vec3d AbstractMesh<M,V,E,P>::poly_vert(const unsigned int pid, const unsigned int offset) const
{
    return vert(poly_vert_id(pid,offset));
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<vec3d> AbstractMesh<M,V,E,P>::poly_verts(const unsigned int pid) const
{
    unsigned int nv = this->verts_per_poly(pid);
    std::vector<vec3d> p_list(nv);
    for(unsigned int off=0; off<nv; ++off) p_list.at(off) = this->poly_vert(pid,off);
    return p_list;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractMesh<M,V,E,P>::poly_verts_id(const unsigned int pid, const bool sort_by_vid) const
{
    if(sort_by_vid)
    {
        std::vector<unsigned int> v_list = this->adj_p2v(pid);
        SORT_VEC(v_list);
        return v_list;
    }
    return this->adj_p2v(pid);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractMesh<M,V,E,P>::poly_v2v(const unsigned int pid, const unsigned int vid) const
{
    assert(this->poly_contains_vert(pid,vid));
    std::vector<unsigned int> verts;
    for(unsigned int eid : this->adj_v2e(vid))
    {
        if(this->poly_contains_edge(pid,eid)) verts.push_back(this->vert_opposite_to(eid,vid));
    }
    return verts;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractMesh<M,V,E,P>::poly_v2e(const unsigned int pid, const unsigned int vid) const
{
    assert(this->poly_contains_vert(pid,vid));
    std::vector<unsigned int> edges;
    for(unsigned int eid : this->adj_v2e(vid))
    {
        if(this->poly_contains_edge(pid,eid)) edges.push_back(eid);
    }
    return edges;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractMesh<M,V,E,P>::poly_vert_valence(const unsigned int pid, const unsigned int vid) const
{
    return this->poly_v2v(pid,vid).size();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractMesh<M,V,E,P>::poly_edge_id(const unsigned int fid, const unsigned int vid0, const unsigned int vid1) const
{
    assert(poly_contains_vert(fid,vid0));
    assert(poly_contains_vert(fid,vid1));

    for(unsigned int eid : adj_p2e(fid))
    {
        if (edge_contains_vert(eid,vid0) && edge_contains_vert(eid,vid1)) return eid;
    }

    assert(false);
    return 0; // warning killer
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractMesh<M,V,E,P>::poly_contains_edge(const unsigned int pid, const unsigned int eid) const
{
    for(unsigned int e : adj_p2e(pid)) if (e == eid) return true;
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractMesh<M,V,E,P>::poly_contains_edge(const unsigned int pid, const unsigned int vid0, const unsigned int vid1) const
{
    for(unsigned int eid : adj_p2e(pid))
    {
        if (edge_contains_vert(eid, vid0) &&
            edge_contains_vert(eid, vid1))
        {
            return true;
        }
    }
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::center_bbox()
{
    vec3d center = bb.center();
    for(unsigned int vid=0; vid<num_verts(); ++vid) vert(vid) -= center;
    bb.min -= center;
    bb.max -= center;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::poly_set_color(const Color & c)
{
    for(unsigned int pid=0; pid<num_polys(); ++pid)
    {
        poly_data(pid).color = c;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::poly_set_alpha(const float alpha)
{
    for(unsigned int pid=0; pid<num_polys(); ++pid)
    {
        poly_data(pid).color.a() = alpha;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::poly_color_wrt_label(const bool sorted, const float s, const float v) // s => saturation, v => value in HSV color space
{
    std::map<int,unsigned int> l_map;
    for(unsigned int pid=0; pid<this->num_polys(); ++pid)
    {
        int l = this->poly_data(pid).label;
        if(DOES_NOT_CONTAIN(l_map,l))
        {
            unsigned int fresh_label = l_map.size();
            l_map[l] = fresh_label;
        }
    }
    unsigned int n_labels = l_map.size();
    for(unsigned int pid=0; pid<this->num_polys(); ++pid)
    {
        if(sorted) this->poly_data(pid).color = Color::hsv_ramp(n_labels, this->poly_data(pid).label);
        else       this->poly_data(pid).color = Color::scatter(n_labels,l_map.at(this->poly_data(pid).label), s, v);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::poly_label_wrt_color()
{
    std::map<Color,int> colormap;
    for(unsigned int pid=0; pid<this->num_polys(); ++pid)
    {
        const Color & c = this->poly_data(pid).color;
        if (DOES_NOT_CONTAIN(colormap,c)) colormap[c] = colormap.size();
    }
    for(unsigned int pid=0; pid<this->num_polys(); ++pid)
    {
        this->poly_data(pid).label = colormap.at(this->poly_data(pid).color);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractMesh<M,V,E,P>::poly_contains_vert(const unsigned int pid, const unsigned int vid) const
{
    for(unsigned int v : adj_p2v(pid)) if(v == vid) return true;
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractMesh<M,V,E,P>::polys_n_unique_colors() const
{
    return vector_poly_unique_colors().size();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractMesh<M,V,E,P>::polys_n_unique_labels() const
{
    return vector_poly_unique_labels().size();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractMesh<M,V,E,P>::polys_are_colored() const
{
    return (polys_n_unique_colors()>1);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractMesh<M,V,E,P>::polys_are_labeled() const
{
    return (polys_n_unique_labels()>1);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::poly_apply_labels(const std::vector<int> & labels)
{
    assert(labels.size() == this->num_polys());
    for(unsigned int pid=0; pid<num_polys(); ++pid)
    {
        poly_data(pid).label = labels.at(pid);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::poly_apply_label(const int label)
{
    for(unsigned int pid=0; pid<num_polys(); ++pid)
    {
        poly_data(pid).label = label;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
AABB AbstractMesh<M,V,E,P>::poly_aabb(const unsigned int pid) const
{
    return AABB(poly_verts(pid));
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::edge_apply_labels(const std::vector<int> & labels)
{
    assert(labels.size() == this->num_edges());
    for(unsigned int eid=0; eid<num_edges(); ++eid)
    {
        edge_data(eid).label = labels.at(eid);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::edge_apply_label(const int label)
{
    for(unsigned int eid=0; eid<num_edges(); ++eid)
    {
        edge_data(eid).label = label;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::vert_apply_labels(const std::vector<int> & labels)
{
    assert(labels.size() == this->num_verts());
    for(unsigned int vid=0; vid<num_verts(); ++vid)
    {
        vert_data(vid).label = labels.at(vid);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::vert_apply_label(const int label)
{
    for(unsigned int vid=0; vid<num_verts(); ++vid)
    {
        vert_data(vid).label = label;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::edge_mark_sharp_creases(const float thresh)
{
    for(unsigned int eid=0; eid<this->num_edges(); ++eid)
    {
        if(edge_dihedral_angle(eid) >= thresh)
        {
            this->edge_data(eid).flags[CREASE] = true;
            this->edge_data(eid).flags[MARKED] = true;
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractMesh<M,V,E,P>::pick_vert(const vec3d & p) const
{
    double closest_dist{ inf_double };
    unsigned int closest_vid{};
    for (unsigned int vid{0}; vid < num_verts(); ++vid)
    {
        const double dist{ vert(vid).dist(p) };
        if (dist < closest_dist)
        {
            closest_dist = dist;
            closest_vid = vid;
        }
    }
    return closest_vid;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractMesh<M,V,E,P>::pick_edge(const vec3d & p) const
{
    double closest_dist{ inf_double };
    unsigned int closest_eid{};
    for (unsigned int eid{0}; eid < num_edges(); ++eid)
    {
        const double dist{ edge_sample_at(eid, 0.5).dist(p) };
        if (dist < closest_dist)
        {
            closest_dist = dist;
            closest_eid = eid;
        }
    }
    return closest_eid;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractMesh<M,V,E,P>::pick_poly(const vec3d & p, bool include_hidden) const
{
    double closest_dist{ inf_double };
    unsigned int closest_pid{};
    for (unsigned int pid{ 0 }; pid < num_polys(); ++pid)
    {
        if (!include_hidden && poly_data(pid).flags[HIDDEN])
        {
            continue;
        }
        const double dist{ poly_centroid(pid).dist(p) };
        if (dist < closest_dist)
        {
            closest_dist = dist;
            closest_pid = pid;
        }
    }
    return closest_pid;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::vert_set_flag(const int flag, const bool b)
{
    for(unsigned int vid=0; vid<this->num_verts(); ++vid)
    {
        this->vert_data(vid).flags[flag] = b;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::vert_set_flag(const int flag, const bool b, const std::vector<unsigned int> & vids)
{
    for(unsigned int vid : vids)
    {
        this->vert_data(vid).flags[flag] = b;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::edge_set_flag(const int flag, const bool b)
{
    for(unsigned int eid=0; eid<this->num_edges(); ++eid)
    {
        this->edge_data(eid).flags[flag] = b;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::edge_set_flag(const int flag, const bool b, const std::vector<unsigned int> & eids)
{
    for(unsigned int eid : eids)
    {
        this->edge_data(eid).flags[flag] = b;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::poly_set_flag(const int flag, const bool b)
{
    for(unsigned int pid=0; pid<this->num_polys(); ++pid)
    {
        this->poly_data(pid).flags[flag] = b;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractMesh<M,V,E,P>::poly_set_flag(const int flag, const bool b, const std::vector<unsigned int> & pids)
{
    for(unsigned int pid : pids)
    {
        this->poly_data(pid).flags[flag] = b;
    }
}
 }
