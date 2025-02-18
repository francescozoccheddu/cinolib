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
*     Luca Pitzalis (lucapizza@gmail.com)                                       *
*     University of Cagliari                                                    *
*                                                                               *
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
#ifndef CINO_HEX_TRANSITION_INSTALL_H
#define CINO_HEX_TRANSITION_INSTALL_H

#include <cinolib/meshes/meshes.h>

namespace cinolib
{

/* This function installs the 8+5 transitions defined in cinolib/hex_transition_schemes.h,
 * which allow to transform a strongly or weakly balanced grid into a pure hexahedral mesh.
 *
 * Transition_verts is a vector having as many entries as the number of grid vertices, and
 * is set to true in correspondence of the vertices where transition schemes must be applied.
 */

template<class M, class V, class E, class F, class P>
CINO_INLINE
void hex_transition_install(const Polyhedralmesh<M,V,E,F,P> & m_in,
                            const std::vector<bool>         & transition_verts,
                                  Polyhedralmesh<M,V,E,F,P> & m_out);
}

#include "hex_transition_install.tpp"

#endif // CINO_HEX_TRANSITION_INSTALL_H
