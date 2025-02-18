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
#ifndef CINO_POISSON_SAMPLING
#define CINO_POISSON_SAMPLING

#include <cinolib/cino_inline.h>
#include <sys/types.h>
#include <vector>

namespace cinolib
{

/* This is almost a 1:1 copy of the reference implementation of the Fast Poisson disk sampling in arbitrary
 * dimensions released by Robert Bridson (https://www.cs.ubc.ca/~rbridson/download/curlnoise.tar.gz). See:
 *
 * Fast Poisson Disk Sampling in Arbitrary Dimensions
 * Robert Bridson
 * SIGGRAPH Technical Sketch, 2007
*/

template<unsigned int Dim, class Point>
CINO_INLINE
void Poisson_sampling(const double         radius,
                      const Point          min,
                      const Point          max,
                      std::vector<Point> & samples,
                      unsigned int                 seed=0,
                      const int            max_attempts=30);

}

#include "Poisson_sampling.tpp"

#endif // CINO_POISSON_SAMPLING
