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
#include <cinolib/lscm.h>
#include <cinolib/laplacian.h>
#include <cinolib/linear_solvers.h>
#include <cinolib/vector_area_matrix.h>

namespace cinolib
{

template<class M, class V, class E, class P>
CINO_INLINE
ScalarField LSCM(const Trimesh<M,V,E,P>     & m,
                 const std::map<unsigned int,vec2d> & bc)
{
    std::map<unsigned int,double> bc_uv;
    if(!bc.empty())
    {
        for(auto obj : bc)
        {
            bc_uv[obj.first]               = obj.second.x();
            bc_uv[obj.first+m.num_verts()] = obj.second.y();
        }
    }
    else // fix two distant points on the boundary
    {
        auto b_verts = m.get_ordered_boundary_vertices();
        unsigned int v0 = b_verts.front();
        unsigned int v1 = b_verts.at(b_verts.size()*0.5);
        unsigned int nv = m.num_verts();
        bc_uv[v0   ] = 0;
        bc_uv[v0+nv] = 0;
        bc_uv[v1   ] = 1;
        bc_uv[v1+nv] = 1;
    }

    Eigen::SparseMatrix<double> L   = laplacian(m, COTANGENT, 2);
    Eigen::SparseMatrix<double> A   = vector_area_matrix(m);
    Eigen::VectorXd             rhs = Eigen::VectorXd::Zero(2*m.num_verts());

    ScalarField f_uv;
    solve_square_system_with_bc(-L+2*A, rhs, f_uv, bc_uv, SIMPLICIAL_LDLT);
    return f_uv;
}

}
