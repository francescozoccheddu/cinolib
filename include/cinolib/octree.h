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
#ifndef CINO_OCTREE_H
#define CINO_OCTREE_H

#include <cinolib/geometry/spatial_data_structure_item.h>
#include <cinolib/meshes/meshes.h>
#include <queue>
#include <unordered_set>
#include <cassert>

namespace cinolib
{

class OctreeNode
{
    public:
        OctreeNode(const OctreeNode * father, const AABB & bbox) : father(father), bbox(bbox) {}
       ~OctreeNode();
        const OctreeNode *father;
        OctreeNode       *children[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
        AABB              bbox;
        std::vector<unsigned int> item_indices; // index Octree::items, avoiding to store a copy of the same object multiple times in each node it appears
        bool              is_inner = false;
};
// TODO: due to the bool flag this struct requires 7 bytes padding. Also the
// pointer to the father is not necessary and could be removed, shrinking it.
// It could be designed better....
// https://stackoverflow.com/questions/4306186/structure-padding-and-packing
// http://www.catb.org/esr/structure-packing/

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

/* Usage:
 *
 *  i)   Create an empty octree
 *  ii)  Use the push_segment/triangle/tetrahedron facilities to populate it
 *  iii) Call build to make the tree
*/

class Octree
{
    public:

        explicit Octree(const unsigned int max_depth      = 7,
                        const unsigned int items_per_leaf = 50);

        virtual ~Octree();

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void push_point      (const unsigned int id, const vec3d & v);
        void push_sphere     (const unsigned int id, const vec3d & c, const double r);
        void push_segment    (const unsigned int id, const std::vector<vec3d> & v);
        void push_triangle   (const unsigned int id, const std::vector<vec3d> & v);
        void push_tetrahedron(const unsigned int id, const std::vector<vec3d> & v);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void build();
        void clear();

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void subdivide(OctreeNode *node);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        template<class M, class V, class E, class P>
        void build_from_mesh_polys(const AbstractPolygonMesh<M,V,E,P> & m)
        {
            assert(items.empty());
            items.reserve(m.num_polys());
            for(unsigned int pid=0; pid<m.num_polys(); ++pid)
            {
                for(unsigned int i=0; i<m.poly_tessellation(pid).size()/3; ++i)
                {
                    vec3d v0 = m.vert(m.poly_tessellation(pid).at(3*i+0));
                    vec3d v1 = m.vert(m.poly_tessellation(pid).at(3*i+1));
                    vec3d v2 = m.vert(m.poly_tessellation(pid).at(3*i+2));
                    push_triangle(pid, {v0,v1,v2});
                }
            }
            build();
        }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        template<class M, class V, class E, class F, class P>
        void build_from_mesh_polys(const AbstractPolyhedralMesh<M,V,E,F,P> & m)
        {
            assert(items.empty());
            items.reserve(m.num_polys());
            for(unsigned int pid=0; pid<m.num_polys(); ++pid)
            {
                switch(m.mesh_type())
                {
                    case TETMESH : push_tetrahedron(pid, m.poly_verts(pid)); break;
                    default: assert(false && "Unsupported element");
                }
            }
            build();
        }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void build_from_vectors(const std::vector<vec3d> & verts,
                                const std::vector<unsigned int>  & tris)
        {
            assert(items.empty());
            items.reserve(tris.size()/3);
            for(unsigned int i=0; i<tris.size(); i+=3)
            {
                push_triangle(i/3, { verts.at(tris.at(i  )),
                                    verts.at(tris.at(i+1)),
                                    verts.at(tris.at(i+2))});
            }
            build();
        }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        template<class M, class V, class E, class P>
        void build_from_mesh_edges(const AbstractMesh<M,V,E,P> & m)
        {
            assert(items.empty());
            items.reserve(m.num_edges());
            for(unsigned int eid=0; eid<m.num_edges(); ++eid)
            {
                push_segment(eid, m.edge_verts(eid));
            }
            build();
        }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        template<class M, class V, class E, class P>
        void build_from_mesh_points(const AbstractMesh<M,V,E,P> & m)
        {
            assert(items.empty());
            items.reserve(m.num_verts());
            for(unsigned int vid=0; vid<m.num_verts(); ++vid)
            {
                push_point(vid, m.vert(vid));
            }
            build();
        }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        unsigned int max_items_per_leaf() const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void debug_mode(const bool b);

        // QUERIES :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        // returns pos, id and distance of the item that is closest to query point p
        void  closest_point(const vec3d & p, unsigned int & id, vec3d & pos, double & dist) const;
        vec3d closest_point(const vec3d & p) const;

        // returns respectively the first item and the full list of items containing query point p
        // note: this query becomes exact if CINOLIB_USES_EXACT_PREDICATES is defined
        bool contains(const vec3d & p, const bool strict, unsigned int & id) const;
        bool contains(const vec3d & p, const bool strict, std::unordered_set<unsigned int> & ids) const;

        bool intersects_line(const vec3d& p, const vec3d& dir, double& min_t, unsigned int& id) const; // first hit
        
        // returns respectively the first and the full list of intersections
        // between items in the octree and a ray R(t) := p + t * dir
        bool intersects_ray(const vec3d & p, const vec3d & dir, double & min_t, unsigned int & id) const; // first hit
        bool intersects_ray(const vec3d & p, const vec3d & dir, std::set<std::pair<double,unsigned int>> & all_hits) const;

        // note: these queries become exact if CINOLIB_USES_EXACT_PREDICATES is defined
        bool intersects_segment (const vec3d s[], const bool ignore_if_valid_complex, std::unordered_set<unsigned int> & ids) const;
        bool intersects_triangle(const vec3d t[], const bool ignore_if_valid_complex, std::unordered_set<unsigned int> & ids) const;

        // WARNING: this function may return false positives because it only checks intersection between
        // the box b and the AABB of the items in the tree. This is a partial result that it is useful for
        // some of the queries above, where a more expensive test between the geometric entity approximated
        // by box b and the actual items will be performed
        bool intersects_box(const AABB & b, std::unordered_set<unsigned int> & ids) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        // all items live here, and leaf nodes only store indices to items
        std::vector<SpatialDataStructureItem*> items;
        OctreeNode                            *root = nullptr;
        std::vector<const OctreeNode*>         leaves;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    private:

        bool intersects_ray_or_line(const vec3d& p, const vec3d& dir, double& min_t, unsigned int& id, bool line) const; // first hit

    protected:

        unsigned int max_depth;      // maximum allowed depth of the tree
        unsigned int items_per_leaf; // prescribed number of items per leaf (can't go deeper than max_depth anyways)
        unsigned int tree_depth = 0; // actual depth of the tree
        bool print_debug_info = false;

        // SUPPORT STRUCTURES ::::::::::::::::::::::::::::::::::::::::::::::::::::

        struct Obj
        {
            double      dist  = inf_double;
            OctreeNode *node  = nullptr;
            int         index = -1; // note: this is the item ID, NOT necessarily the index of vector items!!
            vec3d       pos;        // closest point
        };
        struct Greater
        {
            bool operator()(const Obj & obj1, const Obj & obj2)
            {
                return obj1.dist > obj2.dist;
            }
        };
        typedef std::priority_queue<Obj,std::vector<Obj>,Greater> PrioQueue;
};

}

#ifndef  CINO_STATIC_LIB
#include "octree.cpp"
#endif

#endif // CINO_OCTREE_H
