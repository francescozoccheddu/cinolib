/********************************************************************************
*  This file is part of CinoLib                                                 *
*  Copyright(C) 2022: Marco Livesu                                              *
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
#ifndef CINO_QUAD_UTILS_H
#define CINO_QUAD_UTILS_H

#include <cinolib/geometry/vec_mat.h>

namespace cinolib
{

/* Given a quad Q(q0,q1,q2,q3) and a point p inside it,
 * finds the unique barycentric coordinates of p w.r.t. Q.
 * These are the weights w such that
 *
 *    p = w[0]*q0 + w[1]*q1 + w[2]*q2 + w[3]*q3
 *
 * This is done by computing the inverse bilinear coordinates,
 * as detailed in Sec. 3 of the paper
 *
 *    Generalized barycentric coordinates and applications
 *    Michael S. Floater
 *    Acta Numerica, 2016
*/
template<class T>
CINO_INLINE
void quad_barycentric_coords(const mat<3,1,T> & q0,
                             const mat<3,1,T> & q1,
                             const mat<3,1,T> & q2,
                             const mat<3,1,T> & q3,
                             const mat<3,1,T> & p,
                                   mat<4,1,T> & wgt);
}

#include "quad_utils.tpp"

#endif // CINO_QUAD_UTILS_H
