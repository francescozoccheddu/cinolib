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
#include <cinolib/scalar_field.h>
#include <cinolib/cino_inline.h>
#include <cinolib/min_max_inf.h>
#include <cinolib/clamp.h>
#include <fstream>

namespace cinolib
{

template<class Mesh>
CINO_INLINE
void ScalarField::copy_to_mesh(Mesh & m, const int tex_coord) const
{
    unsigned int nv = m.num_verts();

    if((unsigned int)rows() == nv)
    {
        for(unsigned int vid=0; vid<nv; ++vid)
        {
            switch (tex_coord)
            {
                case U_param : m.vert_data(vid).uvw[0] = (*this)[vid]; break;
                case V_param : m.vert_data(vid).uvw[1] = (*this)[vid]; break;
                case W_param : m.vert_data(vid).uvw[2] = (*this)[vid]; break;
                default: assert(false);
            }
        }
    }
    else if((unsigned int)rows() == nv+nv)
    {        
        for(unsigned int vid=0; vid<nv; ++vid)
        {
            switch (tex_coord)
            {
                case UV_param : m.vert_data(vid).uvw[0] = (*this)[vid];
                                m.vert_data(vid).uvw[1] = (*this)[vid + nv];
                                break;
                case UW_param : m.vert_data(vid).uvw[0] = (*this)[vid];
                                m.vert_data(vid).uvw[2] = (*this)[vid + nv];
                                break;
                case VW_param : m.vert_data(vid).uvw[1] = (*this)[vid];
                                m.vert_data(vid).uvw[2] = (*this)[vid + nv];
                                break;
                default: assert(false);
            }
        }
    }
    else if((unsigned int)rows() == nv+nv+nv)
    {
        assert(tex_coord == UVW_param);
        unsigned int nv2 = nv*2;
        for(unsigned int vid=0; vid<nv; ++vid)
        {
            m.vert_data(vid).uvw[0] = (*this)[vid];
            m.vert_data(vid).uvw[1] = (*this)[vid + nv];
            m.vert_data(vid).uvw[2] = (*this)[vid + nv2];
        }
    }
    else assert(false);
}

// for more info, see:
// http://eigen.tuxfamily.org/dox/TopicCustomizingEigen.html
//
// This method allows you to assign Eigen expressions to ScalarField
//
template<typename OtherDerived>
CINO_INLINE
ScalarField & ScalarField::operator= (const Eigen::MatrixBase<OtherDerived>& other)
{
    this->Eigen::VectorXd::operator=(other);
    return *this;
}
//
// This constructor allows you to construct ScalarField from Eigen expressions
//
template<typename OtherDerived>
CINO_INLINE
ScalarField::ScalarField(const Eigen::MatrixBase<OtherDerived>& other) : Eigen::VectorXd(other) {}

}
