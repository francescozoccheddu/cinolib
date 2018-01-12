/*********************************************************************************
*  Copyright(C) 2016: Marco Livesu                                               *
*  All rights reserved.                                                          *
*                                                                                *
*  This file is part of CinoLib                                                  *
*                                                                                *
*  CinoLib is dual-licensed:                                                     *
*                                                                                *
*   - For non-commercial use you can redistribute it and/or modify it under the  *
*     terms of the GNU General Public License as published by the Free Software  *
*     Foundation; either version 3 of the License, or (at your option) any later *
*     version.                                                                   *
*                                                                                *
*   - If you wish to use it as part of a commercial software, a proper agreement *
*     with the Author(s) must be reached, based on a proper licensing contract.  *
*                                                                                *
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE       *
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.     *
*                                                                                *
*  Author(s):                                                                    *
*                                                                                *
*     Marco Livesu (marco.livesu@gmail.com)                                      *
*     http://pers.ge.imati.cnr.it/livesu/                                        *
*                                                                                *
*     Italian National Research Council (CNR)                                    *
*     Institute for Applied Mathematics and Information Technologies (IMATI)     *
*     Via de Marini, 6                                                           *
*     16149 Genoa,                                                               *
*     Italy                                                                      *
**********************************************************************************/
#ifndef CINO_DRAWABLE_VECTOR_FIELD_H
#define CINO_DRAWABLE_VECTOR_FIELD_H

#include <cinolib/vector_field.h>
#include <cinolib/drawable_object.h>
#include <cinolib/geometry/vec3.h>
#include <cinolib/color.h>
#include <cinolib/gl/draw_arrow.h>

namespace cinolib
{

template <class Mesh>
class DrawableVectorField : public VectorField, public DrawableObject
{
    public:

        explicit DrawableVectorField()
        {
            arrow_color = Color::RED();
            arrow_size  = 0.5;
        }

        explicit DrawableVectorField(const Mesh & m) : VectorField(m.num_polys())
        {
            m_ptr = &m;
            set_arrow_color(Color::RED());
            set_arrow_size(0.5);
        }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        ObjectType object_type() const { return DRAWABLE_VECTOR_FIELD; }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void draw(const float) const
        {
            if (m_ptr)
            {
                for(uint eid=0; eid<m_ptr->num_polys(); ++eid)
                {
                    vec3d base = m_ptr->poly_centroid(eid);
                    vec3d tip  = base + arrow_length * vec_at(eid);

                    arrow<vec3d>(base, tip, arrow_thicknes, arrow_color.rgba);
                }
            }
        }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        vec3d scene_center() const { return vec3d(); }
        float scene_radius() const { return 0.0;     }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void set_arrow_color(const Color & c) { arrow_color = c; }

        void set_arrow_size(float s)
        {
            if (m_ptr)
            {                
                arrow_size     = s;
                arrow_length   = m_ptr->edge_avg_length() * s;
                arrow_thicknes = arrow_length * 0.1;
            }
        }

        Color get_arrow_color() const { return arrow_color; }
        float get_arrow_size()  const { return arrow_size;  }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        // for more info, see:
        // http://eigen.tuxfamily.org/dox/TopicCustomizingEigen.html
        //
        // This method allows you to assign Eigen expressions to MyVectorType
        //
        template<typename OtherDerived>
        DrawableVectorField & operator= (const Eigen::MatrixBase<OtherDerived>& other)
        {
            this->Eigen::VectorXd::operator=(other);
            return *this;
        }
        //
        // This constructor allows you to construct MyVectorType from Eigen expressions
        //
        template<typename OtherDerived>
        DrawableVectorField(const Eigen::MatrixBase<OtherDerived>& other) : Eigen::VectorXd(other) {}

    private:

        const Mesh *m_ptr;

        float arrow_size;
        float arrow_length;
        float arrow_thicknes;
        Color arrow_color;
};

}

#endif // CINO_DRAWABLE_VECTOR_FIELD_H
