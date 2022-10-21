#include "lerp.hpp"

namespace cinolib
{

	template<typename TReal, unsigned int TDim>
	vec<TDim, TReal> lerp1(const std::array<vec<TDim, TReal>, 2>& _src, TReal _alpha)
	{
		return _src[0] * (1.0f - _alpha) + _src[1] * _alpha;
	}

	template<typename TReal, unsigned int TDim>
	vec<TDim, TReal> lerp2(const std::array<vec<TDim, TReal>, 4>& _src, const vec2<TReal>& _alpha)
	{
		const vec<TDim, TReal> y1{ lerp1(std::array{ _src[0], _src[1] }, _alpha.x()) };
		const vec<TDim, TReal> y2{ lerp1(std::array{ _src[2], _src[3] }, _alpha.x()) };
		return lerp1(std::array{ y1, y2 }, _alpha.y());
	}

	template<typename TReal, unsigned int TDim>
	vec<TDim, TReal> lerp3(const std::array<vec<TDim, TReal>, 8>& _src, const vec3<TReal>& _alpha)
	{
		const vec<TDim, TReal> z1{ lerp2(std::array{ _src[0], _src[1], _src[2], _src[3] }, _alpha.rem_coord()) };
		const vec<TDim, TReal> z2{ lerp2(std::array{ _src[4], _src[5], _src[6], _src[7] }, _alpha.rem_coord()) };
		return lerp1(std::array{ z1, z2 }, _alpha.z());
	}

}