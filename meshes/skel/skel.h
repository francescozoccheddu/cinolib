/****************************************************************************
* Italian National Research Council                                         *
* Institute for Applied Mathematics and Information Technologies, Genoa     *
* IMATI-GE / CNR                                                            *
*                                                                           *
* Author: Marco Livesu (marco.livesu@gmail.com)                             *
*                                                                           *
* Copyright(C) 2016                                                         *
* All rights reserved.                                                      *
*                                                                           *
* This file is part of CinoLib                                              *
*                                                                           *
* CinoLib is free software; you can redistribute it and/or modify           *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 3 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
****************************************************************************/
#ifndef SKEL_H
#define SKEL_H

#include <cinolib/cinolib.h>
#include <cinolib/geometry/vec3.h>
#include <cinolib/bbox.h>
#include <cinolib/drawable_object.h>

#include <vector>
#include <queue>
#include <set>
#include <assert.h>

namespace cinolib
{

class Skel
{
    public:

        typedef std::pair<int,int> seg;

        Skel(){}
        Skel(const char * filename) { load(filename); }
        Skel(const std::vector<double> & coords, const std::vector<int> & segs);

    protected:

        Bbox                            bbox;
        std::vector<double>             coords;
        std::vector<int>                segments;
        std::vector<double>             max_spheres;
        std::vector<int>                vtx_bone_ids;
        std::vector<int>                seg_bone_ids;
        std::vector< std::vector<int> > vtx2vtx;
        std::vector< std::vector<int> > vtx2seg;
        std::vector< std::vector<int> > seg2seg;
        std::vector< std::vector<int> > vtx_bones;
        std::vector< std::vector<int> > seg_bones;

        void build_adjacency();
        void build_bones();
        void update_bbox();
        void clear();

    public:

        virtual void operator+=(const Skel & s);

        void load(const std::vector<double> & coords, const std::vector<int> & segments);
        void load(const char * filename);
        void save(const char * filename) const;

        inline const std::vector<double> & vector_coords()   const { return coords;   }
        inline const std::vector<int>    & vector_segments() const { return segments; }

        void remove_vertices(std::vector<int> & to_remove);
        void remove_vertex(int vid_to_remove);

        int add_vertex(const vec3d & v);
        int add_segment(int vid0, int vid1);

        vec3d get_bone_sample   (int bid, float q) const;
        vec3d get_segment_sample(int sid, float q) const;
        int sample_segment_at(int sid, float q);
        int sample_bone_at(int bid, float q);

        inline int num_vertices() const { return coords.size() / 3;    }
        inline int num_segments() const { return segments.size() / 2;  }
        inline int num_bones()    const { return vtx_bones.size();     }

        inline std::vector<int> get_joints() const
        {
            std::vector<int> j;
            for(int vid=0; vid<num_vertices(); ++vid)
            {
                if (vertex_is_joint(vid)) j.push_back(vid);
            }
            return j;
        }

        inline int num_joints() const
        {
            return (int)get_joints().size();
        }

        inline int vertex_valence (int vid) const
        {
            CHECK_BOUNDS(vtx2vtx, vid);
            return vtx2vtx[vid].size();
        }

        inline bool vertex_is_leaf    (int vid) const { return (vertex_valence(vid) == 1);  }
        inline bool vertex_is_bone    (int vid) const { return (vertex_valence(vid) == 2);  }
        inline bool vertex_is_joint   (int vid) const { return (vertex_valence(vid) >= 3);  }
        inline bool vertex_is_feature (int vid) const { return (vertex_valence(vid) != 2);  }

        inline int  segment_valence (int sid) const
        {
            CHECK_BOUNDS(seg2seg, sid);
            return seg2seg[sid].size();
        }

        inline bool segment_is_leaf   (int sid) const { return (segment_valence(sid) == 1); }
        inline bool segment_is_bone   (int sid) const { return (segment_valence(sid) == 2); }
        inline bool segment_is_joint  (int sid) const { return (segment_valence(sid) >= 3); }
        inline bool segment_is_feature(int sid) const { return (segment_valence(sid) != 2); }

        inline std::vector<int> adj_vtx2vtx(int vid) const
        { CHECK_BOUNDS(vtx2vtx, vid); return vtx2vtx[vid]; }

        inline int adj_vtx2vtx(int vid, int i) const
        { CHECK_BOUNDS(vtx2vtx, vid); return vtx2vtx[vid][i]; }

        inline std::vector<int> adj_seg2seg(int sid) const
        { CHECK_BOUNDS(seg2seg, sid); return seg2seg[sid]; }

        inline int adj_seg2seg(int sid, int i) const
        { CHECK_BOUNDS(seg2seg, sid); return seg2seg[sid][i]; }

        inline std::vector<int> adj_vtx2seg(int vid) const
        { CHECK_BOUNDS(vtx2seg, vid); return vtx2seg[vid]; }

