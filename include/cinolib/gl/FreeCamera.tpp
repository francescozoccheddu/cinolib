
#ifndef CINOLIB_GL_FREE_CAMERA_HEADER
#error __FILE__ should not be directly included
#endif

#include <stdexcept>
#include <cmath>
#include <cinolib/deg_rad.h>
#include <cinolib/cot.h>
#include <sstream>

namespace cinolib
{

	// FreeCamera::Projection

	template<typename TScalar>
	void FreeCamera<TScalar>::Projection::setAspect(unsigned int _width, unsigned int _height)
	{
		aspectRatio = static_cast<TScalar>(_width) / static_cast<TScalar>(_height);
	}

	template<typename TScalar>
	inline void FreeCamera<TScalar>::Projection::validate() const
	{
		if (!std::isfinite(aspectRatio) || aspectRatio <= 0)
		{
			throw std::domain_error{ "aspectRatio must be a positive real number" };
		}
		if (!std::isfinite(nearZ) || nearZ <= 0)
		{
			throw std::domain_error{ "nearZ must be a positive real number" };
		}
		if (!std::isfinite(farZ) || farZ <= nearZ)
		{
			throw std::domain_error{ "farZ must be a positive real number greter than nearZ" };
		}
		if (!std::isfinite(verticalFieldOfView) || verticalFieldOfView <= 0)
		{
			throw std::domain_error{ "verticalFieldOfView must be a positive real number" };
		}
	}

	template<typename TScalar>
	inline typename FreeCamera<TScalar>::mat FreeCamera<TScalar>::Projection::matrix() const
	{
		validate();
		const TScalar nfd{ nearZ - farZ };
		const TScalar nfs{ nearZ + farZ };
		if (perspective)
		{
			const TScalar f{ static_cast<TScalar>(cot(to_rad(static_cast<double>(verticalFieldOfView) / 2.0))) };
			return mat({
				//	1					2					3					4
					f / aspectRatio,	0,					0,					0,
					0,					f,					0,					0,
					0,					0,					nfs / nfd,			2 * farZ * nearZ / nfd,
					0,					0,					-1,					0
				});
		}
		else
		{
			const TScalar right{ verticalFieldOfView * aspectRatio };
			const TScalar top{ verticalFieldOfView };
			return mat({
				//	1					2					3					4
					1 / right,			0,					0,					0,
					0,					1 / top,			0,					0,
					0,					0,					2 / nfd,			nfs / nfd,
					0,					0,					0,					1
				});
		}
	}

	template<typename TScalar>
	inline std::string FreeCamera<TScalar>::Projection::serialize() const
	{
		std::stringstream stream{};
		stream << *this;
		return stream.str();
	}

	template<typename TScalar>
	inline typename FreeCamera<TScalar>::Projection FreeCamera<TScalar>::Projection::deserialize(const std::string& _data)
	{
		std::stringstream stream{ _data };
		Projection obj;
		stream >> obj;
		return obj;
	}

	template<typename TScalar>
	inline std::ostream& operator<<(std::ostream& _out, const typename FreeCamera<TScalar>::Projection& _projection)
	{
		constexpr char sep{ ' ' };
		_out
			<< _projection.aspectRatio << sep
			<< _projection.nearZ << sep
			<< _projection.farZ << sep
			<< _projection.verticalFieldOfView << sep
			<< _projection.perspective;
		return _out;
	}

	template<typename TScalar>
	inline std::istream& operator>>(std::istream& _in, typename FreeCamera<TScalar>::Projection& _projection)
	{
		_in
			>> _projection.aspectRatio
			>> _projection.nearZ
			>> _projection.farZ
			>> _projection.verticalFieldOfView
			>> _projection.perspective;
		return _in;
	}

	// FreeCamera::View

	template<typename TScalar>
	inline typename FreeCamera<TScalar>::vec FreeCamera<TScalar>::View::normLeft() const
	{
		return -normRight();
	}

	template<typename TScalar>
	inline typename FreeCamera<TScalar>::vec FreeCamera<TScalar>::View::normRight() const
	{
		return forward.cross(up).normalized();
	}

	template<typename TScalar>
	inline typename FreeCamera<TScalar>::vec FreeCamera<TScalar>::View::normDown() const
	{
		return -normUp();
	}

	template<typename TScalar>
	inline typename FreeCamera<TScalar>::vec FreeCamera<TScalar>::View::normUp() const
	{
		return up.normalized();
	}

	template<typename TScalar>
	inline typename FreeCamera<TScalar>::vec FreeCamera<TScalar>::View::normBack() const
	{
		return -normForward();
	}

	template<typename TScalar>
	inline typename FreeCamera<TScalar>::vec FreeCamera<TScalar>::View::normForward() const
	{
		return forward.normalized();
	}

	template<typename TScalar>
	inline void FreeCamera<TScalar>::View::lookAt(const vec& _target)
	{
		forward = _target - eye;
		const vec right{ normRight() };
		up = right.cross(forward);
	}

