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
#include <cinolib/geometry/triangle_utils.h>
#include <cinolib/standard_elements_tables.h>
#include <cinolib/min_max_inf.h>
#include <set>

namespace cinolib
{

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template <class vec>
CINO_INLINE
double triangle_area(const vec & A, const vec & B, const vec & C)
{
    return (0.5 * (B-A).cross(C-A).norm());
}

// http://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
//
// NOTE: the current implementation requires 21 multiplications and 2 divisions.
// A good alternative could be the method proposed in:
//
//   Computing the Barycentric Coordinates of a Projected Point
//   Wolfgang Heidrich
//   Journal of Graphics, GPU, and Game Tools, 2011
//
// which takes 27 multiplications (3 cross, 3 dot products), and
// combines together projection of the point in the triangle's plane
// and computation of barycentric coordinates
//
template <class vec>
CINO_INLINE
void triangle_barycentric_coords(const vec & A,
                                 const vec & B,
                                 const vec & C,
                                 const vec & P,
                                 double wgts[])
{
    vec    u    = B - A;
    vec    v    = C - A;
    vec    w    = P - A;
    double d00  = u.dot(u);
    double d01  = u.dot(v);
    double d11  = v.dot(v);
    double d20  = w.dot(u);
    double d21  = w.dot(v);
    double den  = d00 * d11 - d01 * d01;

    if(den==0) // degenerate
    {
        wgts[0] = inf_double;
        wgts[1] = inf_double;
        wgts[2] = inf_double;
        return;
    }

    wgts[2] = (d00 * d21 - d01 * d20) / den; assert(!std::isnan(wgts[2]));
    wgts[1] = (d11 * d20 - d01 * d21) / den; assert(!std::isnan(wgts[1]));
    wgts[0] = 1.0f - wgts[1] - wgts[2];      assert(!std::isnan(wgts[0]));
}

}

