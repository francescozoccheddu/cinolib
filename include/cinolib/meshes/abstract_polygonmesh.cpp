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
#include <cinolib/meshes/abstract_polygonmesh.h>
#include <cinolib/to_openGL_unified_verts.h>
#include <cinolib/io/read_write.h>
#include <cinolib/quality.h>
#include <cinolib/stl_container_utilities.h>
#include <cinolib/geometry/polygon_utils.h>
#include <cinolib/vector_serialization.h>
#include <cinolib/how_many_seconds.h>
#include <cinolib/deg_rad.h>
#include <unordered_set>
#include <cinolib/ANSI_color_codes.h>
#include <queue>

namespace cinolib
{

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::load(const char * filename)
{
    this->clear();
    this->mesh_data().filename = std::string(filename);

    std::vector<vec3d>             pos;      // vertex xyz positions
    std::vector<vec3d>             tex;      // vertex uv(w) texture coordinates
    std::vector<vec3d>             nor;      // vertex normals
    std::vector<std::vector<unsigned int>> poly_pos; // polygons with references to pos
    std::vector<std::vector<unsigned int>> poly_tex; // polygons with references to tex
    std::vector<std::vector<unsigned int>> poly_nor; // polygons with references to nor
    std::vector<Color>             poly_col; // per polygon colors
    std::vector<int>               poly_lab; // per polygon labels

    std::string str(filename);
    std::string filetype = str.substr(str.size()-4,4);

    if (filetype.compare(".off") == 0 ||
        filetype.compare(".OFF") == 0)
    {
        read_OFF(filename, pos, poly_pos, poly_col);
    }
    else if (filetype.compare(".obj") == 0 ||
             filetype.compare(".OBJ") == 0)
    {
        //read_OBJ(filename, pos, poly_pos);
        read_OBJ(filename, pos, tex, nor, poly_pos, poly_tex, poly_nor, poly_col, poly_lab);
    }
    else if (filetype.compare(".stl") == 0 ||
             filetype.compare(".STL") == 0)
    {
        std::vector<unsigned int> tris;
        read_STL(filename, pos, tris);
        poly_pos = polys_from_serialized_vids(tris, 3);
    }
    else
    {
        std::cerr << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : load() : file format not supported yet " << std::endl;
    }

    init(pos, tex, nor, poly_pos, poly_tex, poly_nor, poly_col, poly_lab);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::save(const char * filename) const
{
    std::vector<double> coords = serialized_xyz_from_vec3d(this->verts);

    std::string str(filename);
    std::string filetype = str.substr(str.size()-3,3);

    if (filetype.compare("off") == 0 ||
        filetype.compare("OFF") == 0)
    {
        write_OFF(filename, coords, this->polys);
    }
    else if (filetype.compare("obj") == 0 ||
             filetype.compare("OBJ") == 0)
    {
        if(this->polys_are_colored())
        {
            write_OBJ(filename, coords, this->polys, this->vector_poly_colors());
        }
        else write_OBJ(filename, coords, this->polys);
    }
    else if (filetype.compare("stl") == 0 ||
             filetype.compare("STL") == 0)
    {
        std::vector<double> normals;
        normals.reserve(3*this->num_polys());
        for (unsigned int pid=0; pid<this->num_polys(); ++pid)
        {
            normals.push_back(this->poly_data(pid).normal.x());
            normals.push_back(this->poly_data(pid).normal.y());
            normals.push_back(this->poly_data(pid).normal.z());
        }

        write_STL(filename, serialized_xyz_from_vec3d(this->vector_verts()), this->polys, normals);
    }
    else
    {
        std::cerr << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : write() : file format not supported yet " << std::endl;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::clear()
{
    AbstractMesh<M,V,E,P>::clear();
    poly_triangles.clear();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::init(const std::vector<vec3d>             & verts,
                                        const std::vector<std::vector<unsigned int>> & polys)
{
    std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

    // pre-allocate memory
    unsigned int nv = verts.size();
    unsigned int np = polys.size();
    unsigned int ne = 1.5*np;
    this->verts.reserve(nv);
    this->edges.reserve(ne*2);
    this->polys.reserve(np);
    this->poly_triangles.reserve(np);
    this->v2v.reserve(nv);
    this->v2e.reserve(nv);
    this->v2p.reserve(nv);
    this->e2p.reserve(ne);
    this->p2e.reserve(np);
    this->p2p.reserve(np);
    this->v_data.reserve(nv);
    this->e_data.reserve(ne);
    this->p_data.reserve(np);

    // initialize mesh connectivity (and normals)
    for(auto v : verts) this->vert_add(v);
    for(auto p : polys) this->poly_add(p);

    if(this->mesh_data().update_normals) this->update_v_normals();

    this->copy_xyz_to_uvw(UVW_param);

    for(unsigned int eid=0; eid<this->num_edges(); ++eid)
    {
        this->edge_data(eid).flags[MARKED] = (this->edge_is_boundary(eid) || !this->edge_is_manifold(eid));
    }

    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    std::cout << "load mesh\t"     <<
                 this->num_verts() << "V / " <<
                 this->num_edges() << "E / " <<
                 this->num_polys() << "P  [" <<
                 how_many_seconds(t0,t1) << "s]" << std::endl;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::init(      std::vector<vec3d>             & pos,       // vertex xyz positions
                                              std::vector<vec3d>             & tex,       // vertex uv(w) texture coordinates
                                              std::vector<vec3d>             & nor,       // vertex normals
                                              std::vector<std::vector<unsigned int>> & poly_pos,  // polygons with references to pos
                                        const std::vector<std::vector<unsigned int>> & poly_tex,  // polygons with references to tex
                                        const std::vector<std::vector<unsigned int>> & poly_nor,  // polygons with references to nor
                                        const std::vector<Color>             & poly_col,  // per polygon colors
                                        const std::vector<int>               & poly_lab)  // per polygon labels
{
    // if the model is textured or has multiple normals per vertex (or both),
    // cut mesh along seams to create unique openGL-like vertices having
    // xyz, uvw and normals all condensed in a single entity
    //
    if (poly_pos.size() == poly_tex.size() &&
        poly_pos.size() == poly_nor.size())
    {
        std::vector<vec3d> tmp_xyz, tmp_uvw, tmp_nor;
        std::vector<std::vector<unsigned int>> tmp_poly;
        to_openGL_unified_verts(pos, tex, nor, poly_pos, poly_tex, poly_nor, tmp_xyz, tmp_uvw, tmp_nor, tmp_poly);
        pos      = tmp_xyz;
        tex      = tmp_uvw;
        nor      = tmp_nor;
        poly_pos = tmp_poly;
    }
    else if (poly_pos.size() == poly_tex.size())
    {
        std::vector<vec3d> tmp_xyz, tmp_uvw;
        std::vector<std::vector<unsigned int>> tmp_poly;
        to_openGL_unified_verts(pos, tex, poly_pos, poly_tex, tmp_xyz, tmp_uvw, tmp_poly);
        pos      = tmp_xyz;
        tex      = tmp_uvw;
        poly_pos = tmp_poly;
    }
    else if (poly_pos.size() == poly_nor.size())
    {
        std::vector<vec3d> tmp_xyz, tmp_nor;
        std::vector<std::vector<unsigned int>> tmp_poly;
        to_openGL_unified_verts(pos, nor, poly_pos, poly_nor, tmp_xyz, tmp_nor, tmp_poly);
        pos      = tmp_xyz;
        nor      = tmp_nor;
        poly_pos = tmp_poly;
    }

    init(pos, poly_pos);

    // customize uv(w) coordinates
    if(pos.size()==tex.size())
    {
        std::cout << "load textures" << std::endl;
        for(unsigned int vid=0; vid<this->num_verts(); ++vid)
        {
            this->vert_data(vid).uvw = tex.at(vid);
        }
    }
    else this->copy_xyz_to_uvw(UVW_param);

    // customize normals
    if(pos.size()==nor.size())
    {
        std::cout << "load normals" << std::endl;
        for(unsigned int vid=0; vid<this->num_verts(); ++vid)
        {
            this->vert_data(vid).normal = nor.at(vid);
        }
    }

    // customize colors
    if(poly_col.size()==this->num_polys())
    {
        std::cout << "load per polygon colors" << std::endl;
        for(unsigned int pid=0; pid<this->num_polys(); ++pid)
        {
            this->poly_data(pid).color = poly_col.at(pid);
        }
    }

    if(poly_lab.size()==this->num_polys())
    {
        this->poly_apply_labels(poly_lab);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::update_p_tessellation(const unsigned int pid)
{
    // Assume convexity and try trivial tessellation first. If something flips
    // apply earcut algorithm to get a valid triangulation

    poly_triangles.at(pid).clear();
    std::vector<vec3d> n;
    for(unsigned int i=2; i<this->verts_per_poly(pid); ++i)
    {
        unsigned int vid0 = this->polys.at(pid).at( 0 );
        unsigned int vid1 = this->polys.at(pid).at(i-1);
        unsigned int vid2 = this->polys.at(pid).at( i );

        poly_triangles.at(pid).push_back(vid0);
        poly_triangles.at(pid).push_back(vid1);
        poly_triangles.at(pid).push_back(vid2);

        n.push_back((this->vert(vid1)-this->vert(vid0)).cross(this->vert(vid2)-this->vert(vid0)));
    }

    bool bad_tessellation = false;
    for(unsigned int i=0; i<n.size()-1; ++i) if (n.at(i).dot(n.at(i+1))<0) bad_tessellation = true;

    if(bad_tessellation)
    {
        // NOTE: the triangulation is constructed on a proxy polygon obtained
        // projecting the actual polygon onto the best fitting plane. Bad things
        // can still happen for highly non-planar polygons...

        std::vector<vec3d> vlist(this->verts_per_poly(pid));
        for (unsigned int i=0; i<this->verts_per_poly(pid); ++i)
        {
            vlist.at(i) = this->poly_vert(pid,i);
        }
        //
        std::vector<unsigned int> tris;
        poly_triangles.at(pid).clear();
        if(polygon_triangulate(vlist, tris))
        {
            for(unsigned int off : tris) poly_triangles.at(pid).push_back(this->poly_vert_id(pid,off));
        }
        else
        {
            std::cout << "WARNING: could not triangulate a polygon. Is it degenerate?" << std::endl;
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::update_v_normal(const unsigned int vid)
{
    vec3d n(0,0,0);
    for(unsigned int pid : this->adj_v2p(vid))
    {
        n += this->poly_data(pid).normal;
    }
    if (n.norm()>0) n.normalize();
    this->vert_data(vid).normal = n;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::update_p_normal(const unsigned int pid)
{
    // compute the best fitting plane
    std::vector<vec3d> points;
    for(unsigned int off=0; off<this->verts_per_poly(pid); ++off) points.push_back(this->poly_vert(pid,off));
    this->poly_data(pid).normal = polygon_normal(points);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::update_p_normals()
{
    for(unsigned int pid=0; pid<this->num_polys(); ++pid)
    {
        update_p_normal(pid);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::update_p_tessellations()
{
    for(unsigned int pid=0; pid<this->num_polys(); ++pid)
    {
        update_p_tessellation(pid);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::update_v_normals()
{
    for(unsigned int vid=0; vid<this->num_verts(); ++vid)
    {
        update_v_normal(vid);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::update_normals()
{
    this->update_p_normals();
    this->update_v_normals();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
int AbstractPolygonMesh<M,V,E,P>::Euler_characteristic() const
{
    unsigned int nv = this->num_verts();
    unsigned int ne = this->num_edges();
    unsigned int np = this->num_polys();
    return nv - ne + np;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
int AbstractPolygonMesh<M,V,E,P>::genus() const
{
    return (2-Euler_characteristic())*0.5;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractPolygonMesh<M,V,E,P>::mesh_area() const
{
    double area = 0;
    for(unsigned int pid=0; pid<this->num_polys(); ++pid)
    {
        area += this->poly_area(pid);
    }
    return area;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::normalize_area()
{
    this->scale(1.0/sqrt(this->mesh_area()));
    assert(std::fabs(this->mesh_area()-1)<1e-5);
    if(this->mesh_data().update_bbox) this->update_bbox();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractPolygonMesh<M,V,E,P>::mesh_volume() const
{
    // EFFICIENT FEATURE EXTRACTION FOR 2D/3D OBJECTS IN MESH REPRESENTATION
    // Cha Zhang and Tsuhan Chen
    // Proceedings of the International Conference on Image Processing, 2001

    double vol = 0.0;
    vec3d O(0,0,0);
    for(unsigned int pid=0; pid<this->num_polys(); ++pid)
    {
        for(unsigned int i=0; i<this->poly_tessellation(pid).size()/3; ++i)
        {
            vec3d A    = this->vert(this->poly_tessellation(pid).at(3*i+0));
            vec3d B    = this->vert(this->poly_tessellation(pid).at(3*i+1));
            vec3d C    = this->vert(this->poly_tessellation(pid).at(3*i+2));

            vec3d OA   = A - O;
            vec3d n    = this->poly_data(pid).normal;

            vol += (n.dot(OA) > 0) ?  tet_unsigned_volume(A,B,C,O)
                                   : -tet_unsigned_volume(A,B,C,O);
        }
    }
    assert(vol >= 0);
    return vol;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractPolygonMesh<M,V,E,P>::vert_is_saddle(const unsigned int vid, const int tex_coord) const
{
    std::vector<bool> signs;
    for(unsigned int nbr : vert_ordered_verts_link(vid))
    {
        // Discard == signs. For references, see:
        // Decomposing Polygon Meshes by Means of Critical Points
        // Yinan Zhou and Zhiyong Huang
        //
        switch (tex_coord)
        {
            case U_param : if (this->vert_data(nbr).uvw[0] != this->vert_data(vid).uvw[0]) signs.push_back(this->vert_data(nbr).uvw[0] > this->vert_data(vid).uvw[0]); break;
            case V_param : if (this->vert_data(nbr).uvw[1] != this->vert_data(vid).uvw[1]) signs.push_back(this->vert_data(nbr).uvw[1] > this->vert_data(vid).uvw[1]); break;
            case W_param : if (this->vert_data(nbr).uvw[2] != this->vert_data(vid).uvw[2]) signs.push_back(this->vert_data(nbr).uvw[2] > this->vert_data(vid).uvw[2]); break;
            default: assert(false);
        }
    }

    unsigned int sign_switch = 0;
    for(unsigned int i=0; i<signs.size(); ++i)
    {
        if (signs.at(i) != signs.at((i+1)%signs.size())) ++sign_switch;
    }

    if (sign_switch > 2) return true;
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractPolygonMesh<M,V,E,P>::vert_is_critical_p(const unsigned int vid, const int tex_coord) const
{
    return (this->vert_is_local_max(vid,tex_coord) ||
            this->vert_is_local_min(vid,tex_coord) ||
            this->vert_is_saddle   (vid,tex_coord));
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractPolygonMesh<M,V,E,P>::poly_verts_are_CCW(const unsigned int pid, const unsigned int curr, const unsigned int prev) const
{
    unsigned int prev_offset = this->poly_vert_offset(pid, prev);
    unsigned int curr_offset = this->poly_vert_offset(pid, curr);
    if(curr_offset == (prev_offset+1)%this->verts_per_poly(pid)) return true;
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractPolygonMesh<M,V,E,P>::edge_is_CCW(const unsigned int eid, const unsigned int pid) const
{
    unsigned int vid0 = this->edge_vert_id(eid,0);
    unsigned int vid1 = this->edge_vert_id(eid,1);
    return this->poly_verts_are_CCW(pid, vid1, vid0);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::vert_ordered_one_ring(const unsigned int vid,
                                                         std::vector<unsigned int> & v_link,       // sorted list of adjacent vertices
                                                         std::vector<unsigned int> & f_star,       // sorted list of adjacent triangles
                                                         std::vector<unsigned int> & e_star,       // sorted list of edges incident to vid
                                                         std::vector<unsigned int> & e_link) const // sorted list of edges opposite to vid
{
    // see https://en.wikipedia.org/wiki/Simplicial_complex#Closure,_star,_and_link for adefinition of link and star

    v_link.clear();
    f_star.clear();
    e_star.clear();
    e_link.clear();

    if (this->adj_v2e(vid).empty()) return;
    unsigned int curr_e  = this->adj_v2e(vid).front(); assert(edge_is_manifold(curr_e));
    unsigned int curr_v  = this->vert_opposite_to(curr_e, vid);
    unsigned int curr_p  = this->adj_e2p(curr_e).front();
    // impose CCW winding...
    if (!this->poly_verts_are_CCW(curr_p, curr_v, vid)) curr_p = this->adj_e2p(curr_e).back();

    // If there are boundary edges it is important to start from the right triangle (i.e. right-most),
    // otherwise it will be impossible to cover the entire umbrella
    std::vector<unsigned int> b_edges = vert_boundary_edges(vid);
    if (b_edges.size()  > 0)
    {
        assert(b_edges.size() == 2); // otherwise there is no way to cover the whole umbrella walking through adjacent triangles!!!

        unsigned int e = b_edges.front();
        unsigned int p = this->adj_e2p(e).front();
        unsigned int v = this->vert_opposite_to(e, vid);

        if (!this->poly_verts_are_CCW(p, v, vid))
        {
            e = b_edges.back();
            p = this->adj_e2p(e).front();
            v = this->vert_opposite_to(e, vid);
            assert(this->poly_verts_are_CCW(p, v, vid));
        }

        curr_e = e;
        curr_p = p;
        curr_v = v;
    }

    do
    {
        e_star.push_back(curr_e);
        f_star.push_back(curr_p);

        unsigned int off = this->poly_vert_offset(curr_p, curr_v);
        for(unsigned int i=0; i<this->verts_per_poly(curr_p)-1; ++i)
        {
            curr_v = this->poly_vert_id(curr_p,(off+i)%this->verts_per_poly(curr_p));
            if (i>0) e_link.push_back( this->poly_edge_id(curr_p, curr_v, v_link.back()) );
            v_link.push_back(curr_v);
        }

        curr_e = this->poly_edge_id(curr_p, vid, v_link.back()); assert(edge_is_manifold(curr_e));
        curr_p = (this->adj_e2p(curr_e).front() == curr_p) ? this->adj_e2p(curr_e).back() : this->adj_e2p(curr_e).front();

        if(edge_is_boundary(curr_e)) e_star.push_back(curr_e);
        else v_link.pop_back();
    }
    while(e_star.size() < this->adj_v2e(vid).size());
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractPolygonMesh<M,V,E,P>::vert_verts_link(const unsigned int vid) const
{
    return this->adj_v2v(vid);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractPolygonMesh<M,V,E,P>::vert_edges_link(const unsigned int vid) const
{
    std::unordered_set<unsigned int> e_link;
    for(unsigned int pid : this->adj_v2p(vid))
    for(unsigned int eid : this->adj_p2e(pid))
    {
        if(!this->edge_contains_vert(eid,vid)) e_link.insert(eid);
    }
    return std::vector<unsigned int>(e_link.begin(),e_link.end());
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractPolygonMesh<M,V,E,P>::vert_ordered_verts_link(const unsigned int vid) const
{
    std::vector<unsigned int> v_ring; // sorted list of adjacent vertices
    std::vector<unsigned int> f_ring; // sorted list of adjacent triangles
    std::vector<unsigned int> e_ring; // sorted list of edges incident to vid
    std::vector<unsigned int> e_link; // sorted list of edges opposite to vid
    vert_ordered_one_ring(vid, v_ring, f_ring, e_ring, e_link);
    return v_ring;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractPolygonMesh<M,V,E,P>::vert_ordered_polys_star(const unsigned int vid) const
{
    std::vector<unsigned int> v_ring; // sorted list of adjacent vertices
    std::vector<unsigned int> f_ring; // sorted list of adjacent triangles
    std::vector<unsigned int> e_ring; // sorted list of edges incident to vid
    std::vector<unsigned int> e_link; // sorted list of edges opposite to vid
    vert_ordered_one_ring(vid, v_ring, f_ring, e_ring, e_link);
    return f_ring;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractPolygonMesh<M,V,E,P>::vert_ordered_edges_star(const unsigned int vid) const
{
    std::vector<unsigned int> v_ring; // sorted list of adjacent vertices
    std::vector<unsigned int> f_ring; // sorted list of adjacent triangles
    std::vector<unsigned int> e_ring; // sorted list of edges incident to vid
    std::vector<unsigned int> e_link; // sorted list of edges opposite to vid
    vert_ordered_one_ring(vid, v_ring, f_ring, e_ring, e_link);
    return e_ring;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractPolygonMesh<M,V,E,P>::vert_ordered_edges_link(const unsigned int vid) const
{
    std::vector<unsigned int> v_ring; // sorted list of adjacent vertices
    std::vector<unsigned int> f_ring; // sorted list of adjacent triangles
    std::vector<unsigned int> e_ring; // sorted list of edges incident to vid
    std::vector<unsigned int> e_link; // sorted list of edges opposite to vid
    vert_ordered_one_ring(vid, v_ring, f_ring, e_ring, e_link);
    return e_link;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractPolygonMesh<M,V,E,P>::vert_area(const unsigned int vid) const
{
    double area = 0.0;
    for(unsigned int pid : this->adj_v2p(vid)) area += poly_area(pid)/static_cast<double>(this->verts_per_poly(pid));
    return area;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractPolygonMesh<M,V,E,P>::vert_mass(const unsigned int vid) const
{
    return vert_area(vid);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractPolygonMesh<M,V,E,P>::vert_is_boundary(const unsigned int vid) const
{
    for(unsigned int eid : this->adj_v2e(vid)) if (edge_is_boundary(eid)) return true;
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractPolygonMesh<M,V,E,P>::vert_is_manifold(const unsigned int vid) const
{
    for(unsigned int eid : this->adj_v2e(vid))
    {
        if(!this->edge_is_manifold(eid)) return false;
    }

    std::vector<unsigned int> e_link = this->vert_edges_link(vid);
    std::unordered_set<unsigned int> edge_set(e_link.begin(), e_link.end());

    std::queue<unsigned int> q;
    q.push(e_link.front());

    std::unordered_set<unsigned int> visited;
    visited.insert(e_link.front());

    while(!q.empty())
    {
        unsigned int curr = q.front();
        q.pop();

        assert(CONTAINS(visited, curr));

        for(unsigned int nbr : this->adj_e2e(curr))
        {
            // still in the link of vid, but not visited yet
            if(CONTAINS(edge_set, nbr) && !CONTAINS(visited, nbr))
            {
                visited.insert(nbr);
                q.push(nbr);
            }
        }
    }

    return (visited.size() == e_link.size());
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractPolygonMesh<M,V,E,P>::vert_boundary_edges(const unsigned int vid) const
{
    std::vector<unsigned int> b_edges;
    for(unsigned int eid : this->adj_v2e(vid)) if (edge_is_boundary(eid)) b_edges.push_back(eid);
    return b_edges;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractPolygonMesh<M,V,E,P>::vert_is_visible(const unsigned int vid) const
{
    for(unsigned int pid : this->adj_v2p(vid))
    {
        if(!this->poly_data(pid).flags[HIDDEN]) return true;
    }
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::vert_cluster_one_ring(const unsigned int                       vid,
                                                         std::vector<std::vector<unsigned int>> & clusters,
                                                         const bool                       marked_edges_are_borders)
{
    /* separate the polys incident to vid in clusters. Clusters are naturally separated
     * by boundary edges, but also marked edges can be optionally considered as barriers
     */
    clusters.clear();
    std::unordered_set<unsigned int> visited;

    for(unsigned int pid : this->adj_v2p(vid))
    {
        if(CONTAINS(visited, pid)) continue;
        visited.insert(pid);

        std::vector<unsigned int> c;
        c.push_back(pid);

        std::queue<unsigned int> q;
        q.push(pid);

        while(!q.empty())
        {
            unsigned int pid = q.front();
            q.pop();

            for(unsigned int nbr : this->adj_p2p(pid))
            {
                if(!this->poly_contains_vert(nbr,vid)) continue; // not in the umbrella
                if(CONTAINS(visited,nbr)) continue; // already visited

                unsigned int eid   = this->edge_shared(pid,nbr);
                bool flood = true;
                if(marked_edges_are_borders && this->edge_data(eid).flags[MARKED]) flood = false;

                if(flood)
                {
                    visited.insert(nbr);
                    q.push(nbr);
                    c.push_back(nbr);
                }
            }
        }
        clusters.push_back(c);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractPolygonMesh<M,V,E,P>::vert_adj_visible_polys(const unsigned int vid, const vec3d dir, const double ang_thresh)
{
    std::vector<unsigned int> nbrs;
    for(unsigned int pid : this->adj_v2p(vid))
    {
        if(!this->poly_data(pid).flags[HIDDEN])
        {
            vec3d n = this->poly_data(pid).normal;
            if(dir.angle_deg(n) < ang_thresh) nbrs.push_back(pid);
        }
    }
    return nbrs;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractPolygonMesh<M,V,E,P>::vert_add(const vec3d & pos)
{
    unsigned int vid = this->num_verts();
    //
    this->verts.push_back(pos);
    //
    V data;
    this->v_data.push_back(data);
    //
    this->v2v.push_back(std::vector<unsigned int>());
    this->v2e.push_back(std::vector<unsigned int>());
    this->v2p.push_back(std::vector<unsigned int>());
    //
    if(this->mesh_data().update_bbox)
    {
        this->bb.min = this->bb.min.min(pos);
        this->bb.max = this->bb.max.max(pos);
    }
    //
    return vid;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractPolygonMesh<M,V,E,P>::vert_merge(const unsigned int vid0, const unsigned int vid1)
{
    std::vector<unsigned int> old_polys = this->adj_v2p(vid1);
    std::vector<std::vector<unsigned int>> new_polys;
    for(unsigned int pid : old_polys)
    {
        // if there is a polygon containing both vid0 and vid1 abort the operation
        if(this->poly_contains_vert(pid,vid0)) return false;

        std::vector<unsigned int> p = this->poly_verts_id(pid);
        std::replace(p.begin(), p.end(), vid1, vid0);
        new_polys.push_back(p);
    }

    for(auto p : new_polys) poly_add(p);
    vert_remove(vid1);

    return true;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::vert_switch_id(const unsigned int vid0, const unsigned int vid1)
{
    // [28 Aug 2017] Tested on 10K random id switches : PASSED

    if (vid0 == vid1) return;

    std::swap(this->verts.at(vid0),  this->verts.at(vid1));
    std::swap(this->v_data.at(vid0), this->v_data.at(vid1));
    std::swap(this->v2v.at(vid0),    this->v2v.at(vid1));
    std::swap(this->v2e.at(vid0),    this->v2e.at(vid1));
    std::swap(this->v2p.at(vid0),    this->v2p.at(vid1));

    std::unordered_set<unsigned int> verts_to_update;
    verts_to_update.insert(this->adj_v2v(vid0).begin(), this->adj_v2v(vid0).end());
    verts_to_update.insert(this->adj_v2v(vid1).begin(), this->adj_v2v(vid1).end());

    std::unordered_set<unsigned int> edges_to_update;
    edges_to_update.insert(this->adj_v2e(vid0).begin(), this->adj_v2e(vid0).end());
    edges_to_update.insert(this->adj_v2e(vid1).begin(), this->adj_v2e(vid1).end());

    std::unordered_set<unsigned int> polys_to_update;
    polys_to_update.insert(this->adj_v2p(vid0).begin(), this->adj_v2p(vid0).end());
    polys_to_update.insert(this->adj_v2p(vid1).begin(), this->adj_v2p(vid1).end());

    for(unsigned int nbr : verts_to_update)
    {
        for(unsigned int & vid : this->v2v.at(nbr))
        {
            if (vid == vid0) vid = vid1; else
            if (vid == vid1) vid = vid0;
        }
    }

    for(unsigned int eid : edges_to_update)
    {
        for(unsigned int i=0; i<2; ++i)
        {
            unsigned int & vid = this->edges.at(2*eid+i);
            if (vid == vid0) vid = vid1; else
            if (vid == vid1) vid = vid0;
        }
    }

    for(unsigned int pid : polys_to_update)
    {
        for(unsigned int & vid : this->polys.at(pid))
        {
            if (vid == vid0) vid = vid1; else
            if (vid == vid1) vid = vid0;
        }
        for(unsigned int & vid : this->poly_triangles.at(pid))
        {
            if (vid == vid0) vid = vid1; else
            if (vid == vid1) vid = vid0;
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::vert_remove(const unsigned int vid)
{
    polys_remove(this->adj_v2p(vid));
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::vert_remove_unreferenced(const unsigned int vid)
{
    this->v2v.at(vid).clear();
    this->v2e.at(vid).clear();
    this->v2p.at(vid).clear();
    vert_switch_id(vid, this->num_verts()-1);
    this->verts.pop_back();
    this->v_data.pop_back();
    this->v2v.pop_back();
    this->v2e.pop_back();
    this->v2p.pop_back();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractPolygonMesh<M,V,E,P>::edge_is_manifold(const unsigned int eid) const
{
    unsigned int val = this->edge_valence(eid);
    return (val>0 && val<3);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractPolygonMesh<M,V,E,P>::edge_dihedral_angle(const unsigned int eid) const
{
    if(!this->edge_is_manifold(eid)) return 0;
    if( this->edge_is_boundary(eid)) return 0;

    unsigned int   pid0 = this->adj_e2p(eid).front();
    unsigned int   pid1 = this->adj_e2p(eid).back();
    vec3d  n0   = this->poly_data(pid0).normal;
    vec3d  n1   = this->poly_data(pid1).normal;

    return n0.angle_rad(n1);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractPolygonMesh<M,V,E,P>::edge_is_boundary(const unsigned int eid) const
{
    return (this->edge_valence(eid) == 1);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractPolygonMesh<M,V,E,P>::edge_is_incident_to_boundary(const unsigned int eid) const
{
    for(unsigned int vid : this->adj_e2v(eid)) if(this->vert_is_boundary(vid)) return true;
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractPolygonMesh<M,V,E,P>::edges_share_poly(const unsigned int eid1, const unsigned int eid2) const
{
    for(unsigned int pid1 : this->adj_e2p(eid1))
    for(unsigned int pid2 : this->adj_e2p(eid2))
    {
        if (pid1 == pid2) return true;
    }
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractPolygonMesh<M,V,E,P>::edge_shared(const unsigned int pid0, const unsigned int pid1) const
{
    std::vector<unsigned int> shared_edges;
    for(unsigned int eid : this->adj_p2e(pid0))
    {
        if (this->poly_contains_edge(pid1,eid))
        {
            shared_edges.push_back(eid);
        }
    }
    assert(shared_edges.size()==1); // this limits the class of polygon meshes that I can handle...
                                    // I might decide to change this!!!!
    return shared_edges.front();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractPolygonMesh<M,V,E,P>::edge_add(const unsigned int vid0, const unsigned int vid1)
{
    assert(this->edge_id(vid0, vid1)==-1); // make sure it doesn't exist already
    assert(vid0 < this->num_verts());
    assert(vid1 < this->num_verts());
    assert(!this->verts_are_adjacent(vid0, vid1));
    assert(DOES_NOT_CONTAIN_VEC(this->v2v.at(vid0), vid1));
    assert(DOES_NOT_CONTAIN_VEC(this->v2v.at(vid1), vid0));
    //
    unsigned int eid = this->num_edges();
    //
    this->edges.push_back(vid0);
    this->edges.push_back(vid1);
    //
    this->e2p.push_back(std::vector<unsigned int>());
    //
    E data;
    this->e_data.push_back(data);
    //
    this->v2v.at(vid1).push_back(vid0);
    this->v2v.at(vid0).push_back(vid1);
    //
    this->v2e.at(vid0).push_back(eid);
    this->v2e.at(vid1).push_back(eid);
    //
    return eid;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::edge_switch_id(const unsigned int eid0, const unsigned int eid1)
{
    // [28 Aug 2017] Tested on 10K random id switches : PASSED

    if (eid0 == eid1) return;

    for(unsigned int off=0; off<2; ++off) std::swap(this->edges.at(2*eid0+off), this->edges.at(2*eid1+off));

    std::swap(this->e2p.at(eid0),    this->e2p.at(eid1));
    std::swap(this->e_data.at(eid0), this->e_data.at(eid1));

    std::unordered_set<unsigned int> verts_to_update;
    verts_to_update.insert(this->edge_vert_id(eid0,0));
    verts_to_update.insert(this->edge_vert_id(eid0,1));
    verts_to_update.insert(this->edge_vert_id(eid1,0));
    verts_to_update.insert(this->edge_vert_id(eid1,1));

    std::unordered_set<unsigned int> polys_to_update;
    polys_to_update.insert(this->adj_e2p(eid0).begin(), this->adj_e2p(eid0).end());
    polys_to_update.insert(this->adj_e2p(eid1).begin(), this->adj_e2p(eid1).end());

    for(unsigned int vid : verts_to_update)
    {
        for(unsigned int & eid : this->v2e.at(vid))
        {
            if (eid == eid0) eid = eid1; else
            if (eid == eid1) eid = eid0;
        }
    }

    for(unsigned int pid : polys_to_update)
    {
        for(unsigned int & eid : this->p2e.at(pid))
        {
            if (eid == eid0) eid = eid1; else
            if (eid == eid1) eid = eid0;
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::edge_remove(const unsigned int eid)
{
    polys_remove(this->adj_e2p(eid));
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::edge_remove_unreferenced(const unsigned int eid)
{
    this->e2p.at(eid).clear();
    edge_switch_id(eid, this->num_edges()-1);
    this->edges.resize(this->edges.size()-2);
    this->e_data.pop_back();
    this->e2p.pop_back();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::edge_mark_labeling_boundaries()
{
    for(unsigned int eid=0; eid<this->num_edges(); ++eid)
    {
        std::unordered_set<int> unique_labels;
        for(unsigned int pid : this->adj_e2p(eid)) unique_labels.insert(this->poly_data(pid).label);
        this->edge_data(eid).flags[MARKED] = (unique_labels.size()>=2);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::edge_mark_color_discontinuities()
{
    for(unsigned int eid=0; eid<this->num_edges(); ++eid)
    {
        std::set<Color> unique_colors;
        for(unsigned int pid : this->adj_e2p(eid)) unique_colors.insert(this->poly_data(pid).color);

        this->edge_data(eid).flags[MARKED] = (unique_colors.size()>=2);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::edge_mark_boundaries()
{
    for(unsigned int eid=0; eid<this->num_edges(); ++eid)
    {
        this->edge_data(eid).flags[MARKED] = edge_is_boundary(eid);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractPolygonMesh<M,V,E,P>::poly_vert_offset(const unsigned int pid, const unsigned int vid) const
{
    for(unsigned int offset=0; offset<verts_per_poly(pid); ++offset)
    {
        if (this->poly_vert_id(pid,offset) == vid) return offset;
    }
    assert(false && "Something is off here...");
    return 0; // warning killer
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractPolygonMesh<M,V,E,P>::poly_angle_at_vert(const unsigned int pid, const unsigned int vid, const int unit) const
{
    assert(this->poly_contains_vert(pid,vid));

    unsigned int   curr   = this->poly_vert_offset(pid, vid);
    unsigned int   nv     = this->verts_per_poly(pid);
    unsigned int   next   = (curr+1   )%nv;
    unsigned int   prev   = (curr-1+nv)%nv;
    vec3d  p      = this->poly_vert(pid, curr);
    vec3d  u      = this->poly_vert(pid, prev) - p;
    vec3d  v      = this->poly_vert(pid, next) - p;
    double angle  = u.angle_rad(v);

    if((-u).cross(v).dot(this->poly_data(pid).normal)<0)
    {
        angle = 2*M_PI - angle;
    }

    switch(unit)
    {
        case RAD : return angle;
        case DEG : return to_deg(angle);
        default  : assert(false); return 0; // warning killer
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
int AbstractPolygonMesh<M,V,E,P>::poly_shared(const unsigned int eid0, const unsigned int eid1) const
{
    for(unsigned int pid0 : this->adj_e2p(eid0))
    for(unsigned int pid1 : this->adj_e2p(eid1))
    {
        if (pid0 == pid1) return pid0;
    }
    return -1;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
int AbstractPolygonMesh<M,V,E,P>::poly_id(const std::vector<unsigned int> & vlist) const
{
    assert(!vlist.empty());
    std::vector<unsigned int> query = SORT_VEC(vlist);

    unsigned int vid = vlist.front();
    for(unsigned int pid : this->adj_v2p(vid))
    {
        if(this->poly_verts_id(pid,true)==query) return pid;
    }
    return -1;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractPolygonMesh<M,V,E,P>::polys_adjacent_along(const unsigned int pid, const unsigned int eid) const
{
    std::vector<unsigned int> polys;
    for(unsigned int nbr : this->adj_p2p(pid))
    {
        if (this->poly_contains_edge(nbr,eid)) polys.push_back(nbr);
    }
    return polys;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<vec3d> AbstractPolygonMesh<M,V,E,P>::poly_vlist(const unsigned int pid) const
{
    std::vector<vec3d> vlist(this->verts_per_poly(pid));
    for (unsigned int i=0; i<this->verts_per_poly(pid); ++i)
    {
        vlist.at(i) = this->poly_vert(pid,i);
    }
    return vlist;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
const std::vector<unsigned int> & AbstractPolygonMesh<M,V,E,P>::poly_tessellation(const unsigned int pid) const
{
    return poly_triangles.at(pid);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractPolygonMesh<M,V,E,P>::polys_adjacent_along(const unsigned int pid, const unsigned int vid0, const unsigned int vid1) const
{
    unsigned int eid = this->poly_edge_id(pid, vid0, vid1);
    return polys_adjacent_along(pid, eid);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
int AbstractPolygonMesh<M,V,E,P>::poly_opposite_to(const unsigned int eid, const unsigned int pid) const
{
    assert(this->poly_contains_edge(pid,eid));
    assert(this->edge_is_manifold(eid));
    assert(!this->adj_e2p(eid).empty());

    if (this->edge_is_boundary(eid)) return -1;
    if (this->adj_e2p(eid).front() != pid) return this->adj_e2p(eid).front();
    return this->adj_e2p(eid).back();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractPolygonMesh<M,V,E,P>::polys_are_adjacent(const unsigned int pid0, const unsigned int pid1) const
{
    for(unsigned int eid : this->adj_p2e(pid0))
    for(unsigned int pid : this->polys_adjacent_along(pid0, eid))
    {
        if (pid == pid1) return true;
    }
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
bool AbstractPolygonMesh<M,V,E,P>::poly_is_boundary(const unsigned int pid) const
{
    return (this->adj_p2p(pid).size() < 3);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::poly_switch_id(const unsigned int pid0, const unsigned int pid1)
{
    // [28 Aug 2017] Tested on 10K random id switches : PASSED

    if (pid0 == pid1) return;

    std::swap(this->polys.at(pid0),          this->polys.at(pid1));
    std::swap(this->p_data.at(pid0),         this->p_data.at(pid1));
    std::swap(this->p2e.at(pid0),            this->p2e.at(pid1));
    std::swap(this->p2p.at(pid0),            this->p2p.at(pid1));
    std::swap(this->poly_triangles.at(pid0), this->poly_triangles.at(pid1));

    std::unordered_set<unsigned int> verts_to_update;
    verts_to_update.insert(this->adj_p2v(pid0).begin(), this->adj_p2v(pid0).end());
    verts_to_update.insert(this->adj_p2v(pid1).begin(), this->adj_p2v(pid1).end());

    std::unordered_set<unsigned int> edges_to_update;
    edges_to_update.insert(this->adj_p2e(pid0).begin(), this->adj_p2e(pid0).end());
    edges_to_update.insert(this->adj_p2e(pid1).begin(), this->adj_p2e(pid1).end());

    std::unordered_set<unsigned int> polys_to_update;
    polys_to_update.insert(this->adj_p2p(pid0).begin(), this->adj_p2p(pid0).end());
    polys_to_update.insert(this->adj_p2p(pid1).begin(), this->adj_p2p(pid1).end());

    for(unsigned int vid : verts_to_update)
    {
        for(unsigned int & pid : this->v2p.at(vid))
        {
            if (pid == pid0) pid = pid1; else
            if (pid == pid1) pid = pid0;
        }
    }

    for(unsigned int eid : edges_to_update)
    {
        for(unsigned int & pid : this->e2p.at(eid))
        {
            if (pid == pid0) pid = pid1; else
            if (pid == pid1) pid = pid0;
        }
    }

    for(unsigned int nbr : polys_to_update)
    {
        for(unsigned int & pid : this->p2p.at(nbr))
        {
            if (pid == pid0) pid = pid1; else
            if (pid == pid1) pid = pid0;
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
unsigned int AbstractPolygonMesh<M,V,E,P>::poly_add(const std::vector<unsigned int> & vlist)
{
    if(poly_id(vlist)!=-1)
    {
        std::cout << ANSI_fg_color_red << "WARNING: adding duplicated poly!" << ANSI_fg_color_default << std::endl;
        return poly_id(vlist);
    }
#ifndef NDEBUG
    for(unsigned int vid : vlist) assert(vid < this->num_verts());
#endif

    unsigned int pid = this->num_polys();
    this->polys.push_back(vlist);

    P data;
    this->p_data.push_back(data);

    this->p2e.push_back(std::vector<unsigned int>());
    this->p2p.push_back(std::vector<unsigned int>());

    // add missing edges
    for(unsigned int i=0; i<vlist.size(); ++i)
    {
        unsigned int vid0 = vlist.at(i);
        unsigned int vid1 = vlist.at((i+1)%vlist.size());
        int  eid = this->edge_id(vid0, vid1);

        if (eid == -1) eid = this->edge_add(vid0, vid1);
    }

    // update connectivity
    for(unsigned int vid : vlist)
    {
        this->v2p.at(vid).push_back(pid);
    }
    //
    for(unsigned int i=0; i<vlist.size(); ++i)
    {
        unsigned int vid0 = vlist.at(i);
        unsigned int vid1 = vlist.at((i+1)%vlist.size());
        int  eid = this->edge_id(vid0, vid1);
        assert(eid >= 0);

        for(unsigned int nbr : this->e2p.at(eid))
        {
            assert(nbr!=pid);
            if (this->polys_are_adjacent(pid,nbr)) continue;
            this->p2p.at(nbr).push_back(pid);
            this->p2p.at(pid).push_back(nbr);
        }

        this->e2p.at(eid).push_back(pid);
        this->p2e.at(pid).push_back(eid);
    }

    if(this->mesh_data().update_normals) this->update_p_normal(pid);
    this->poly_triangles.push_back(std::vector<unsigned int>());
    update_p_tessellation(pid);

    return pid;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::polys_remove(const std::vector<unsigned int> & pids)
{
    // in order to avoid id conflicts remove all the
    // polys starting from the one with highest id
    //
    std::vector<unsigned int> tmp = pids;
    std::sort(tmp.begin(), tmp.end());
    std::reverse(tmp.begin(), tmp.end());
    for(unsigned int pid : tmp) poly_remove(pid);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::poly_remove(const unsigned int pid)
{
    // [28 Aug 2017] Tested on progressive random removal until almost no polys are left: PASSED

    std::set<unsigned int,std::greater<unsigned int>> dangling_verts; // higher ids first
    std::set<unsigned int,std::greater<unsigned int>> dangling_edges; // higher ids first

    // disconnect from vertices
    for(unsigned int vid : this->adj_p2v(pid))
    {
        REMOVE_FROM_VEC(this->v2p.at(vid), pid);
        if (this->v2p.at(vid).empty()) dangling_verts.insert(vid);
    }

    // disconnect from edges
    for(unsigned int eid : this->adj_p2e(pid))
    {
        REMOVE_FROM_VEC(this->e2p.at(eid), pid);
        if (this->e2p.at(eid).empty()) dangling_edges.insert(eid);
    }

    // disconnect from other polygons
    for(unsigned int nbr : this->adj_p2p(pid)) REMOVE_FROM_VEC(this->p2p.at(nbr), pid);

    // delete dangling edges
    for(unsigned int eid : dangling_edges)
    {
        unsigned int vid0 = this->edge_vert_id(eid,0);
        unsigned int vid1 = this->edge_vert_id(eid,1);
        if (vid1 > vid0) std::swap(vid0,vid1); // make sure the highest id is processed first
        REMOVE_FROM_VEC(this->v2e.at(vid0), eid);
        REMOVE_FROM_VEC(this->v2e.at(vid1), eid);
        REMOVE_FROM_VEC(this->v2v.at(vid0), vid1);
        REMOVE_FROM_VEC(this->v2v.at(vid1), vid0);
        edge_remove_unreferenced(eid);
    }

    // delete dangling vertices
    for(unsigned int vid : dangling_verts)
    {
        assert(this->adj_v2e(vid).empty());
        for(unsigned int nbr : this->adj_v2v(vid)) REMOVE_FROM_VEC(this->v2v.at(nbr), vid);
        vert_remove_unreferenced(vid);
    }

    poly_remove_unreferenced(pid);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::poly_remove_unreferenced(const unsigned int pid)
{
    this->polys.at(pid).clear();
    this->p2e.at(pid).clear();
    this->p2p.at(pid).clear();
    poly_switch_id(pid, this->num_polys()-1);
    this->polys.pop_back();
    this->p_data.pop_back();
    this->p2e.pop_back();
    this->p2p.pop_back();
    this->poly_triangles.pop_back();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<ipair> AbstractPolygonMesh<M,V,E,P>::get_boundary_edges() const
{
    std::vector<ipair> res;
    for(unsigned int eid=0; eid<this->num_edges(); ++eid)
    {
        if (this->edge_is_boundary(eid))
        {
            ipair e;
            e.first  = this->edge_vert_id(eid,0);
            e.second = this->edge_vert_id(eid,1);
            res.push_back(e);
        }
    }
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractPolygonMesh<M,V,E,P>::get_boundary_vertices() const
{
    std::vector<unsigned int> res;
    for(unsigned int vid=0; vid<this->num_verts(); ++vid) if (this->vert_is_boundary(vid)) res.push_back(vid);
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractPolygonMesh<M,V,E,P>::poly_mass(const unsigned int pid) const
{
    return this->poly_area(pid);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractPolygonMesh<M,V,E,P>::poly_area(const unsigned int pid) const
{
    double area = 0.0;
    std::vector<unsigned int> tris = poly_tessellation(pid);
    for(unsigned int i=0; i<tris.size()/3; ++i)
    {
        area += triangle_area(this->vert(tris.at(3*i+0)),
                              this->vert(tris.at(3*i+1)),
                              this->vert(tris.at(3*i+2)));
    }
    return area;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
double AbstractPolygonMesh<M,V,E,P>::poly_perimeter(const unsigned int pid) const
{
    double perimeter = 0.0;
    for(unsigned int eid : this->adj_p2e(pid)) perimeter += this->edge_length(eid);
    return perimeter;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::poly_flip_winding_order(const unsigned int pid)
{
    std::reverse(this->polys.at(pid).begin(), this->polys.at(pid).end());

    if(this->mesh_data().update_normals)
    {
        update_p_normal(pid);
        for(unsigned int off=0; off<this->verts_per_poly(pid); ++off) update_v_normal(this->poly_vert_id(pid,off));
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::operator+=(const AbstractPolygonMesh<M,V,E,P> & m)
{
    unsigned int nv = this->num_verts();
    unsigned int ne = this->num_edges();
    unsigned int np = this->num_polys();

    std::vector<unsigned int> tmp;
    for(unsigned int pid=0; pid<m.num_polys(); ++pid)
    {
        std::vector<unsigned int> p;
        for(unsigned int vid : m.adj_p2v(pid)) p.push_back(nv + vid);
        this->polys.push_back(p);

        this->p_data.push_back(m.poly_data(pid));

        tmp.clear();
        for(unsigned int eid : m.p2e.at(pid)) tmp.push_back(ne + eid);
        this->p2e.push_back(tmp);

        tmp.clear();
        for(unsigned int nbr : m.p2p.at(pid)) tmp.push_back(np + nbr);
        this->p2p.push_back(tmp);

        tmp.clear();
        for(unsigned int vid : m.poly_tessellation(pid)) tmp.push_back(nv + vid);
        this->poly_triangles.push_back(tmp);
    }
    for(unsigned int eid=0; eid<m.num_edges(); ++eid)
    {
        this->edges.push_back(nv + m.edge_vert_id(eid,0));
        this->edges.push_back(nv + m.edge_vert_id(eid,1));

        this->e_data.push_back(m.edge_data(eid));

        tmp.clear();
        for(unsigned int tid : m.e2p.at(eid)) tmp.push_back(np + tid);
        this->e2p.push_back(tmp);
    }
    for(unsigned int vid=0; vid<m.num_verts(); ++vid)
    {
        this->verts.push_back(m.vert(vid));
        this->v_data.push_back(m.vert_data(vid));

        tmp.clear();
        for(unsigned int eid : m.v2e.at(vid)) tmp.push_back(ne + eid);
        this->v2e.push_back(tmp);

        tmp.clear();
        for(unsigned int tid : m.v2p.at(vid)) tmp.push_back(np + tid);
        this->v2p.push_back(tmp);

        tmp.clear();
        for(unsigned int nbr : m.v2v.at(vid)) tmp.push_back(nv + nbr);
        this->v2v.push_back(tmp);
    }

    if(this->mesh_data().update_bbox) this->update_bbox();

    std::cout << "Appended " << m.mesh_data().filename << " to mesh " << this->mesh_data().filename << std::endl;
    std::cout << this->num_verts() << " verts" << std::endl;
    std::cout << this->num_edges() << " edges" << std::endl;
    std::cout << this->num_polys() << " polys" << std::endl;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
std::vector<unsigned int> AbstractPolygonMesh<M,V,E,P>::get_ordered_boundary_vertices() const
{
    // WARNING: assumes the mesh contains exactly ONE simply connected boundary!

    std::vector<unsigned int> b_verts;

    unsigned int seed = 0;
    while(seed<this->num_edges() && !this->edge_is_boundary(seed)) ++seed;
    if(seed >= this->num_edges()) return b_verts;

    b_verts.push_back(this->edge_vert_id(seed,0));
    b_verts.push_back(this->edge_vert_id(seed,1));

    unsigned int prev = b_verts.front();
    unsigned int curr = b_verts.back();

    do
    {
        bool found_next = false;
        for(unsigned int eid : this->adj_v2e(curr))
        {
            if(!this->edge_is_boundary(eid)) continue;
            unsigned int vid = this->vert_opposite_to(eid, curr);
            if(vid!=prev && this->vert_is_boundary(vid))
            {
                b_verts.push_back(vid);
                prev = curr;
                curr = vid;
                found_next = true;
                break;
            }
        }
        assert(found_next);
        assert(b_verts.size()<=this->num_verts()+1);
    }
    while(b_verts.front()!=b_verts.back());
    b_verts.pop_back();
    return b_verts;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class P>
CINO_INLINE
void AbstractPolygonMesh<M,V,E,P>::poly_export_element(const unsigned int pid, std::vector<vec3d> & verts, std::vector<std::vector<unsigned int>> & faces) const
{
    std::vector<unsigned int> f(this->verts_per_poly(pid));
    std::iota(f.begin(), f.end(), static_cast<unsigned int>(verts.size()));
    for(unsigned int vid : adj_p2v(pid)) verts.push_back(this->vert(vid));
    faces.push_back(f);
}

}
