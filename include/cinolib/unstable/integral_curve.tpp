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
#include "integral_curve.h"
#include <cinolib/cino_inline.h>
#include <cinolib/ipair.h>
#include <cinolib/geometry/triangle_utils.h>
#include <cinolib/gl/draw_cylinder.h>
#include <cinolib/gl/draw_arrow.h>
#include <cinolib/gl/draw_sphere.h>
#include <cinolib/geometry/triangle_utils.h>
#include <cinolib/geometry/tetrahedron_utils.h>
#include <cinolib/meshes/drawable_trimesh.h>
#include <cinolib/meshes/tetmesh.h>
#include <cinolib/geometry/triangle.h>
#include <cinolib/geometry/segment.h>
#include <cinolib/geometry/ray.h>
#include <cinolib/geometry/tetrahedron.h>
#include <cinolib/ray_triangle_intersection.h>
#include <map>
#include <cinolib/standard_elements_tables.h>
#include <cstdlib> // for abort()

namespace cinolib
{

template<class Mesh>
CINO_INLINE
IntegralCurve<Mesh>::IntegralCurve(const Mesh                & m,
                                   const VectorField         & grad,
                                   const unsigned int                  pid,
                                   const std::vector<double> & bary)
    : DrawableCurve()
    , status(COMPUTING)
    , m_ptr(&m)
    , grad_ptr(&grad)
{
    Sample seed;
    seed.pid  = pid;
    seed.bary = bary;
    for(unsigned int off=0; off<m.verts_per_poly(); ++off) seed.pos += bary.at(off) * m.poly_vert(pid, off);

    trace_curve(seed);
}

template<class Mesh>
CINO_INLINE
IntegralCurve<Mesh>::IntegralCurve(const Mesh        & m,
                                   const VectorField & grad,
                                   const unsigned int          vid)
    : DrawableCurve()
    , status(COMPUTING)
    , m_ptr(&m)
    , grad_ptr(&grad)
{
    assert(!m.adj_v2p(vid).empty());
    unsigned int pid = m.adj_v2p(vid).front();

    unsigned int off = 0;
    while(off<m.verts_per_poly() && m.poly_vert_id(pid,off)!=vid) ++off;
    assert(m.poly_vert_id(pid,off)==vid && "Cannot find incident element to begin with");

    std::vector<double> bary = std::vector<double>(m.verts_per_poly(),0);
    bary[off] = 1.0;

    Sample seed;
    seed.pos  = m.vert(vid);
    seed.pid  = pid;
    seed.bary = bary;

    trace_curve(seed);
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class Mesh>
CINO_INLINE
void IntegralCurve<Mesh>::trace_curve(const Sample & seed)
{
    append_sample(seed);

    do
    {
        Sample s = move_forward(samples().back());
        append_sample(s);
    }
    while (!is_converged(samples().back()));
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class Mesh>
CINO_INLINE
bool IntegralCurve<Mesh>::is_on_vertex(const Sample & s, unsigned int & vid, const double tol) const
{
    assert(s.pid!=-1);

    std::vector<unsigned int> non_zero_coords;
    for(unsigned int off=0; off<s.bary.size(); ++off)
    {
        if (s.bary.at(off) > tol) non_zero_coords.push_back(off);
    }

    if (non_zero_coords.size() == 1)
    {
        vid = m_ptr->poly_vert_id(s.pid, non_zero_coords.front());
        return true;
    }
    return false;
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class Mesh>
CINO_INLINE
bool IntegralCurve<Mesh>::is_on_edge(const Sample & s, unsigned int & eid, const double tol) const
{
    assert(s.pid!=-1);

    std::vector<unsigned int> non_zero_coords;
    for(size_t off=0; off<s.bary.size(); ++off)
    {
        if (s.bary.at(off) > tol) non_zero_coords.push_back(off);
    }

    if (non_zero_coords.size() == 2)
    {
        unsigned int vid0 = m_ptr->poly_vert_id(s.pid, non_zero_coords.at(0));
        unsigned int vid1 = m_ptr->poly_vert_id(s.pid, non_zero_coords.at(1));
        for(unsigned int id : m_ptr->adj_p2e(s.pid))
        {
            if (m_ptr->edge_contains_vert(id, vid0) &&
                m_ptr->edge_contains_vert(id, vid1))
            {
                eid = id;
                return true;
            }
        }
        assert(false && "Integral curve - Something is off here...");
    }
    return false;
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class Mesh>
CINO_INLINE
Curve::Sample IntegralCurve<Mesh>::make_sample(const unsigned int vid) const
{
    assert(!m_ptr->adj_v2p(vid).empty());
    unsigned int pid = m_ptr->adj_v2p(vid).front();
    unsigned int off = m_ptr->poly_vert_offset(pid, vid);
    Sample s;
    s.pos  = m_ptr->vert(vid);
    s.pid  = pid;
    s.bary = std::vector<double>(m_ptr->verts_per_poly(pid),0);
    s.bary.at(off) = 1.0;
    return s;
}

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
// ::::::::::::::::::::                                              :::::::::::::::::::: //
// :::::::::::::::::::: TEMPLATE SPECIALIZATIONS FOR TRIANGLE MESHES :::::::::::::::::::: //
// ::::::::::::::::::::                                              :::::::::::::::::::: //
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

template<>
CINO_INLINE
bool IntegralCurve<Trimesh<>>::is_on_face(const Sample & s, unsigned int & fid, const double tol) const
{
    assert(s.pid!=-1);

    std::vector<unsigned int> non_zero_coords;
    for(unsigned int off=0; off<s.bary.size(); ++off)
    {
        if (s.bary.at(off) > tol) non_zero_coords.push_back(off);
    }

    if (non_zero_coords.size() == 3)
    {
        fid = s.pid;
        return true;
    }
    return false;
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<>
CINO_INLINE
Curve::Sample IntegralCurve<Trimesh<>>::move_forward_from_vertex(const unsigned int vid)
{
    vec3d v = m_ptr->vert(vid);
    vec3d n = m_ptr->vert_data(vid).normal;
    Plane tangent_plane(v,n);

    vec3d grad{0,0,0};
    for(unsigned int fid : m_ptr->adj_v2p(vid)) grad += grad_ptr->vec_at(fid);
    grad = tangent_plane.project_onto(v+grad) - v;
    grad.normalize();
    assert(grad.norm() > 0);

    std::map<unsigned int,vec3d> tangent_space;
    for(unsigned int nbr : m_ptr->adj_v2v(vid))
    {
        tangent_space[nbr] = tangent_plane.project_onto(m_ptr->vert(nbr));
    }
    tangent_space[vid] = m_ptr->vert(vid);

    for(unsigned int fid : m_ptr->adj_v2p(vid))
    {
        int     eid = m_ptr->edge_opposite_to(fid, vid); assert(eid >= 0);
        vec3d   e0  = tangent_space.at( m_ptr->edge_vert_id(eid,0) );
        vec3d   e1  = tangent_space.at( m_ptr->edge_vert_id(eid,1) );
        vec3d   inters;
        // FIXME (francescozoccheddu)
        //if (intersection(Ray(v,grad), Segment(e0,e1), inters))
        abort(); // remove when fixed
        {
            Sample sample;
            sample.pid = fid;
            triangle_barycentric_coords(tangent_space.at( m_ptr->poly_vert_id(fid,0) ),
                                        tangent_space.at( m_ptr->poly_vert_id(fid,1) ),
                                        tangent_space.at( m_ptr->poly_vert_id(fid,2) ),
                                        inters, sample.bary.data());

            for(unsigned int off=0; off<3; ++off) sample.pos += sample.bary.at(off) * m_ptr->poly_vert(fid, off);

            return sample;
        }
    }
    assert(false && "Integral curve - Something is off here...");
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<>
CINO_INLINE
Curve::Sample IntegralCurve<Trimesh<>>::move_forward_from_edge(const unsigned int eid, const vec3d & p)
{
    assert(m_ptr->edge_is_manifold(eid));
    unsigned int   f0 = m_ptr->adj_e2p(eid).front();
    unsigned int   f1 = m_ptr->adj_e2p(eid).back();
    unsigned int   v0 = m_ptr->edge_vert_id(eid,0);
    unsigned int   v1 = m_ptr->edge_vert_id(eid,1);
    unsigned int   v2 = m_ptr->vert_opposite_to(f0, v0, v1);
    unsigned int   v3 = m_ptr->vert_opposite_to(f1, v0, v1);
    vec3d n  = m_ptr->poly_data(f0).normal + m_ptr->poly_data(f1).normal; n.normalize();

    Plane tangent_plane(p,n);

    std::map<unsigned int,vec3d> tangent_space;
    tangent_space[v0] = m_ptr->vert(v0);
    tangent_space[v1] = m_ptr->vert(v1);
    tangent_space[v2] = tangent_plane.project_onto(m_ptr->vert(v2));
    tangent_space[v3] = tangent_plane.project_onto(m_ptr->vert(v3));

    vec3d n0 = triangle_normal(tangent_space.at( m_ptr->poly_vert_id(f0,0) ),
                               tangent_space.at( m_ptr->poly_vert_id(f0,1) ),
                               tangent_space.at( m_ptr->poly_vert_id(f0,2) ));

    vec3d n1 = triangle_normal(tangent_space.at( m_ptr->poly_vert_id(f1,0) ),
                               tangent_space.at( m_ptr->poly_vert_id(f1,1) ),
                               tangent_space.at( m_ptr->poly_vert_id(f1,2) ));

    vec3d g0 = tangent_plane.project_onto(p + grad_ptr->vec_at(f0)) - p;
    vec3d g1 = tangent_plane.project_onto(p + grad_ptr->vec_at(f1)) - p;

    vec3d e  = m_ptr->edge_vert(eid,0) - m_ptr->edge_vert(eid,1); e.normalize();

    // if the gradient skins into, move along the edge towards the vertex
    // best aligned along the gradient direction

    if (n0.dot(e.cross(g0)) * n1.dot(e.cross(g1)) < 0) // gradient skins into condition
    {
        if (e.dot(g0) > 0) return make_sample(m_ptr->edge_vert_id(eid,0));
        else               return make_sample(m_ptr->edge_vert_id(eid,1));
    }

    // otherwise go for the standard way: compute the average gradient between the
    // triangles incident to the edge and step into the triangle pointed by the arrow

    vec3d grad = g0 + g1;
    grad = tangent_plane.project_onto(p + grad) - p; // this should be redundant...
    grad.normalize();
    assert(grad.norm() > 0);

    std::vector<ipair> edges_to_check;
    for(unsigned int e : m_ptr->adj_p2e(f0)) if (e!=eid) edges_to_check.push_back(std::make_pair(e,f0));
    for(unsigned int e : m_ptr->adj_p2e(f1)) if (e!=eid) edges_to_check.push_back(std::make_pair(e,f1));

    for(auto obj : edges_to_check)
    {
        unsigned int  e   = obj.first;
        unsigned int  fid = obj.second;
        vec3d v0  = tangent_space.at( m_ptr->edge_vert_id(e,0) );
        vec3d v1  = tangent_space.at( m_ptr->edge_vert_id(e,1) );
        vec3d inters;
        // FIXME (francescozoccheddu)
        //if (intersection(Ray(p,grad), Segment(v0,v1), inters))
        abort(); // remove when fixed
        {
            Sample sample;
            sample.pid = fid;
            triangle_barycentric_coords(tangent_space.at( m_ptr->poly_vert_id(fid,0) ),
                                        tangent_space.at( m_ptr->poly_vert_id(fid,1) ),
                                        tangent_space.at( m_ptr->poly_vert_id(fid,2) ),
                                        inters, sample.bary.data());

            for(unsigned int off=0; off<3; ++off) sample.pos += sample.bary.at(off) * m_ptr->poly_vert(fid,off);

            return sample;
        }
    }
    assert(false && "Integral curve - Something is off here...");
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<>
CINO_INLINE
Curve::Sample IntegralCurve<Trimesh<>>::move_forward_from_face(const unsigned int fid, const vec3d & p)
{
    vec3d grad = grad_ptr->vec_at(fid);
    grad.normalize();

    for(unsigned int e=0; e<3; ++e)
    {
        vec3d   e0  = m_ptr->poly_vert(fid,  TRI_EDGES[e][0]);
        vec3d   e1  = m_ptr->poly_vert(fid,  TRI_EDGES[e][1]);
        vec3d   inters;
        // FIXME (francescozoccheddu)
        //if (intersection(Ray(p, grad), Segment(e0, e1), inters))
        abort(); // remove when fixed
        {
            Sample sample;
            sample.pos = inters;
            sample.pid = fid;
            triangle_barycentric_coords(m_ptr->poly_vert(fid,0),
                                        m_ptr->poly_vert(fid,1),
                                        m_ptr->poly_vert(fid,2),
                                        inters, sample.bary.data());
            return sample;
        }
    }
    assert(false && "Integral curve - Something is off here...");
}


//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::


template<>
CINO_INLINE
Curve::Sample IntegralCurve<Trimesh<>>::move_forward(const Sample & s)
{
    unsigned int id;
    if (is_on_vertex(s,id))
    {
        return move_forward_from_vertex(id);
    }
    else if (is_on_edge(s,id))
    {
        return move_forward_from_edge(id, s.pos);
    }
    else if (is_on_face(s,id))
    {
        return move_forward_from_face(id, s.pos);
    }
    else assert(false && "Integral curve - Something is off here...");
}


//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<>
CINO_INLINE
bool IntegralCurve<Trimesh<>>::is_converged(const Curve::Sample & sample)
{
    if (sample.pid == -1) // i.e. boundary reached. Remove dummy sample
    {
        status = END_ON_BORDER;
        pop_back();
        return true;
    }

    bool maxima_reached = false;

    unsigned int id;
    if (is_on_vertex(sample, id))
    {
        if (m_ptr->vert_is_local_max(id)) maxima_reached = true;
    }
    else if (is_on_edge(sample, id))
    {
        unsigned int vid0        = m_ptr->edge_vert_id(id,0);
        unsigned int vid1        = m_ptr->edge_vert_id(id,1);
        bool vid0_is_max = m_ptr->vert_is_local_max(vid0);
        bool vid1_is_max = m_ptr->vert_is_local_max(vid1);

        if (vid0_is_max && vid1_is_max) maxima_reached = true;
    }
    else if (is_on_face(sample, id))
    {
        unsigned int vid0        = m_ptr->poly_vert_id(id,0);
        unsigned int vid1        = m_ptr->poly_vert_id(id,1);
        unsigned int vid2        = m_ptr->poly_vert_id(id,2);
        bool vid0_is_max = m_ptr->vert_is_local_max(vid0);
        bool vid1_is_max = m_ptr->vert_is_local_max(vid1);
        bool vid2_is_max = m_ptr->vert_is_local_max(vid2);

        if (vid0_is_max && vid1_is_max && vid2_is_max) maxima_reached = true;
    }

    if (maxima_reached)
    {
        status = END_ON_MAXIMA;
        return true;
    }

    if (samples().size() >= m_ptr->num_verts())
    {
        std::cerr << "INTEGRAL CURVE ERROR - Infinite loop. Something is REALLY off here!" << std::endl;
        status = INFINITE_LOOP;
        return true;
    }

    return false;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
// ::::::::::::::::::::                                                 :::::::::::::::::::: //
// :::::::::::::::::::: TEMPLATE SPECIALIZATIONS FOR TETRAHEDRAL MESHES :::::::::::::::::::: //
// ::::::::::::::::::::                                                 :::::::::::::::::::: //
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

template<>
CINO_INLINE
bool IntegralCurve<Tetmesh<>>::is_on_face(const Curve::Sample & s, unsigned int & tid, unsigned int & fid, const double tol) const
{
    assert(s.pid!=-1);

    std::vector<unsigned int> non_zero_coords;
    for(unsigned int off=0; off<s.bary.size(); ++off)
    {
        if (s.bary.at(off) > tol) non_zero_coords.push_back(off);
    }

    if (non_zero_coords.size() == 3)
    {
        std::vector<unsigned int> my_f;
        my_f.push_back(m_ptr->poly_vert_id(s.pid, non_zero_coords.at(0)));
        my_f.push_back(m_ptr->poly_vert_id(s.pid, non_zero_coords.at(1)));
        my_f.push_back(m_ptr->poly_vert_id(s.pid, non_zero_coords.at(2)));
        std::sort(my_f.begin(), my_f.end());

        for(unsigned int f=0; f<4; ++f)
        {
            std::vector<unsigned int> curr_f;
            curr_f.push_back(m_ptr->poly_vert_id(s.pid, TET_FACES[f][0]));
            curr_f.push_back(m_ptr->poly_vert_id(s.pid, TET_FACES[f][1]));
            curr_f.push_back(m_ptr->poly_vert_id(s.pid, TET_FACES[f][2]));
            std::sort(curr_f.begin(), curr_f.end());

            if (my_f == curr_f)
            {
                tid = s.pid;
                fid = f;
                return true;
            }
        }
        assert(false && "Integral curve - Something is off here...");
    }
    return false;
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<>
CINO_INLINE
bool IntegralCurve<Tetmesh<>>::is_on_poly(const Curve::Sample & s, unsigned int & pid, const double tol) const
{
    assert(s.pid!=-1);

    std::vector<unsigned int> non_zero_coords;
    for(unsigned int off=0; off<s.bary.size(); ++off)
    {
        if (s.bary.at(off) > tol) non_zero_coords.push_back(off);
    }

    if (non_zero_coords.size() == 4)
    {
        pid = s.pid;
        return true;
    }
    return false;
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<>
CINO_INLINE
Curve::Sample IntegralCurve<Tetmesh<>>::move_forward_from_vertex(const unsigned int vid)
{
    vec3d p = m_ptr->vert(vid);
    vec3d grad{0,0,0};
    for(unsigned int pid : m_ptr->adj_v2p(vid)) grad += grad_ptr->vec_at(pid);
    grad.normalize();
    assert(grad.norm() > 0);

    for(unsigned int pid : m_ptr->adj_v2p(vid))
    {
        unsigned int  fid = m_ptr->poly_face_opposite_to(pid, vid);
        vec3d v0  = m_ptr->poly_vert(pid, TET_FACES[fid][0]);
        vec3d v1  = m_ptr->poly_vert(pid, TET_FACES[fid][1]);
        vec3d v2  = m_ptr->poly_vert(pid, TET_FACES[fid][2]);
        vec3d inters;
        if (ray_triangle_intersection(p, grad, v0, v1, v2, inters))
        {
            Sample sample;
            sample.pos = inters;
            sample.pid = pid;
            tet_barycentric_coords(m_ptr->poly_vert(pid, 0),
                                   m_ptr->poly_vert(pid, 1),
                                   m_ptr->poly_vert(pid, 2),
                                   m_ptr->poly_vert(pid, 3),
                                   inters, sample.bary.data());
            return sample;
        }
    }

    Sample dummy;
    return dummy;
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<>
CINO_INLINE
Curve::Sample IntegralCurve<Tetmesh<>>::move_forward_from_edge(const unsigned int eid, const vec3d & p)
{
    vec3d grad{0,0,0};
    for(unsigned int pid : m_ptr->adj_e2p(eid)) grad += grad_ptr->vec_at(pid);
    grad.normalize();
    assert(grad.norm() > 0);

    unsigned int v0 = m_ptr->edge_vert_id(eid, 0);
    unsigned int v1 = m_ptr->edge_vert_id(eid, 1);

    for(unsigned int pid : m_ptr->adj_e2p(eid))
    {
        for(unsigned int f=0; f<m_ptr->faces_per_poly(); ++f)
        {
            // skip faces containing the edge eid
            if (m_ptr->poly_vert_id(pid, TET_FACES[f][0]) == v0 &&
                m_ptr->poly_vert_id(pid, TET_FACES[f][1]) == v1) continue;
            if (m_ptr->poly_vert_id(pid, TET_FACES[f][0]) == v1 &&
                m_ptr->poly_vert_id(pid, TET_FACES[f][1]) == v0) continue;
            if (m_ptr->poly_vert_id(pid, TET_FACES[f][1]) == v0 &&
                m_ptr->poly_vert_id(pid, TET_FACES[f][2]) == v1) continue;
            if (m_ptr->poly_vert_id(pid, TET_FACES[f][1]) == v1 &&
                m_ptr->poly_vert_id(pid, TET_FACES[f][2]) == v0) continue;
            if (m_ptr->poly_vert_id(pid, TET_FACES[f][2]) == v0 &&
                m_ptr->poly_vert_id(pid, TET_FACES[f][0]) == v1) continue;
            if (m_ptr->poly_vert_id(pid, TET_FACES[f][2]) == v1 &&
                m_ptr->poly_vert_id(pid, TET_FACES[f][0]) == v0) continue;

            vec3d v0 = m_ptr->poly_vert(pid, TET_FACES[f][0]);
            vec3d v1 = m_ptr->poly_vert(pid, TET_FACES[f][1]);
            vec3d v2 = m_ptr->poly_vert(pid, TET_FACES[f][2]);

            vec3d inters;
            if (ray_triangle_intersection(p, grad, v0, v1, v2, inters))
            {
                Sample sample;
                sample.pos = inters;
                sample.pid = pid;
                tet_barycentric_coords(m_ptr->poly_vert(pid, 0),
                                       m_ptr->poly_vert(pid, 1),
                                       m_ptr->poly_vert(pid, 2),
                                       m_ptr->poly_vert(pid, 3),
                                       inters, sample.bary.data());
                return sample;
            }
        }
    }

    Sample dummy;
    return dummy;
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<>
CINO_INLINE
Curve::Sample IntegralCurve<Tetmesh<>>::move_forward_from_face(const unsigned int pid, const unsigned int fid, const vec3d & p)
{
    std::vector<unsigned int> polys_to_process;
    polys_to_process.push_back(pid);
    vec3d grad = grad_ptr->vec_at(pid);

    int nbr = m_ptr->poly_adj_through_face(pid, fid);
    if (nbr != -1)
    {
        polys_to_process.push_back(nbr);
        grad += grad_ptr->vec_at(nbr);
    }
    grad.normalize();
    if (grad.norm() == 0) // need to understand when and why this may happen....
    {
        Sample dummy;
        return dummy;
    }

    std::vector<unsigned int> illegal_f;
    illegal_f.push_back(m_ptr->poly_vert_id(pid, TET_FACES[fid][0]));
    illegal_f.push_back(m_ptr->poly_vert_id(pid, TET_FACES[fid][1]));
    illegal_f.push_back(m_ptr->poly_vert_id(pid, TET_FACES[fid][2]));
    std::sort(illegal_f.begin(), illegal_f.end());

    for(unsigned int curr_poly : polys_to_process)
    {
        for(unsigned int f=0; f<m_ptr->faces_per_poly(); ++f)
        {
            std::vector<unsigned int> curr_f;
            curr_f.push_back(m_ptr->poly_vert_id(curr_poly, TET_FACES[f][0]));
            curr_f.push_back(m_ptr->poly_vert_id(curr_poly, TET_FACES[f][1]));
            curr_f.push_back(m_ptr->poly_vert_id(curr_poly, TET_FACES[f][2]));
            std::sort(curr_f.begin(), curr_f.end());

            if (curr_f == illegal_f) continue;

            vec3d v0 = m_ptr->vert(curr_f[0]);
            vec3d v1 = m_ptr->vert(curr_f[1]);
            vec3d v2 = m_ptr->vert(curr_f[2]);

            vec3d inters;
            if (ray_triangle_intersection(p, grad, v0, v1, v2, inters))
            {
                Sample sample;
                sample.pos = inters;
                sample.pid = curr_poly;
                tet_barycentric_coords(m_ptr->poly_vert(curr_poly, 0),
                                       m_ptr->poly_vert(curr_poly, 1),
                                       m_ptr->poly_vert(curr_poly, 2),
                                       m_ptr->poly_vert(curr_poly, 3),
                                       inters, sample.bary.data());
                return sample;
            }
        }
    }
    // this may actually happen if the point is on the surface and the gradient points outside of the domain...
    // assert(false && "Integral curve - Something is off here...");
    Sample dummy;
    return dummy;
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<>
CINO_INLINE
Curve::Sample IntegralCurve<Tetmesh<>>::move_forward_from_poly(const unsigned int pid, const vec3d & p)
{
    vec3d grad = grad_ptr->vec_at(pid);
    grad.normalize();

    for(unsigned int f=0; f<m_ptr->faces_per_poly(); ++f)
    {
        vec3d v0 = m_ptr->poly_vert(pid, TET_FACES[f][0]);
        vec3d v1 = m_ptr->poly_vert(pid, TET_FACES[f][1]);
        vec3d v2 = m_ptr->poly_vert(pid, TET_FACES[f][2]);
        vec3d inters;
        if (ray_triangle_intersection(p, grad, v0, v1, v2, inters))
        {
            Sample sample;
            sample.pos = inters;
            sample.pid = pid;
            tet_barycentric_coords(m_ptr->poly_vert(pid,0),
                                   m_ptr->poly_vert(pid,1),
                                   m_ptr->poly_vert(pid,2),
                                   m_ptr->poly_vert(pid,3),
                                   inters, sample.bary.data());
            return sample;
        }
    }
    assert(false && "Integral curve - Something is off here...");
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<>
CINO_INLINE
Curve::Sample IntegralCurve<Tetmesh<>>::move_forward(const Curve::Sample & s)
{
    unsigned int id, id2;
    if (is_on_vertex(s,id)) // vid
    {
        return move_forward_from_vertex(id);
    }
    else if (is_on_edge(s,id)) // eid
    {
        return move_forward_from_edge(id, s.pos);
    }
    else if (is_on_face(s,id,id2)) // tet id, facet id
    {
        return move_forward_from_face(id, id2, s.pos);
    }
    else if (is_on_poly(s,id)) // tet id
    {
        return move_forward_from_poly(id, s.pos);
    }
    else assert(false && "Integral curve - Something is off here...");
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<>
CINO_INLINE
bool IntegralCurve<Tetmesh<>>::is_converged(const Curve::Sample & sample)
{
    if (sample.pid == -1) // i.e. boundary reached. Remove dummy sample
    {
        status = END_ON_BORDER;
        pop_back();
        return true;
    }

    bool maxima_reached = false;

    unsigned int id, id1;
    if (is_on_vertex(sample, id))
    {
        if (m_ptr->vert_is_local_max(id))  maxima_reached = true;
    }
    else if (is_on_edge(sample, id))
    {
        unsigned int vid0        = m_ptr->edge_vert_id(id,0);
        unsigned int vid1        = m_ptr->edge_vert_id(id,1);
        bool vid0_is_max = m_ptr->vert_is_local_max(vid0);
        bool vid1_is_max = m_ptr->vert_is_local_max(vid1);

        if (vid0_is_max && vid1_is_max) maxima_reached = true;
    }
    else if (is_on_face(sample, id, id1))
    {
        unsigned int vid0        = m_ptr->poly_vert_id(id, TET_FACES[id1][0]);
        unsigned int vid1        = m_ptr->poly_vert_id(id, TET_FACES[id1][1]);
        unsigned int vid2        = m_ptr->poly_vert_id(id, TET_FACES[id1][2]);
        bool vid0_is_max = m_ptr->vert_is_local_max(vid0);
        bool vid1_is_max = m_ptr->vert_is_local_max(vid1);
        bool vid2_is_max = m_ptr->vert_is_local_max(vid2);

        if (vid0_is_max && vid1_is_max && vid2_is_max) maxima_reached = true;
    }
    else if (is_on_poly(sample, id))
    {
        unsigned int vid0        = m_ptr->poly_vert_id(id,0);
        unsigned int vid1        = m_ptr->poly_vert_id(id,1);
        unsigned int vid2        = m_ptr->poly_vert_id(id,2);
        unsigned int vid3        = m_ptr->poly_vert_id(id,3);
        bool vid0_is_max = m_ptr->vert_is_local_max(vid0);
        bool vid1_is_max = m_ptr->vert_is_local_max(vid1);
        bool vid2_is_max = m_ptr->vert_is_local_max(vid2);
        bool vid3_is_max = m_ptr->vert_is_local_max(vid3);

        if (vid0_is_max && vid1_is_max && vid2_is_max && vid3_is_max) maxima_reached = true;
    }

    if (maxima_reached)
    {
        status = END_ON_MAXIMA;
        return true;
    }

    if (samples().size() >= m_ptr->num_verts())
    {
        std::cerr << "INTEGRAL CURVE ERROR - Infinite loop. Something is REALLY off here!" << std::endl;
        return true;
    }

    return false;
}

}
