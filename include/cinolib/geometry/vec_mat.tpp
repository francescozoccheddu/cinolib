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
#include <cinolib/geometry/vec_mat.h>
#include <cinolib/geometry/vec_mat_utils.h>
#include <cassert>

namespace cinolib
{

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r, c, T> operator*(const T& scalar, const mat<r, c, T>& m)
{
    mat<r, c, T> res;
    vec_times<r* c, T>(m._vec, scalar, res._vec);
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
std::ostream & operator<< (std::ostream& out, const mat<r,c,T>& mat)
{
    for(unsigned int i=0; i<r; ++i)
        for(unsigned int j=0; j<c; ++j)
        {
            if(i>0 && c>1 && j%c==0) out << "\n";
            out << mat(i,j) << " ";
        }
    return out;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
std::istream& operator>> (std::istream& in, mat<r, c, T>& mat)
{
    for (unsigned int i = 0; i < r; ++i)
        for (unsigned int j = 0; j < c; ++j)
        {
            in >> mat(i, j);
        }
    return in;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
constexpr mat<r,c,T>::mat(const std::initializer_list<mat<r,1,T>> & il) : _vec{}
{
    assert(il.size() <= c);
    std::size_t ci{};
    for (const mat<r, 1, T>& col : il)
    {
        for (std::size_t ri{}; ri < r; ri++)
        {
            _mat[ri][ci] = col._vec[ri];
        }
        ci++;
    }
    if (ci < c)
    {
        const mat<r, 1, T> last{ il.size() ? *(il.end() - 1) : mat<r, 1, T>{} };
        while (ci < c)
        {
            for (std::size_t ri{}; ri < r; ri++)
            {
                _mat[ri][ci] = last[ri];
            }
            ci++;
        }
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
constexpr mat<r, c, T>::mat(const std::initializer_list<T>& il) : _vec{}
{
    assert(il.size() <= r * c);
    std::size_t i{};
    for (const T v : il)
    {
        _vec[i++] = v;
    }
    const T last{ il.size() ? *(il.end()-1) : T{} };
    while (i < r * c)
    {
        _vec[i++] = last;
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
constexpr mat<r, c, T>::mat(const T* values) : _vec{}
{
    for (std::size_t i{}; i < r * c; i++)
    {
        _vec[i] = values[i];
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
constexpr mat<r, c, T>::mat() 
{
    for (std::size_t i{}; i < r * c; i++)
    {
        _vec[i] = T{};
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> mat<r,c,T>::ZERO()
{
    return mat<r, c, T>{};
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> mat<r,c,T>::ONES()
{
    return mat<r, c, T>{(T)1};
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> mat<r,c,T>::DIAG(const T scalar)
{
    assert(r==c);
    mat<r,c,T> m;
    mat_set_diag<r,T>(m._mat, scalar);
    return m;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> mat<r,c,T>::DIAG(const mat<r,1,T> & diag)
{
    assert(r==c);
    mat<r,c,T> m;
    mat_set_diag<r,T>(m._mat, diag._vec);
    return m;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> mat<r,c,T>::ROT_2D(const T angle_rad)
{
    assert(r==c);
    mat<r,c,T> m;
    mat_set_rot_2d<r,T>(m._mat, angle_rad);
    return m;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> mat<r,c,T>::HOMOGENEOUS(const mat<r-1,c-1,T>& non_homogeneous)
{
    static_assert(r == c, "matrix must be square");
    mat<r, c, T> m;
    mat_set_diag<r, T>(m._mat, 1);
    for (unsigned int ri{}; ri < r - 1; ri++)
    {
        for (unsigned int ci{}; ci < c - 1; ci++)
        {
            m(ri,ci) = non_homogeneous(ri,ci);
        }
    }
    return m;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> mat<r,c,T>::ROT_3D(const mat<r,1,T> & axis, const T angle_rad)
{
    assert(r==c);
    mat<r,c,T> m;
    mat_set_rot_3d<r,T>(m._mat, angle_rad, axis._vec);
    return m;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> mat<r,c,T>::TRANS(const mat<r-1,1,T> & tx)
{
    assert(r==c);
    mat<r,c,T> m;
    mat_set_trans<r,T>(m._mat, tx._vec);
    return m;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<c,1,T> mat<r,c,T>::row(const unsigned int i) const
{
    mat<c,1,T> row;
    mat_row<r,c,T>(_mat, i, row._vec);
    return row;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,1,T> mat<r,c,T>::col(const unsigned int i) const
{
    mat<r,1,T> col;
    mat_col<r,c,T>(_mat, i, col._vec);
    return col;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,1,T> mat<r,c,T>::diag() const
{
    assert(r==c);
    mat<r,1,T> diag;
    mat_diag<r,T>(_mat, diag._vec);
    return diag;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
void mat<r,c,T>::set_row(const unsigned int i, const mat<c,1,T> & row)
{
    mat_set_row<r,c,T>(_mat, i, row._vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
constexpr void mat<r,c,T>::set_col(const unsigned int i, const mat<r,1,T> & col )
{
    mat_set_col<r,c,T>(_mat, i, col._vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
void mat<r,c,T>::set_diag(const mat<r,1,T> & diag)
{
    mat_set_diag<r,c,T>(_mat, diag._vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
T mat<r,c,T>::det() const
{
    assert(r==c);
    return mat_det<r,T>(_mat);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
T mat<r,c,T>::trace() const
{
    assert(r==c);
    return mat_trace<r,T>(_mat);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<c,r,T> mat<r,c,T>::transpose() const
{
    mat<c,r,T> res;
    mat_transpose<r,c,T>(_mat, res._mat);
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> mat<r,c,T>::inverse() const
{
    assert(r==c);
    mat<r,c,T> res;
    mat_inverse<r,T>(_mat, res._mat);
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
void mat<r,c,T>::eigenvectors(mat<r,c,T> & evec) const
{
    mat_eigenvec<r,c,T>(_mat, evec._mat);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
void mat<r,c,T>::eigenvalues(mat<r,1,T> & eval) const
{
    mat_eigenval<r,c,T>(_mat, eval._vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
void mat<r,c,T>::eigendecomp(mat<r,1,T> & eval, mat<r,c,T> & evec) const
{
    mat_eigendec<r,c,T>(_mat, eval._vec, evec._mat);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
void mat<r,c,T>::SVD(mat<r,c,T> & U, mat<r,1,T> & S, mat<r,c,T> & V) const
{
    mat_svd<r,c,T>(_mat, U._mat, S._vec, V._mat);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
void mat<r,c,T>::SSVD(mat<r,c,T> & U, mat<r,1,T> & S, mat<r,c,T> & V) const
{
    mat_ssvd<r,c,T>(_mat, U._mat, S._vec, V._mat);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,1,T> mat<r,c,T>::solve(const mat<c,1,T> & b)
{
    mat<r,1,T> res;
    mat_solve_Cramer(_mat, b._vec, res._vec);
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
const T & mat<r,c,T>::operator[](const unsigned int i) const
{
    return _vec[i];
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
T & mat<r,c,T>::operator[](const unsigned int i)
{
    return _vec[i];
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
const T & mat<r,c,T>::operator()(const unsigned int i, const unsigned int j) const
{
    return _mat[i][j];
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
T & mat<r,c,T>::operator()(const unsigned int i, const unsigned int j)
{
    return _mat[i][j];
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T>mat<r,c,T>::operator-() const
{
    mat<r,c,T> res;
    vec_minus<r*c,T>(_vec, res._vec);
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T>mat<r,c,T>::operator-(const mat<r,c,T> & op) const
{
    mat<r,c,T> res;
    vec_minus<r*c,T>(_vec, op._vec, res._vec);
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> mat<r,c,T>::operator+(const mat<r,c,T> & op) const
{
    mat<r,c,T> res;
    vec_plus<r*c,T>(_vec, op._vec, res._vec);
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> & mat<r,c,T>::operator+=(const mat<r,c,T> & op)
{
    vec_plus<r*c,T>(_vec, op._vec, _vec);
    return *this;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> & mat<r,c,T>::operator-=(const mat<r,c,T> & op)
{
    vec_minus<r*c,T>(_vec, op._vec, _vec);
    return *this;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> mat<r,c,T>::operator*(const T & scalar) const
{
    mat<r,c,T> res;
    vec_times<r*c,T>(_vec, scalar, res._vec);
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
template<unsigned int c2>
CINO_INLINE
mat<r,c2,T> mat<r,c,T>::operator*(const mat<c,c2,T> & op) const
{
    mat<r,c2,T> res;
    mat_times<r,c,c2,T>(_mat, op._mat, res._mat);
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> mat<r,c,T>::operator/(const T & scalar) const
{
    mat<r,c,T> res;
    vec_divide<r*c,T>(_vec, scalar, res._vec);
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> & mat<r,c,T>::operator*=(const T & scalar)
{
    vec_times<r*c,T>(_vec, scalar, _vec);
    return *this;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> & mat<r,c,T>::operator/=(const T & scalar)
{
    vec_divide<r*c,T>(_vec, scalar, _vec);
    return *this;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
bool mat<r,c,T>::operator==(const mat<r,c,T> & op) const
{
    return vec_equals<r*c,T>(_vec, op._vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
bool mat<r,c,T>::operator<(const mat<r,c,T> & op) const
{
    return vec_less<r*c,T>(_vec, op._vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
double mat<r,c,T>::norm() const
{
    return vec_norm<r*c,T>(_vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
T mat<r,c,T>::norm_sqrd() const
{
    return vec_norm_sqrd<r*c,T>(_vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
double mat<r,c,T>::norm_p(const float p) const
{
    return vec_norm_p<r*c,T>(_vec, p);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
double mat<r,c,T>::dist(const mat<r,c,T> & v) const
{
    return vec_dist<r*c,T>(_vec, v._vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
T mat<r,c,T>::dist_sqrd(const mat<r,c,T> & v) const
{
    return vec_dist_sqrd<r*c,T>(_vec, v._vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> & mat<r,c,T>::rotate(const mat<3,1,T> & axis, const T angle_rad)
{
    mat3d R = mat3d::ROT_3D(axis,angle_rad);
    (*this) = R*(*this);
    return *this;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> mat<r, c, T>::normalized() const
{
    mat clone{*this};
    clone.normalize();
    return clone;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
double mat<r,c,T>::normalize()
{
    return vec_normalize<r*c,T>(_vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
T mat<r,c,T>::min_entry() const
{
    return vec_min_entry<r*c,T>(_vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
T mat<r,c,T>::max_entry() const
{
    return vec_max_entry<r*c,T>(_vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> mat<r,c,T>::min(const mat<r,c,T> & v) const
{
    mat<r,c,T> res;
    vec_min<r*c,T>(_vec, v._vec, res._vec);
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r,c,T> mat<r,c,T>::max(const mat<r,c,T> & v) const
{
    mat<r,c,T> res;
    vec_max<r*c,T>(_vec, v._vec, res._vec);
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
void mat<r,c,T>::clamp(const T min, const T max)
{
    vec_clamp<r*c,T>(_vec, min, max);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
void mat<r,c,T>::swap(const unsigned int i, const unsigned int j, const unsigned int k, const unsigned int l)
{
    mat_swap<r,c,T>(_mat, i, j, k, l);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

// e.g. from cartesian to homogeneous coords
template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r+1,c,T> mat<r,c,T>::add_coord(const T & val) const
{
    assert(c==1);
    mat<r+1,c,T> res;
    vec_copy<r>(_vec,res._vec);
    res[r] = val;
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

// e.g. from homogeneous to cartesian coords
template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<r-1,c,T> mat<r,c,T>::rem_coord() const
{
    assert(c==1);
    assert(r>=2);
    mat<r-1,c,T> res;
    vec_copy<r-1>(_vec,res._vec);
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
bool mat<r,c,T>::is_null() const
{
    return vec_is_null<r*c,T>(_vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
bool mat<r,c,T>::is_nan() const
{
    return vec_is_nan<r*c,T>(_vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
bool mat<r,c,T>::is_inf() const
{
    return vec_is_inf<r*c,T>(_vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
bool mat<r,c,T>::is_deg() const // either null, nan or inf
{
    return vec_is_deg<r*c,T>(_vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
bool mat<r, c, T>::is_finite() const // either null, nan or inf
{
    return vec_is_finite<r* c, T>(_vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
template<typename TOther>
CINO_INLINE
typename std::enable_if<std::is_same<TOther, T>::value, const mat<r, c, TOther>&>::type mat<r, c, T>::cast() const
{
    return *this;
}

template<unsigned int r, unsigned int c, class T>
template<typename TOther>
CINO_INLINE
typename std::enable_if<!std::is_same<TOther, T>::value, mat<r, c, TOther>>::type mat<r, c, T>::cast() const
{
    mat<r, c, TOther> other;
    for (std::size_t i{}; i < r * c; i++)
    {
        other._vec[i] = static_cast<TOther>(_vec[i]);
    }
    return other;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
mat<3,1,T> mat<r,c,T>::cross(const mat<3,1,T> & v) const
{
    assert(r==3 && c==1);
    mat<3,1,T> res;
    vec_cross<T>(_vec, v._vec, res._vec);
    return res;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
T mat<r,c,T>::dot(const mat<r,c,T> & v) const
{
    assert(r==1 || c==1);
    return vec_dot<r,T>(_vec, v._vec);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
T mat<r,c,T>::angle_deg(const mat<r,c,T> & v, const bool normalize) const
{
    assert(c==1);
    return vec_angle_deg<r,T>(_vec, v._vec, normalize);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<unsigned int r, unsigned int c, class T>
CINO_INLINE
T mat<r,c,T>::angle_rad(const mat<r,c,T> & v, const bool normalize) const
{
    assert(c==1);
    return vec_angle_rad<r,T>(_vec, v._vec, normalize);
}

}
