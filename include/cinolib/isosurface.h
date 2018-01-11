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
#ifndef CINO_ISOSURFACE_H
#define CINO_ISOSURFACE_H

#include <cinolib/cino_inline.h>
#include <cinolib/ipair.h>
#include <cinolib/meshes/tetmesh.h>


namespace cinolib
{

template<class M = Mesh_std_attributes, // default template arguments
         class V = Vert_std_attributes,
         class E = Edge_std_attributes,
         class F = Polygon_std_attributes,
         class P = Polyhedron_std_attributes>
class Isosurface
{
    public:

        explicit Isosurface(){}
        explicit Isosurface(const Tetmesh<M,V,E,F,P> & m, const float iso_value);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        Trimesh<M,V,E,F> export_as_trimesh() const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void tessellate(std::vector<double> & coords,
                        std::vector<uint>   & new_polys,
                        std::vector<double> & new_field) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        const std::map<ipair,double> & edges_split() const { return split_info; }

    protected:

        void fix_subtet_orientation(const uint                  pid,
                                    const uint                  n_subtets,
                                    const std::vector<double> & coords,
                                          std::vector<uint>   & cells) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        const Tetmesh<M,V,E,F,P> *m_ptr;
        float                     iso_value;
        std::vector<double>       coords;
        std::vector<uint>         tris;
        std::vector<double>       t_norms;
        std::map<ipair,double>    split_info;
};


}

#ifndef  CINO_STATIC_LIB
#include "isosurface.cpp"
#endif

#endif // CINO_ISOSURFACE_H
