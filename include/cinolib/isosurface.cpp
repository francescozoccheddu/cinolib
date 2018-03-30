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
#include <cinolib/isosurface.h>
#include <cinolib/cino_inline.h>
#include <cinolib/marching_tets.h>

namespace cinolib
{

CINO_INLINE
bool is_into_interval(double v, double bound_0, double bound1)
{
    if (v >= bound_0 && v <= bound1) return true;
    if (v <= bound_0 && v >= bound1) return true;
    return false;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
Isosurface<M,V,E,F,P>::Isosurface(const Tetmesh<M,V,E,F,P> &m, const float iso_value)
    : iso_value(iso_value)
{
    marching_tets(m, iso_value, coords, tris, t_norms);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
Trimesh<M,V,E,F> Isosurface<M,V,E,F,P>::export_as_trimesh() const
{
    return Trimesh<M,V,E,F>(coords, tris);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
std::vector<uint> Isosurface<M,V,E,F,P>::tessellate(Tetmesh<M,V,E,F,P> & m) const
{
    typedef std::pair<uint,double> split_data;
    std::set<split_data,std::greater<split_data>> edges_to_split; // from highest to lowest id

    for(uint eid=0; eid<m.num_edges(); ++eid)
    {
        double f0 = m.vert_data(m.edge_vert_id(eid,0)).uvw[0];
        double f1 = m.vert_data(m.edge_vert_id(eid,1)).uvw[0];

        if (is_into_interval(iso_value, f0, f1))
        {
            double alpha = std::fabs(iso_value - f0)/fabs(f1 - f0);
            edges_to_split.insert(std::make_pair(eid,alpha));
        }
    }

    std::vector<uint> new_vids;
    for(auto e : edges_to_split)
    {
        uint vid = m.edge_split(e.first, e.second);
        m.vert_data(vid).uvw[0] = iso_value;
        new_vids.push_back(vid);
    }

    return new_vids;
}

}
