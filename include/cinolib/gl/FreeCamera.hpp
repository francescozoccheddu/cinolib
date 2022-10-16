#ifndef CINOLIB_GL_FREE_CAMERA_INCLUDED
#define CINOLIB_GL_FREE_CAMERA_INCLUDED

// TODO (francescozoccheddu) code style and naming convention should match the rest of cinolib
// TODO (francescozoccheddu) documentation should be added

#include <cinolib/geometry/vec_mat.h>
#include <string>
#include <iostream>

namespace cinolib
{

	// angles in degrees; vectors do not have to be normalized (unless stated otherwise)

	template<typename TScalar>
	class FreeCamera final
	{

		// components

	public:

		using Vec = vec3<TScalar>;
		using Mat = mat<4, 4, TScalar>;

		class Projection final
		{

		public:

			TScalar aspectRatio{ 1 }; // width / height
			TScalar farZ{ 100 }, nearZ{ 1 }; // world units
			TScalar verticalFieldOfView{ 67 }; // angle (perspective) or world units (orthographic)
			bool perspective{ true };

			void setAspect(unsigned int _width, unsigned int _height);

			void validate() const;

			Mat matrix() const; // not cached

			std::string serialize() const;
			static Projection deserialize(const std::string& _data);

			friend std::ostream& operator<<(std::ostream& _out, const Projection& _projection) // ugly, but the only way I found
			{
				constexpr char sep{ ' ' };
				return _out
					<< _projection.aspectRatio << sep
					<< _projection.nearZ << sep
					<< _projection.farZ << sep
					<< _projection.verticalFieldOfView << sep
					<< _projection.perspective;
			}

			friend std::istream& operator>>(std::istream& _in, Projection& _projection) // ugly, but the only way I found
			{
				return _in
					>> _projection.aspectRatio
					>> _projection.nearZ
					>> _projection.farZ
					>> _projection.verticalFieldOfView
					>> _projection.perspective;
			}


		} projection;

		class View final
		{

		private:

			void getYawAndPitch(const Vec& _worldUp, const Vec& _worldForward, TScalar& _yaw, TScalar& _pitch) const;
			static TScalar wrapAngle(TScalar _angle);
			static void validateWorldDirections(const Vec& _worldUp, const Vec& _worldForward);
			static void validatePitch(TScalar _pitch);
			static void validatePitchLimit(TScalar _pitchLimit);

		public:

			Vec eye{ 0,0,0 };
			Vec up{ 0,1,0 };
			Vec forward{ 0,0,-1 };

			Vec normLeft() const;
			Vec normRight() const;
			Vec normDown() const;
			Vec normUp() const;
			Vec normBack() const;
			Vec normForward() const;

			Vec centerAt(TScalar _depth) const;

			void lookAt(const Vec& _target);
			void lookAtFrom(const Vec& _target, const Vec& _eye);
			void rotateAroundCenterAt(const Vec& _axis, TScalar _angle, TScalar _depth);
			void rotate(const Vec& _axis, TScalar _angle);

			void rotateFps(const Vec& _worldUp, const Vec& _worldForward, TScalar _yaw, TScalar _pitch, TScalar _pitchLimit = 10);
			void rotateTps(const Vec& _worldUp, const Vec& _worldForward, TScalar _pivotDepth, TScalar _yaw, TScalar _pitch, TScalar _pitchLimit = 10);

			static View lookAt(const Vec& _eye, const Vec& _target, const Vec& _up);
			static View fps(const Vec& _worldUp, const Vec& _worldForward, const Vec& _eye, TScalar _yaw, TScalar _pitch);
			static View tps(const Vec& _worldUp, const Vec& _worldForward, const Vec& _target, const TScalar _distance, TScalar _yaw, TScalar _pitch);

			void validate() const;

			Mat matrix() const; // not cached

			std::string serialize() const;
			static View deserialize(const std::string& _data);

			friend std::ostream& operator<<(std::ostream& _out, const View& _view) // ugly, but the only way I found
			{
				constexpr char sep{ ' ' };
				return _out
					<< _view.eye << sep
					<< _view.up << sep
					<< _view.forward << sep;
			}

			friend std::istream& operator>>(std::istream& _in, View& _view) // ugly, but the only way I found
			{
				return _in
					>> _view.eye
					>> _view.up
					>> _view.forward;
			}

		} view;

		// matrices

	private:

		mutable Mat m_projectionCache, m_viewCache, m_projectionViewCache;
		mutable bool m_projectionDirty{ true }, m_viewDirty{ true }, m_projectionViewDirty{ true };

	public:

		void updateProjection();
		void updateView();
		void updateProjectionAndView();

		const Mat& projectionMatrix() const; // cached
		const Mat& viewMatrix() const; // cached
		const Mat& projectionViewMatrix() const; // cached

		std::string serialize() const;
		static FreeCamera deserialize(const std::string& _data);

	};

	// serialization

	template<typename TScalar>
	std::ostream& operator<<(std::ostream& _out, const FreeCamera<TScalar>& _camera);
	template<typename TScalar>
	std::istream& operator>>(std::istream& _in, FreeCamera<TScalar>& _camera);

}

// template implementation
#define CINOLIB_GL_FREE_CAMERA_HEADER
#include "FreeCamera.tpp" 
#undef CINOLIB_GL_FREE_CAMERA_HEADER

#endif // include guard