        inline int adj_vtx2seg(int vid, int i) const
        { CHECK_BOUNDS(vtx2seg, vid); return vtx2seg[vid][i]; }

        inline int vertex_bone_id(int vid) const
        { CHECK_BOUNDS(vtx_bone_ids, vid); return vtx_bone_ids[vid]; }

        inline void set_vertex_bone_id(int vid, int bid)
        { CHECK_BOUNDS(vtx_bone_ids, vid); vtx_bone_ids[vid] = bid; }

        inline int segment_vertex_id(int sid, int i) const
        { CHECK_BOUNDS(segments,2*sid+i); return segments[2*sid+i]; }

        inline int segment_bone_id(int sid) const
        { CHECK_BOUNDS(seg_bone_ids, sid); return seg_bone_ids[sid]; }

        inline void set_segment_bone_id(int sid, int bid)
        { CHECK_BOUNDS(seg_bone_ids, sid); seg_bone_ids[sid] = bid; }

        inline bool segment_contains_vertex(int sid, int vid) const
        {
            return (segment_vertex_id(sid, 0) == vid) ||
                   (segment_vertex_id(sid, 1) == vid);
        }

        inline std::vector<int> vertex_bone(int bid)  const
        { CHECK_BOUNDS(vtx_bones, bid); return vtx_bones[bid]; }
        inline std::vector<int> segment_bone(int bid) const
        { CHECK_BOUNDS(seg_bones, bid); return seg_bones[bid]; }

        inline double segment_length(int sid) const
        {
            seg s = segment(sid);
            return (vertex(s.first) - vertex(s.second)).length();
        }

        inline double bone_length(int bid) const
        {
            double length = 0;
            std::vector<int> bone = segment_bone(bid);
            for(int i=0; i<(int)bone.size(); ++i) length += segment_length(bone[i]);
            return length;
        }

        inline void set_vertex(int vid, const vec3<double> & v)
        {
            int vid_ptr = vid * 3;
            CHECK_BOUNDS(coords, vid_ptr+2);
            coords[vid_ptr + 0] = v.x();
            coords[vid_ptr + 1] = v.y();
            coords[vid_ptr + 2] = v.z();
        }

        inline vec3<double> vertex(int vid) const
        {
            int vid_ptr = vid * 3;
            CHECK_BOUNDS(coords, vid_ptr);
            return vec3<double>(coords[vid_ptr+0], coords[vid_ptr+1], coords[vid_ptr+2]);
        }

        inline seg segment(int sid) const
        {
            int sid_ptr = sid * 2;
            CHECK_BOUNDS(segments, sid_ptr+1);
            return std::make_pair(segments[sid_ptr+0], segments[sid_ptr+1]);
        }

        inline void set_segment(int sid, int i, int vid)
        {
            int sid_ptr = sid * 2;
            CHECK_BOUNDS(segments, sid_ptr+i);
            segments[sid_ptr + i] = vid;
        }

        inline vec3<double> segment_vertex(int sid, int i) const
        {
            return vertex(segment_vertex_id(sid,i));
        }

        inline void segment_switch_order(int sid)
        {
            int sid_ptr = sid * 2;
            CHECK_BOUNDS(segments, sid_ptr+1);
            int tmp = segments[sid_ptr + 0];
            segments[sid_ptr + 0] = segments[sid_ptr + 1];
            segments[sid_ptr + 1] = tmp;
        }

        inline double max_sphere_radius(int vid) const
        {
            CHECK_BOUNDS(max_spheres, vid);
            return max_spheres[vid];
        }

        inline void reset_max_sphere_radius()
        {
            max_spheres.clear();
            max_spheres.resize(num_vertices(), 0.0);
        }

        inline void set_max_sphere_radius(int vid, double r)
        {
            CHECK_BOUNDS(max_spheres, vid);
            max_spheres[vid] = r;
        }

        inline int next_vertex(int curr_vid, int prev_vid) const
        {
            assert(vertex_is_bone(curr_vid));
            if (adj_vtx2vtx(curr_vid, 0) == prev_vid)
            {
                return adj_vtx2vtx(curr_vid, 1);
            }
            return adj_vtx2vtx(curr_vid, 0);
        }

        inline std::set<int> adjacent_bones(int vid) const
        {
            std::set<int> adj_bones;
            std::vector<int> nbrs = adj_vtx2vtx(vid);
            for(int i=0; i<(int)nbrs.size(); ++i)
            {
                int nbr = nbrs[i];
                adj_bones.insert( vertex_bone_id(nbr) );
            }
            return adj_bones;
        }

        bool are_bones_adjacent(const int bone_a, const int bone_b) const;
};

}

#ifndef  CINO_STATIC_LIB
#include "skel.cpp"
#endif

#endif // SKEL_H
