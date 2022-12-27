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
#ifndef CINO_VEC_MAT_H
#define CINO_VEC_MAT_H

#include <ostream>
#include <istream>
#include <cinolib/cino_inline.h>
#include <cinolib/symbols.h>
#include <type_traits>

namespace cinolib
{

template<unsigned int r, unsigned int c, class T>
class mat final
{
    public:

        union
        {
            T _vec[r*c];  // 1D view
            T _mat[r][c]; // 2D view
        };

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        constexpr mat(const std::initializer_list<mat<r,1,T>> & il);
        constexpr mat(const std::initializer_list<T> & il);
        constexpr mat(const T * values);
        constexpr mat();

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        static mat<r,c,T> ZERO  ();
        static mat<r,c,T> ONES  ();
        static mat<r,c,T> DIAG  (const T scalar);
        static mat<r,c,T> DIAG  (const mat<r,  1,T> & diag);
        static mat<r,c,T> TRANS (const mat<r-1,1,T> & tx);
        static mat<r,c,T> ROT_3D(const mat<r,  1,T> & axis, const T angle_rad);
        static mat<r,c,T> ROT_2D(const T angle_rad);
        static mat<r,c,T> HOMOGENEOUS(const mat<r-1,c-1,T>& non_homogeneous);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        const T * ptr() const { return _vec; }
              T * ptr()       { return _vec; }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        mat<c,1,T> row (const unsigned int i) const;
        mat<r,1,T> col (const unsigned int i) const;
        mat<r,1,T> diag()             const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void set_row (const unsigned int i, const mat<c,1,T> & row);
        constexpr void set_col (const unsigned int i, const mat<r,1,T> & col );
        void set_diag(              const mat<r,1,T> & diag);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        T          det()       const;
        T          trace()     const;
        mat<c,r,T> transpose() const;
        mat<r,c,T> inverse()   const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void eigenvectors(mat<r,c,T> & evec)                    const;
        void eigenvalues (mat<r,1,T> & eval)                    const;
        void eigendecomp (mat<r,1,T> & eval, mat<r,c,T> & evec) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void SVD (mat<r,c,T> & U, mat<r,1,T> & S, mat<r,c,T> & V) const;
        void SSVD(mat<r,c,T> & U, mat<r,1,T> & S, mat<r,c,T> & V) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        mat<r,1,T> solve(const mat<c,1,T> & b);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        const T          & operator[] (const unsigned int i)  const;
              T          & operator[] (const unsigned int i);
        const T          & operator() (const unsigned int i, const unsigned int j)  const;
              T          & operator() (const unsigned int i, const unsigned int j);
              mat<r,c,T>   operator-  ()                            const;
              mat<r,c,T>   operator-  (const mat<r,c,T> & op)       const;
              mat<r,c,T>   operator+  (const mat<r,c,T> & op)       const;
              mat<r,c,T> & operator+= (const mat<r,c,T> & op);
              mat<r,c,T> & operator-= (const mat<r,c,T> & op);
              mat<r,c,T>   operator*  (const T & scalar)            const;
              mat<r,c,T>   operator/  (const T & scalar)            const;
              mat<r,c,T> & operator*= (const T & scalar);
              mat<r,c,T> & operator/= (const T & scalar);
              bool         operator== (const mat<r,c,T> & op)       const;
              bool         operator<  (const mat<r,c,T> & op)       const;

        template<unsigned int c2>
        mat<r,c2,T> operator*(const mat<c,c2,T> & op) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        mat<r,c,T> & rotate(const mat<3,1,T> & axis, const T angle_rad);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        mat normalized() const;

        double normalize();
        double norm     () const;
        T      norm_sqrd() const;
        double norm_p   (const float p) const;
        double dist     (const mat<r,c,T> & v) const;
        T      dist_sqrd(const mat<r,c,T> & v) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        T          min_entry()               const;
        T          max_entry()               const;
        mat<r,c,T> min(const mat<r,c,T> & v) const;
        mat<r,c,T> max(const mat<r,c,T> & v) const;
        void       clamp(const T min, const T max);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void swap(const unsigned int i, const unsigned int j, const unsigned int k, const unsigned int l);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        mat<r+1,c,T> add_coord(const T & val) const; // e.g. from cartesian to homogeneous coords
        mat<r-1,c,T> rem_coord()              const; // e.g. from homogeneous to cartesian coords

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        bool is_null() const;
        bool is_nan()  const;
        bool is_inf()  const;
        bool is_deg()  const; // either null, nan or inf
        bool is_finite()  const; // null, normal or subnormal

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        // Specialized Cartesian XYZ access
        const T & x() const { return _vec[0]; }
              T & x()       { return _vec[0]; }
        const T & y() const { return _vec[1]; }
              T & y()       { return _vec[1]; }
        const T & z() const { return _vec[2]; }
              T & z()       { return _vec[2]; }

        // Specialized Parametric UVW access
        const T & u() const { return _vec[0]; }
              T & u()       { return _vec[0]; }
        const T & v() const { return _vec[1]; }
              T & v()       { return _vec[1]; }
        const T & w() const { return _vec[2]; }
              T & w()       { return _vec[2]; }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        
        // consider moving to 'if constexpr' if cinolib goes to C++17

        template<typename TOther>
        typename std::enable_if<std::is_same<TOther, T>::value, const mat<r, c, TOther>&>::type cast() const;

        template<typename TOther>
        typename std::enable_if<!std::is_same<TOther, T>::value, mat<r, c, TOther>>::type cast() const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        mat<3,1,T> cross(const mat<3,1,T> & v) const;
        T          dot  (const mat<r,c,T> & v) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        T angle_deg(const mat<r,c,T> & v0, const bool normalize = true) const;
        T angle_rad(const mat<r,c,T> & v0, const bool normalize = true) const;
};

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r, c, T> operator*(const T& scalar, const mat<r, c, T>& m);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
std::ostream & operator<< (std::ostream& out, const mat<r,c,T>& mat);


//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
std::istream& operator>> (std::istream& in, mat<r, c, T>& mat);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

// useful types to have

template<unsigned int d, typename T>
using sqmat = mat<d, d, T>;

template<typename T>
using mat4 = sqmat<4, T>;

typedef mat<2,2,double> mat2d;
typedef mat<2,2,float>  mat2f;
typedef mat<2,2,int>    mat2i;

typedef mat<3,3,double> mat3d;
typedef mat<3,3,float>  mat3f;
typedef mat<3,3,int>    mat3i;

typedef mat<4,4,double> mat4d;
typedef mat<4,4,float>  mat4f;
typedef mat<4,4,int>    mat4i;

typedef mat<2,1,double> vec2d;
typedef mat<2,1,float>  vec2f;
typedef mat<2,1,int>    vec2i;

template<unsigned int d, typename T>
using vec = mat<d,1,T>;

template<typename T>
using vec2 = vec<2,T>;

template<typename T>
using vec3 = vec<3, T>;

template<typename T>
using vec4 = vec<4, T>;

typedef mat<3,1,double> vec3d;
typedef mat<3,1,float>  vec3f;
typedef mat<3,1,int>    vec3i;

typedef mat<4,1,double> vec4d;
typedef mat<4,1,float>  vec4f;
typedef mat<4,1,int>    vec4i;

}

#include "vec_mat.tpp"

#endif // CINO_VEC_MAT_H
