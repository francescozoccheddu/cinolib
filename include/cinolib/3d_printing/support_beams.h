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
#ifndef CINO_SUPPORT_BEAMS_H
#define CINO_SUPPORT_BEAMS_H

#include <cinolib/meshes/trimesh.h>

namespace cinolib
{

/* outputs a support segment that emanates from the centroid
 * of each overhanging triangle down to the building platform
 * or to the upfacing triangle immediately below the overhang.
 * Supporting segments are serialized in a vector of 3D points
*/
template<class M, class V, class E, class P>
CINO_INLINE
void support_beams(const Trimesh<M,V,E,P>                  & m,
                   const vec3d                             & build_dir,
                   const std::vector<std::pair<unsigned int,unsigned int>> & overhangs,
                   const float                               floor,
                         std::vector<vec3d>                & beams);
}

#ifndef  CINO_STATIC_LIB
#include "support_beams.cpp"
#endif

#endif // CINO_SUPPORT_BEAMS_H
