#ifndef CINOLIB_GL_FREE_CAMERA_INCLUDED
#define CINOLIB_GL_FREE_CAMERA_INCLUDED

// TODO (francescozoccheddu) code style and naming convention should match the rest of cinolib
// TODO (francescozoccheddu) documentation should be added

#include <cinolib/geometry/vec_mat.h>
#include <cinolib/deg_rad.h>
#include <string>
#include <ostream>
#include <istream>

namespace cinolib
{

	template<typename TScalar>
	class FreeCamera final
	{

		// Components

	public:

		using vec = vec3<TScalar>;
		using mat = mat<4, 4, TScalar>;

		class Projection final
		{

		public:

			TScalar aspectRatio{ 1 }; // width / height
			TScalar farZ{ 100 }, nearZ{ 1 }; // world units
			TScalar verticalFieldOfView{ 67 }; // angle degrees (perspective) or world units (orthographic)
			bool perspective{ true };

			void setAspect(unsigned int _width, unsigned int _height);

			inline void validate() const;

			inline mat matrix() const; // not cached

			inline std::string serialize() const;
			inline static Projection deserialize(const std::string& _data);

		} projection;

		class View final
		{

		public:

			vec eye{ 0,0,0 };
			vec up{ 0,1,0 }; // does not have to be normalized
			vec forward{ 0,0,1 }; // does not have to be normalized

			inline vec normLeft() const;
			inline vec normRight() const;
			inline vec normDown() const;
			inline vec normUp() const;
			inline vec normBack() const;
			inline vec normForward() const;

			inline void lookAt(const vec& _target);
			inline void rotateAroundPivot(const vec& _axis, TScalar _angle, const vec& _pivot);
			inline void rotateAround(const vec& _axis, TScalar _angle);

			inline void validate() const;

			inline mat matrix() const; // not cached

			inline std::string serialize() const;
			inline static View deserialize(const std::string& _data);

		} view;

		// Matrices

	private:

		mutable mat m_projectionCache, m_viewCache, m_projectionViewCache;
		mutable bool m_projectionDirty{ true }, m_viewDirty{ true }, m_projectionViewDirty{ true };

	public:

		inline void updateProjection();
		inline void updateView();
		inline void updateProjectionAndView();

		inline const mat& projectionMatrix() const; // cached
		inline const mat& viewMatrix() const; // cached
		inline const mat& projectionViewMatrix() const; // cached

		inline std::string serialize() const;
		inline static FreeCamera deserialize(const std::string& _data);


	};

	// Serialization

	template<typename TScalar>
	inline std::ostream& operator<<(std::ostream& _out, const typename FreeCamera<TScalar>::Projection& _projection);
	template<typename TScalar>
	inline std::istream& operator>>(std::istream& _in, typename FreeCamera<TScalar>::Projection& _projection);

	template<typename TScalar>
	inline std::ostream& operator<<(std::ostream& _out, const typename FreeCamera<TScalar>::View& _view);
	template<typename TScalar>
	inline std::istream& operator>>(std::istream& _in, typename FreeCamera<TScalar>::View& _view);

	template<typename TScalar>
	inline std::ostream& operator<<(std::ostream& _out, const FreeCamera<TScalar>& _camera);
	template<typename TScalar>
	inline std::istream& operator>>(std::istream& _in, FreeCamera<TScalar>& _camera);

}

// template implementation
#define CINOLIB_GL_FREE_CAMERA_HEADER
#include "FreeCamera.tpp" 
#undef CINOLIB_GL_FREE_CAMERA_HEADER

#endif // include guard
