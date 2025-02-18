#ifndef DRAWABLE_TRIANGLE_SOUP_H
#define DRAWABLE_TRIANGLE_SOUP_H

#include <cinolib/drawable_object.h>
#include <cinolib/gl/draw_lines_tris.h>

namespace cinolib
{

class DrawableTriangleSoup : public DrawableObject
{
    public:

        DrawableTriangleSoup(const std::vector<double> & coords,
                             const std::vector<unsigned int>   & tris,
                             const Color               & poly_color = Color::WHITE(),
                             const Color               & edge_color = Color::BLACK());

        ~DrawableTriangleSoup(){}

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void draw(const float scene_size=1) const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        ObjectType object_type()  const { return DRAWABLE_TRIMESH; }
        vec3d      scene_center() const { return vec3d{0,0,0};     } // TODO
        float      scene_radius() const { return 0.0;              } // TODO

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        RenderData drawlist;
};

}

#ifndef  CINO_STATIC_LIB
#include "drawable_triangle_soup.cpp"
#endif

#endif // DRAWABLE_TRIANGLE_SOUP_H
