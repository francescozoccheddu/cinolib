/*********************************************************************************
*  Copyright(C) 2016: Marco Livesu                                               *
*  All rights reserved.                                                          *
*                                                                                *
*  This file is part of CinoLib                                                  *
*                                                                                *
*  CinoLib is dual-licensed:                                                     *
*                                                                                *
*   - For non-commercial use you can redistribute it and/or modify it under the  *
*     terms of the GNU General Public License as published by the Free Software  *
*     Foundation; either version 3 of the License, or (at your option) any later *
*     version.                                                                   *
*                                                                                *
*   - If you wish to use it as part of a commercial software, a proper agreement *
*     with the Author(s) must be reached, based on a proper licensing contract.  *
*                                                                                *
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE       *
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.     *
*                                                                                *
*  Author(s):                                                                    *
*                                                                                *
*     Marco Livesu (marco.livesu@gmail.com)                                      *
*     http://pers.ge.imati.cnr.it/livesu/                                        *
*                                                                                *
*     Italian National Research Council (CNR)                                    *
*     Institute for Applied Mathematics and Information Technologies (IMATI)     *
*     Via de Marini, 6                                                           *
*     16149 Genoa,                                                               *
*     Italy                                                                      *
**********************************************************************************/
#include <cinolib/meshes/polyhedralmesh.h>
#include <cinolib/io/read_write.h>
#include <cinolib/bbox.h>
#include <cinolib/geometry/vec3.h>
#include <cinolib/vector_serialization.h>

#include <algorithm>
#include <cmath>
#include <float.h>
#include <map>
#include <set>

namespace cinolib
{

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
Polyhedralmesh<M,V,E,F,P>::Polyhedralmesh(const char * filename)
{
    load(filename);
    init();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
Polyhedralmesh<M,V,E,F,P>::Polyhedralmesh(const std::vector<vec3d>             & verts,
                                          const std::vector<std::vector<uint>> & faces,
                                          const std::vector<std::vector<uint>> & polys,
                                          const std::vector<std::vector<bool>> & polys_face_winding)
{
    this->verts = verts;
    this->faces = faces;
    this->polys = polys;
    this->polys_face_winding = polys_face_winding;

    init();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
void Polyhedralmesh<M,V,E,F,P>::load(const char * filename)
{
    this->clear();
    std::vector<double> coords;

    std::string str(filename);
    std::string filetype = str.substr(str.size()-6,6);

    if (filetype.compare("hybrid") == 0 ||
        filetype.compare("HYBRID") == 0)
    {
        read_HYBDRID(filename, coords, this->faces, this->polys, this->polys_face_winding);
    }
    else if (filetype.compare(".hedra") == 0 ||
             filetype.compare(".HEDRA") == 0)
    {
        read_HEDRA(filename, coords, this->faces, this->polys, this->polys_face_winding);
    }
    else
    {
        std::cerr << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : load() : file format not supported yet " << std::endl;
        exit(-1);
    }

    this->verts = vec3d_from_serialized_xyz(coords);

    std::cout << this->num_verts() << " verts read" << std::endl;
    std::cout << this->num_faces() << " faces read" << std::endl;
    std::cout << this->num_polys() << " polys read" << std::endl;

    this->mesh_data().filename = std::string(filename);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
void Polyhedralmesh<M,V,E,F,P>::save(const char * filename) const
{
    std::string str(filename);
    std::string filetype = str.substr(str.size()-6,6);

    if (filetype.compare(".hedra") == 0 ||
        filetype.compare(".HEDRA") == 0)
    {
        write_HEDRA(filename, this->verts, this->faces, this->polys, this->polys_face_winding);
    }
    else
    {
        std::cerr << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : write() : file format not supported yet " << std::endl;
        exit(-1);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
void Polyhedralmesh<M,V,E,F,P>::init()
{
    AbstractPolyhedralMesh<M,V,E,F,P>::init();
    update_face_tessellation();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
void Polyhedralmesh<M,V,E,F,P>::clear()
{
    AbstractPolyhedralMesh<M,V,E,F,P>::clear();
    triangulated_faces.clear();
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
void Polyhedralmesh<M,V,E,F,P>::update_face_tessellation()
{
    triangulated_faces.resize(this->num_faces());
    std::set<uint> bad_faces;

    for(uint fid=0; fid<this->num_faces(); ++fid)
    {
        // Assume convexity and try trivial tessellation first. If something flips
        // apply earcut algorithm to get a valid triangulation (bad_faces vector)

        std::vector<vec3d> n;
        for (uint i=2; i<this->verts_per_face(fid); ++i)
        {
            uint vid0 = this->faces.at(fid).at( 0 );
            uint vid1 = this->faces.at(fid).at(i-1);
            uint vid2 = this->faces.at(fid).at( i );

            triangulated_faces.at(fid).push_back(vid0);
            triangulated_faces.at(fid).push_back(vid1);
            triangulated_faces.at(fid).push_back(vid2);

            n.push_back((this->vert(vid1)-this->vert(vid0)).cross(this->vert(vid2)-this->vert(vid0)));
        }
        // check for badly tessellated polygons...
        for(uint i=0; i<n.size()-1; ++i) if (n.at(i).dot(n.at(i+1))<0) bad_faces.insert(fid);
    }
    //
    for(uint fid : bad_faces)
    {
        // NOTE: the triangulation is constructed on a proxy polygon obtained
        // projecting the actual polygon onto the best fitting plane. Bad things
        // can still happen for highly non-planar polygons

        std::vector<vec3d> vlist(this->verts_per_face(fid));
        for (uint i=0; i<this->verts_per_face(fid); ++i)
        {
            vlist.at(i) = this->face_vert(fid,i);
        }
        //
        std::vector<uint> tris;
        if (polygon_triangulate(vlist, tris))
        {
            triangulated_faces.at(fid).clear();
            for(uint off : tris) triangulated_faces.at(fid).push_back(this->face_vert_id(fid,off));
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
void Polyhedralmesh<M,V,E,F,P>::update_f_normal(const uint fid)
{
    assert(this->verts_per_face(fid)>2);
    std::vector<vec3d> points;
    for(uint off=0; off<this->verts_per_face(fid); ++off) points.push_back(this->face_vert(fid,off));
    this->face_data(fid).normal = polygon_normal(points);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
std::vector<uint> Polyhedralmesh<M,V,E,F,P>::face_tessellation(const uint fid) const
{
    return triangulated_faces.at(fid);
}

}
