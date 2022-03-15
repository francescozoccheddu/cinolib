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
#ifndef CINO_SUBDIVISION_MIDPOINT_H
#define CINO_SUBDIVISION_MIDPOINT_H

#include <cinolib/meshes/meshes.h>
#include <map>

namespace cinolib
{

/* This method implements midpoint subdivision, as described in:
 *
 * Hexahedral Meshing Using Midpoint Subdivision and Integer Programming
 * T.S. Li, R.M. McKeag, C.G. Armstrong
 * Computer Methods in Applied Mechanics and Engineering, 1995
*/

template<class M, class V, class E, class F, class P>
CINO_INLINE
void subdivision_midpoint(const AbstractPolyhedralMesh<M,V,E,F,P> & m_in,
                                AbstractPolyhedralMesh<M,V,E,F,P> & m_out);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class M, class V, class E, class F, class P>
CINO_INLINE
void subdivision_midpoint(const AbstractPolyhedralMesh<M,V,E,F,P> & m_in,
                                AbstractPolyhedralMesh<M,V,E,F,P> & m_out,
                                std::map<unsigned int,unsigned int>               & edge_verts,
                                std::map<unsigned int,unsigned int>               & face_verts,
                                std::map<unsigned int,unsigned int>               & poly_verts);
}

#include "subdivision_midpoint.tpp"

#endif // CINO_SUBDIVISION_MIDPOINT_H
