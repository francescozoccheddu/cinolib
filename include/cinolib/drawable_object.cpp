#include <cinolib/drawable_object.h>
#include <cinolib/cino_inline.h>
#include <cinolib/gl/draw_lines_tris.h>
#include <cmath>

namespace cinolib
{

	CINO_INLINE DrawableObject::DrawableObject()
		: transform(mat4d::DIAG(1))
	{}

	CINO_INLINE void DrawableObject::draw_transformed(const float scene_size) const
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glMultMatrixd(transform.transpose().ptr());

		draw(scene_size);

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	CINO_INLINE vec3d DrawableObject::scene_center_transformed() const
	{
		return transform * scene_center();
	}

	CINO_INLINE float DrawableObject::scene_radius_transformed() const
	{
		// does not work for shearing transformations
		double maxs{ 0.0 };
		for (unsigned int d{ 0 }; d < 3; d++)
		{
			maxs = std::max(maxs, vec3d{ transform(d, 0),transform(d, 1),transform(d, 2) }.norm());
		}
		return static_cast<float>(maxs * scene_radius());
	}

}