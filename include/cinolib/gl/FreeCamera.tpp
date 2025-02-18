
#ifndef CINOLIB_GL_FREE_CAMERA_HEADER
#error __FILE__ should not be directly included
#endif

#include <stdexcept>
#include <cmath>
#include <cinolib/deg_rad.h>
#include <cinolib/cot.h>
#include <cinolib/clamp.h>
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
	void FreeCamera<TScalar>::Projection::validate() const
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
	typename FreeCamera<TScalar>::Mat FreeCamera<TScalar>::Projection::matrix() const
	{
		validate();
		const TScalar nfd{ nearZ - farZ };
		const TScalar nfs{ nearZ + farZ };
		if (perspective)
		{
			const TScalar f{ static_cast<TScalar>(cot(to_rad(static_cast<double>(verticalFieldOfView) / 2.0))) };
			return Mat({
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
			return Mat({
				//	1					2					3					4
					1 / right,			0,					0,					0,
					0,					1 / top,			0,					0,
					0,					0,					2 / nfd,			nfs / nfd,
					0,					0,					0,					1
				});
		}
	}

	template<typename TScalar>
	std::string FreeCamera<TScalar>::Projection::serialize() const
	{
		std::stringstream stream{};
		stream << *this;
		return stream.str();
	}

	template<typename TScalar>
	typename FreeCamera<TScalar>::Projection FreeCamera<TScalar>::Projection::deserialize(const std::string& _data)
	{
		std::stringstream stream{ _data };
		Projection obj;
		stream >> obj;
		return obj;
	}

	// FreeCamera::View

	template<typename TScalar>
	void FreeCamera<TScalar>::View::getYawAndPitch(const Vec& _worldUp, const Vec& _worldForward, TScalar& _yaw, TScalar& _pitch) const
	{		
		// FIXME (francescozoccheddu) placeholder that only works for up=(0,1,0) and forward=(0,0,-1)
		const Vec& normWorldUp{ _worldUp.normalized() };
		const Vec& normWorldForward{ _worldForward.normalized() };
		if (!(normWorldUp == Vec{ 0,1,0 } && normWorldForward == Vec{ 0,0,-1 }))
		{
			throw std::runtime_error{ "not implemented yet" };
		}
		const vec3d back{ normBack() };
		_pitch = to_deg(std::asin(-back.y()));
		_yaw = to_deg(std::atan2(back.x(), back.z()));
	}

	template<typename TScalar>
	TScalar FreeCamera<TScalar>::View::wrapAngle(TScalar _angle)
	{
		_angle = std::fmod(static_cast<double>(_angle) + 180, 360);
		return static_cast<TScalar>(_angle >= 0 ? _angle - 180 : _angle + 180);
	}

	template<typename TScalar>
	void FreeCamera<TScalar>::View::validateWorldDirections(const Vec& _worldUp, const Vec& _worldForward)
	{
		if (_worldUp.is_deg())
		{
			throw std::domain_error{ "worldUp must be a non-null, finite vector" };
		}
		if (_worldForward.is_deg())
		{
			throw std::domain_error{ "worldForward must be a non-null, finite vector" };
		}
	}

	template<typename TScalar>
	void FreeCamera<TScalar>::View::validatePitch(TScalar _pitch)
	{
		if (std::abs(_pitch) >= 90)
		{
			throw std::out_of_range{ "pitch must fall in (-90,90)" };
		}
	}

	template<typename TScalar>
	void FreeCamera<TScalar>::View::validatePitchLimit(TScalar _pitchLimit)
	{
		if (_pitchLimit <= 0 || _pitchLimit >= 45)
		{
			throw std::out_of_range{ "pitchLimit must fall in (0,90)" };
		}
	}

	template<typename TScalar>
	typename FreeCamera<TScalar>::Vec FreeCamera<TScalar>::View::normLeft() const
	{
		return -normRight();
	}

	template<typename TScalar>
	typename FreeCamera<TScalar>::Vec FreeCamera<TScalar>::View::normRight() const
	{
		return forward.cross(up).normalized();
	}

	template<typename TScalar>
	typename FreeCamera<TScalar>::Vec FreeCamera<TScalar>::View::normDown() const
	{
		return -normUp();
	}

	template<typename TScalar>
	typename FreeCamera<TScalar>::Vec FreeCamera<TScalar>::View::normUp() const
	{
		return up.normalized();
	}

	template<typename TScalar>
	typename FreeCamera<TScalar>::Vec FreeCamera<TScalar>::View::normBack() const
	{
		return -normForward();
	}

	template<typename TScalar>
	typename FreeCamera<TScalar>::Vec FreeCamera<TScalar>::View::normForward() const
	{
		return forward.normalized();
	}

	template<typename TScalar>
	typename FreeCamera<TScalar>::Vec FreeCamera<TScalar>::View::centerAt(TScalar _depth) const
	{
		return eye + normForward() * _depth;
	}

	template<typename TScalar>
	void FreeCamera<TScalar>::View::lookAt(const Vec& _target)
	{
		forward = _target - eye;
		const Vec right{ normRight() };
		up = right.cross(forward);
	}

	template<typename TScalar>
	void FreeCamera<TScalar>::View::lookAtFrom(const Vec& _target, const Vec& _eye)
	{
		eye = _eye;
		lookAt(_target);
	}

	template<typename TScalar>
	void FreeCamera<TScalar>::View::rotateAroundCenterAt(const Vec& _axis, TScalar _angle, TScalar _depth)
	{
		const cinolib::mat<3, 3, TScalar> rotation(cinolib::mat<3, 3, TScalar>::ROT_3D(_axis.normalized(), static_cast<TScalar>(to_rad(_angle))));
		const Vec target{ centerAt(_depth) };
		eye = rotation * (eye - target) + target;
		lookAt(target);
	}

	template<typename TScalar>
	void FreeCamera<TScalar>::View::rotate(const Vec& _axis, TScalar _angle)
	{
		const cinolib::mat<3, 3, TScalar> rotation(cinolib::mat<3, 3, TScalar>::ROT_3D(_axis.normalized(), static_cast<TScalar>(to_rad(_angle))));
		up = rotation * up;
		forward = rotation * forward;
	}

	template<typename TScalar>
	void FreeCamera<TScalar>::View::rotateFps(const Vec& _worldUp, const Vec& _worldForward, TScalar _yaw, TScalar _pitch, TScalar _pitchLimit)
	{
		validateWorldDirections(_worldUp, _worldForward);
		validatePitchLimit(_pitchLimit);
		TScalar yaw, pitch;
		getYawAndPitch(_worldUp, _worldForward, yaw, pitch);
		yaw += _yaw;
		pitch += _pitch;
		pitch = clamp(pitch, -90 + _pitchLimit, 90 - _pitchLimit);
		*this = fps(_worldUp, _worldForward, eye, yaw, pitch);
	}

	template<typename TScalar>
	void FreeCamera<TScalar>::View::rotateTps(const Vec& _worldUp, const Vec& _worldForward, TScalar _pivotDepth, TScalar _yaw, TScalar _pitch, TScalar _pitchLimit)
	{
		validateWorldDirections(_worldUp, _worldForward);
		validatePitchLimit(_pitchLimit);
		TScalar yaw, pitch;
		getYawAndPitch(_worldUp, _worldForward, yaw, pitch);
		yaw += _yaw;
		pitch += _pitch;
		pitch = clamp(pitch, -90 + _pitchLimit, 90 - _pitchLimit);
		*this = tps(_worldUp, _worldForward, centerAt(_pivotDepth), _pivotDepth, yaw, pitch);
	}

	template<typename TScalar>
	typename FreeCamera<TScalar>::View FreeCamera<TScalar>::View::lookAt(const Vec& _eye, const Vec& _target, const Vec& _up)
	{
		if (_eye == _target)
		{
			throw std::domain_error{ "target cannot be equal to eye" };
		}
		View view;
		view.eye = _eye;
		view.up = _up;
		view.forward = _target - _eye;
		view.validate();
		return view;
	}

	template<typename TScalar>
	typename FreeCamera<TScalar>::View FreeCamera<TScalar>::View::fps(const Vec& _worldUp, const Vec& _worldForward, const Vec& _eye, TScalar _yaw, TScalar _pitch)
	{
		validateWorldDirections(_worldUp, _worldForward);
		_pitch = wrapAngle(_pitch);
		validatePitch(_pitch);
		View view;
		view.eye = _eye;
		view.up = _worldUp;
		view.forward = _worldForward;
		view.rotate(_worldUp, _yaw);
		view.rotate(view.normRight(), _pitch);
		return view;
	}

	template<typename TScalar>
	typename FreeCamera<TScalar>::View FreeCamera<TScalar>::View::tps(const Vec& _worldUp, const Vec& _worldForward, const Vec& _target, TScalar _distance, TScalar _yaw, TScalar _pitch)
	{
		validateWorldDirections(_worldUp, _worldForward);
		_pitch = wrapAngle(_pitch);
		validatePitch(_pitch);
		if (_distance == 0)
		{
			throw std::domain_error{ "distance cannot be null" };
		}
		View view;
		view.eye = _target - _worldForward.normalized() * _distance;
		view.up = _worldUp;
		view.forward = _worldForward;
		view.rotateAroundCenterAt(_worldUp, _yaw, _distance);
		view.rotateAroundCenterAt(view.normRight(), _pitch, _distance);
		return view;
	}

	template<typename TScalar>
	void FreeCamera<TScalar>::View::validate() const
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
	typename FreeCamera<TScalar>::Mat FreeCamera<TScalar>::View::matrix() const
	{
		validate();

		const Vec right(normRight());
		const Vec back(normBack());
		const Vec up(normUp());

		return Mat({
			//	1				2				3				4
				right.x(),		right.y(),		right.z(),		-right.dot(eye),
				up.x(),			up.y(),			up.z(),			-up.dot(eye),
				back.x(),		back.y(),		back.z(),		-back.dot(eye),
				0,				0,				0,				1
			});
	}

	template<typename TScalar>
	std::string FreeCamera<TScalar>::View::serialize() const
	{
		std::stringstream stream{};
		stream << *this;
		return stream.str();
	}

	template<typename TScalar>
	typename FreeCamera<TScalar>::View FreeCamera<TScalar>::View::deserialize(const std::string& _data)
	{
		std::stringstream stream{ _data };
		View obj;
		stream >> obj;
		return obj;
	}

	template<typename TScalar>
	std::ostream& operator<<(std::ostream& _out, const typename FreeCamera<TScalar>::View& _view)
	{
		constexpr char sep{ ' ' };
		return _out
			<< _view.eye << sep
			<< _view.up << sep
			<< _view.forward << sep;
	}

	template<typename TScalar>
	std::istream& operator>>(std::istream& _in, typename FreeCamera<TScalar>::View& _view)
	{
		return _in
			>> _view.eye
			>> _view.up
			>> _view.forward;
	}

	// FreeCamera

	template<typename TScalar>
	void FreeCamera<TScalar>::updateProjection()
	{
		m_projectionViewDirty = m_projectionDirty = true;
	}

	template<typename TScalar>
	void FreeCamera<TScalar>::updateView()
	{
		m_projectionViewDirty = m_viewDirty = true;
	}

	template<typename TScalar>
	void FreeCamera<TScalar>::updateProjectionAndView()
	{
		updateProjection();
		updateView();
	}

	template<typename TScalar>
	const typename FreeCamera<TScalar>::Mat& FreeCamera<TScalar>::projectionMatrix() const
	{
		if (m_projectionDirty)
		{
			m_projectionDirty = false;
			m_projectionCache = projection.matrix();
		}
		return m_projectionCache;
	}

	template<typename TScalar>
	const typename FreeCamera<TScalar>::Mat& FreeCamera<TScalar>::viewMatrix() const
	{
		if (m_viewDirty)
		{
			m_viewDirty = false;
			m_viewCache = view.matrix();
		}
		return m_viewCache;
	}

	template<typename TScalar>
	const typename FreeCamera<TScalar>::Mat& FreeCamera<TScalar>::projectionViewMatrix() const
	{
		if (m_projectionViewDirty)
		{
			m_projectionViewDirty = false;
			m_projectionViewCache = projectionMatrix() * viewMatrix();
		}
		return m_projectionViewCache;
	}

	template<typename TScalar>
	std::string FreeCamera<TScalar>::serialize() const
	{
		std::stringstream stream{};
		stream << *this;
		return stream.str();
	}

	template<typename TScalar>
	FreeCamera<TScalar> FreeCamera<TScalar>::deserialize(const std::string& _data)
	{
		std::stringstream stream{ _data };
		FreeCamera obj;
		stream >> obj;
		return obj;
	}

	template<typename TScalar>
	std::ostream& operator<<(std::ostream& _out, const FreeCamera<TScalar>& _camera)
	{
		constexpr char sep{ ' ' };
		return _out
			<< _camera.projection << sep
			<< _camera.view;
	}

	template<typename TScalar>
	std::istream& operator>>(std::istream& _in, FreeCamera<TScalar>& _camera)
	{
		return _in
			>> _camera.projection
			>> _camera.view;
	}

}