	template<typename TScalar>
	inline void FreeCamera<TScalar>::View::lookAtFrom(const vec& _target, const vec& _eye)
	{
		eye = _eye;
		lookAt(_target);
	}

	template<typename TScalar>
	inline void FreeCamera<TScalar>::View::rotateAroundPivot(const vec& _axis, TScalar _angle, const vec& _pivot)
	{

	}

	template<typename TScalar>
	inline void FreeCamera<TScalar>::View::rotate(const vec& _axis, TScalar _angle)
	{
		rotateAroundPivot(_axis, _angle, eye);
	}

	template<typename TScalar>
	inline void FreeCamera<TScalar>::View::validate() const
	{
		if (!eye.is_finite())
		{
			throw std::domain_error{ "eye must be a finite vector" };
		}
		if (up.is_deg())
		{
			throw std::domain_error{ "up must be a non-null, finite vector" };
		}
		if (forward.is_deg())
		{
			throw std::domain_error{ "forward must be a non-null, finite vector" };
		}
	}

	template<typename TScalar>
	inline typename FreeCamera<TScalar>::mat FreeCamera<TScalar>::View::matrix() const
	{
		validate();

		const vec right(normRight());
		const vec back(normBack());
		const vec up(normUp());

		return mat({
			//	1			2			3			4
				right.x(),	up.x(),		back.x(),	-eye.x(),
				right.y(),	up.y(),		back.y(),	-eye.y(),
				right.z(),	up.z(),		back.z(),	-eye.z(),
				0,			0,			0,			1
			});
	}

	template<typename TScalar>
	inline std::string FreeCamera<TScalar>::View::serialize() const
	{
		std::stringstream stream{};
		stream << *this;
		return stream.str();
	}

	template<typename TScalar>
	inline typename FreeCamera<TScalar>::View FreeCamera<TScalar>::View::deserialize(const std::string& _data)
	{
		std::stringstream stream{ _data };
		View obj;
		stream >> obj;
		return obj;
	}

	template<typename TScalar>
	inline std::ostream& operator<<(std::ostream& _out, const typename FreeCamera<TScalar>::View& _view)
	{
		constexpr char sep{ ' ' };
		_out
			<< _view.eye << sep
			<< _view.up << sep
			<< _view.forward << sep;
		return _out;
	}

	template<typename TScalar>
	inline std::istream& operator>>(std::istream& _in, typename FreeCamera<TScalar>::View& _view)
	{
		_in
			>> _view.eye
			>> _view.up
			>> _view.forward;
		return _in;
	}

	// FreeCamera

	template<typename TScalar>
	inline void FreeCamera<TScalar>::updateProjection()
	{
		m_projectionViewDirty = m_projectionDirty = true;
	}

	template<typename TScalar>
	inline void FreeCamera<TScalar>::updateView()
	{
		m_projectionViewDirty = m_viewDirty = true;
	}

	template<typename TScalar>
	inline void FreeCamera<TScalar>::updateProjectionAndView()
	{
		updateProjection();
		updateView();
	}

	template<typename TScalar>
	inline typename const FreeCamera<TScalar>::mat& FreeCamera<TScalar>::projectionMatrix() const
	{
		if (m_projectionDirty)
		{
			m_projectionDirty = false;
			m_projectionCache = projection.matrix();
		}
		return m_projectionCache;
	}

	template<typename TScalar>
	inline typename const FreeCamera<TScalar>::mat& FreeCamera<TScalar>::viewMatrix() const
	{
		if (m_viewDirty)
		{
			m_viewDirty = false;
			m_viewCache = view.matrix();
		}
		return m_viewCache;
	}

	template<typename TScalar>
	inline typename const FreeCamera<TScalar>::mat& FreeCamera<TScalar>::projectionViewMatrix() const
	{
		if (m_projectionViewDirty)
		{
			m_projectionViewDirty = false;
			m_projectionViewCache = projectionMatrix() * viewMatrix();
		}
		return m_projectionViewCache;
	}

	template<typename TScalar>
	inline std::string FreeCamera<TScalar>::serialize() const
	{
		std::stringstream stream{};
		stream << *this;
		return stream.str();
	}

	template<typename TScalar>
	inline FreeCamera<TScalar> FreeCamera<TScalar>::deserialize(const std::string& _data)
	{
		std::stringstream stream{ _data };
		FreeCamera obj;
		stream >> obj;
		return obj;
	}

	template<typename TScalar>
	inline std::ostream& operator<<(std::ostream& _out, const FreeCamera<TScalar>& _camera)
	{
		constexpr char sep{ ' ' };
		/*_out
			<< _camera.projection << sep
			<< _camera.view;*/
		return _out;
	}

	template<typename TScalar>
	inline std::istream& operator>>(std::istream& _in, FreeCamera<TScalar>& _camera)
	{
		/*_in
			>> _camera.projection
			>> _camera.view;*/
		return _in;
	}

}