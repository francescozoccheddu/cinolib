/********************************************************************************
*  This file is part of CinoLib                                                 *
*  Copyright(C) 2021: Marco Livesu                                              *
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
#ifndef CINO_VEC_H
#define CINO_VEC_H

#include <cinolib/geometry/vec_mat.h>
#include <initializer_list>

namespace cinolib
{

/*
 * Cinolib implementation of (column) vectors.
 *
 * This class is a wrapper for (a subset of) basic linear algebra operations for raw C arrays.
 * The full set of per vector and per matrix functionalities can be directly accessed through
 * the original methods, defined in:
 *
 *    cinolib/geometry/vec_utils.h
 *    cinolib/geometry/mat_utils.h
*/

template<uint d, class T>
class vec : public vec_mat<d,1,T>
{
    public:

        explicit vec();
        explicit vec(const std::initializer_list<T> & il);
        explicit vec(const T & scalar);

        // Specialized (faster?) for R^2, R^3 and R^4 ::::::::::::::::::::::::::::

        explicit vec(const T & v0, const T & v1);
        explicit vec(const T & v0, const T & v1, const T & v2);
        explicit vec(const T & v0, const T & v1, const T & v2, const T & v3);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        const T * ptr() const { return this->ptr_vec(); }
              T * ptr()       { return this->ptr_vec(); }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        // Specialized Cartesian XYZ access
        const T & x() const { return this->vec[0]; }
              T & x()       { return this->vec[0]; }
        const T & y() const { return this->vec[1]; }
              T & y()       { return this->vec[1]; }
        const T & z() const { return this->vec[2]; }
              T & z()       { return this->vec[2]; }

        // Specialized Parametric UVW access
        const T & u() const { return this->vec[0]; }
              T & u()       { return this->vec[0]; }
        const T & v() const { return this->vec[1]; }
              T & v()       { return this->vec[1]; }
        const T & w() const { return this->vec[2]; }
              T & w()       { return this->vec[2]; }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        T      length_sqrd() const;
        double length     () const;
        double dist       (const vec<d,T> & v) const;
        T      dist_sqrd  (const vec<d,T> & v) const;
        T      normalize  ();
};

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::::::::: EXTERNAL BINARY OPERATORS (FOR MORE READABLE CODE) ::::::::::
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<uint d, class T> CINO_INLINE T        dot      (const vec<d,T> & v0, const vec<d,T> & v1);
template<        class T> CINO_INLINE vec<3,T> cross    (const vec<3,T> & v0, const vec<3,T> & v1);
template<uint d, class T> CINO_INLINE T        angle_deg(const vec<d,T> & v0, const vec<d,T> & v1, const bool unit_length = false);
template<uint d, class T> CINO_INLINE T        angle_rad(const vec<d,T> & v0, const vec<d,T> & v1, const bool unit_length = false);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

// useful types to have
//typedef vec<double,2> vec2d;
//typedef vec<float,2>  vec2f;
//typedef vec<int,2>    vec2i;
//typedef vec<double,3> vec3d;
//typedef vec<float,3>  vec3f;
//typedef vec<int,3>    vec3i;
//typedef vec<double,4> vec4d;
//typedef vec<float,4>  vec4f;
//typedef vec<int,4>    vec4i;

}

#ifndef  CINO_STATIC_LIB
#include "vec.cpp"
#endif

#endif // CINO_VEC_H
