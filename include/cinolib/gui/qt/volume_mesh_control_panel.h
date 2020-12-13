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
#ifndef CINO_VOLUME_MESH_CONTROL_PANEL_H
#define CINO_VOLUME_MESH_CONTROL_PANEL_H

#ifdef CINOLIB_USES_QT

#include <cinolib/scalar_field.h>
#include <cinolib/drawable_vector_field.h>
#include <cinolib/drawable_isosurface.h>
#include <cinolib/drawable_segment_soup.h>
#include <cinolib/gui/qt/glcanvas.h>

#include <QWidget>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QSlider>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QColorDialog>

namespace cinolib
{

template<class Mesh>
class VolumeMeshControlPanel
{
    typedef typename Mesh::M_type M;
    typedef typename Mesh::V_type V;
    typedef typename Mesh::E_type E;
    typedef typename Mesh::F_type F;
    typedef typename Mesh::P_type P;

    enum
    {
        OUTSIDE = 0,
        INSIDE  = 1
    };

    public:

        VolumeMeshControlPanel(Mesh *m, GLcanvas *canvas, QWidget *parent = NULL);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void show();

    protected:

        void connect();
        void set_title();
        void set_isosurface();
        void set_tex1d(const int in_out);
        void set_tex2d(const int in_out);
        void set_slice();

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        Mesh                          *m;
        DrawableIsosurface<M,V,E,F,P>  isosurface;
        DrawableVectorField            gradient;
        DrawableSegmentSoup            face_normals;
        DrawableSegmentSoup            vert_normals;
        GLcanvas                      *canvas;
        QWidget                       *widget;
        QHBoxLayout                   *global_layout;
        QFont                          global_font;
        //
        QPushButton                   *but_load;
        QPushButton                   *but_save;
        QCheckBox                     *cb_show_mesh;
        QRadioButton                  *rb_point_shading;
        QRadioButton                  *rb_flat_shading;
        QRadioButton                  *rb_smooth_shading;
        //
        QRadioButton                  *rb_out_vert_color;
        QRadioButton                  *rb_out_face_color;
        QRadioButton                  *rb_out_poly_color;
        QRadioButton                  *rb_out_qual_color;
        QRadioButton                  *rb_out_tex1D;
        QRadioButton                  *rb_out_tex2D;
        QPushButton                   *but_set_out_vert_color;
        QPushButton                   *but_set_out_face_color;
        QPushButton                   *but_set_out_poly_color;
        QPushButton                   *but_load_out_tex2d;
        std::string                    out_tex2d_filename;
        QComboBox                     *cb_out_tex1D_type;
        QComboBox                     *cb_out_tex2D_type;
        QSlider                       *sl_out_tex2D_density;
        //
        QRadioButton                  *rb_in_vert_color;
        QRadioButton                  *rb_in_face_color;
        QRadioButton                  *rb_in_poly_color;
        QRadioButton                  *rb_in_qual_color;
        QRadioButton                  *rb_in_tex1D;
        QRadioButton                  *rb_in_tex2D;
        QPushButton                   *but_set_in_vert_color;
        QPushButton                   *but_set_in_face_color;
        QPushButton                   *but_set_in_poly_color;
        QPushButton                   *but_load_in_tex2d;
        std::string                    in_tex2d_filename;
        QComboBox                     *cb_in_tex1D_type;
        QComboBox                     *cb_in_tex2D_type;
        QSlider                       *sl_in_tex2D_density;
        //
        QCheckBox                     *cb_out_wireframe;
        QSlider                       *sl_out_wireframe_width;
        QSlider                       *sl_out_wireframe_alpha;
        QPushButton                   *but_out_wireframe_color;
        QCheckBox                     *cb_in_wireframe;
        QSlider                       *sl_in_wireframe_width;
        QSlider                       *sl_in_wireframe_alpha;
        QPushButton                   *but_in_wireframe_color;
        //        
        QCheckBox                     *cb_isosurface;
        QSlider                       *sl_isovalue;
        QPushButton                   *but_isosurface_color;
        QPushButton                   *but_isosurface_save;
        QPushButton                   *but_serialize_field;
        QPushButton                   *but_deserialize_field;
        //
        QCheckBox                     *cb_gradient;
        QSlider                       *sl_gradient_size;
        QPushButton                   *but_gradient_color;
        QPushButton                   *but_gradient_serialize;
        QPushButton                   *but_gradient_deserialize;
        //
        QSlider                       *sl_slice_x;
        QSlider                       *sl_slice_y;
        QSlider                       *sl_slice_z;
        QSlider                       *sl_slice_l;
        QSlider                       *sl_slice_q;
        QCheckBox                     *cb_slice_flip_x;
        QCheckBox                     *cb_slice_flip_y;
        QCheckBox                     *cb_slice_flip_z;
        QCheckBox                     *cb_slice_flip_q;
        QCheckBox                     *cb_slice_flip_l;
        QRadioButton                  *rb_slice_AND;
        QRadioButton                  *rb_slice_OR;
        QPushButton                   *but_slice_reset;
        //
        QCheckBox                     *cb_marked_edges;
        QPushButton                   *but_marked_edges_color;
        QSlider                       *sl_marked_edges_width;
        //
        QCheckBox                     *cb_marked_faces;
        QPushButton                   *but_marked_faces_color;
        //
        QSlider                       *sl_ambient_occlusion;
        QPushButton                   *but_compute_AO;
        //
        QComboBox                     *cb_actions;
        //
        QCheckBox                     *cb_face_normals;
        QCheckBox                     *cb_vert_normals;
};

}

#ifndef  CINO_STATIC_LIB
#include "volume_mesh_control_panel.cpp"
#endif

#endif // #ifdef CINOLIB_USES_QT

#endif // CINO_VOLUME_MESH_CONTROL_PANEL_H
