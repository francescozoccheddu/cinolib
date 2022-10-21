#ifndef CINO_LERP_H
#define CINO_LERP_H

#include <array>
#include <cinolib/geometry/vec_mat.h>

namespace cinolib
{

	template<typename TReal, unsigned int TDim>
	vec<TDim, TReal> lerp1(const std::array<vec<TDim, TReal>, 2>& _src, TReal _alpha);

	template<typename TReal, unsigned int TDim>
	vec<TDim, TReal> lerp2(const std::array<vec<TDim, TReal>, 4>& _src, const vec2<TReal>& _alpha);

	template<typename TReal, unsigned int TDim>
	vec<TDim, TReal> lerp3(const std::array<vec<TDim, TReal>, 8>& _src, const vec3<TReal>& _alpha);

}

#include "lerp.tpp"

#endif // CINO_LERP_H
