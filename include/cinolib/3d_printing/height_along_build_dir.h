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
#ifndef CINO_HEIGHT_ALONG_BUILD_DIR_H
#define CINO_HEIGHT_ALONG_BUILD_DIR_H

#include <cinolib/meshes/trimesh.h>

namespace cinolib
{

// returns the height of mesh m along a given build direction
// (e.g. for 3D printing or 3 axis CNC milling). Height is
// computed by projecting all mesh vertices onto the build
// direction, and then measuring the 1D distance among the
// extrema. The smallest projection is also stored in the
// output variable "floor" and it ideally represents the
// level of the building platform. This quantity is used
// by other functions in cinolib, e.g. to estimate the
// volume of support structures
//
template<class M, class V, class E, class P>
CINO_INLINE
float height_along_build_dir(const Trimesh<M,V,E,P> & m,
                             const vec3d            & build_dir, // assumed to be unit length!
                                   float            & floor);

}

#include "height_along_build_dir.tpp"

#endif // CINO_HEIGHT_ALONG_BUILD_DIR_H
