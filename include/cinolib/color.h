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
#ifndef CINO_COLOR_H
#define CINO_COLOR_H

#include <cinolib/cino_inline.h>
#include <cinolib/geometry/vec_mat.h>
#include <stdint.h>
#include <iostream>
#include <cmath>

namespace cinolib
{

class Color
{
    public:

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        static constexpr Color BLACK() { return Color( 0, 0, 0); }
        static constexpr Color WHITE() { return Color( 1, 1, 1); }
        //
        static constexpr Color RED    (const float lum = 1.f) { return Color( lum*1.f,     0.f,     0.f); }
        static constexpr Color GREEN  (const float lum = 1.f) { return Color(     0.f, lum*1.f,     0.f); }
        static constexpr Color BLUE   (const float lum = 1.f) { return Color(     0.f,     0.f, lum*1.f); }
        static constexpr Color YELLOW (const float lum = 1.f) { return Color( lum*1.f, lum*1.f,     0.f); }
        static constexpr Color MAGENTA(const float lum = 1.f) { return Color( lum*1.f,     0.f, lum*1.f); }
        static constexpr Color CYAN   (const float lum = 1.f) { return Color(     0.f, lum*1.f, lum*1.f); }
        static constexpr Color GRAY   (const float lum = 1.f) { return Color( lum*.5f, lum*.5f, lum*.5f); }
        //
        static constexpr Color PASTEL_YELLOW()  { return Color(254.f/255.f, 229.f/255.f, 157.f/255.f); }
        static constexpr Color PASTEL_MAGENTA() { return Color(252.f/255.f,  89.f/255.f, 148.f/255.f); }
        static constexpr Color PASTEL_GREEN()   { return Color(198.f/255.f, 223.f/255.f, 182.f/255.f); }
        static constexpr Color PASTEL_CYAN()    { return Color( 77.f/255.f, 193.f/255.f, 198.f/255.f); }
        static constexpr Color PASTEL_VIOLET()  { return Color(124.f/255.f, 158.f/255.f, 251.f/255.f); }
        static constexpr Color PASTEL_PINK()    { return Color(182.f/255.f, 200.f/255.f, 230.f/255.f); }
        static constexpr Color PASTEL_RED()     { return Color(253.f/255.f, 104.f/255.f, 118.f/255.f); }
        static constexpr Color PASTEL_ORANGE()  { return Color(253.f/255.f, 135.f/255.f,  86.f/255.f); }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        explicit constexpr Color(
            const float r = 1.0,
            const float g = 1.0,
            const float b = 1.0,
            const float a = 1.0) : rgba{ r,g,b,a }
        {}

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

              bool    operator==(const Color & c) const;
              bool    operator!=(const Color & c) const;
              bool    operator< (const Color & c) const;
              bool    operator<=(const Color & c) const;
              float & operator[](const unsigned int    i);
        const float & operator[](const unsigned int    i) const;
              Color & operator*=(const double  d);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        uint8_t r_uchar() const { return static_cast<uint8_t>(r() * 255.0); }
        uint8_t g_uchar() const { return static_cast<uint8_t>(g() * 255.0); }
        uint8_t b_uchar() const { return static_cast<uint8_t>(b() * 255.0); }
        uint8_t a_uchar() const { return static_cast<uint8_t>(a() * 255.0); }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        static constexpr Color lerp(const Color& a, const Color& b, const float progress) {
            const float invProgress{ 1.0f - progress };
            return Color{
                a.r() * invProgress + b.r() * progress,
                a.g() * invProgress + b.g() * progress,
                a.b() * invProgress + b.b() * progress,
                a.a() * invProgress + b.a() * progress
            };
        }
        static Color scatter(unsigned int range, unsigned int value, float sat=.5f, float val=.85f);
        static Color hsv_ramp(unsigned int range, unsigned int value);
        static Color parula_ramp(unsigned int range, unsigned int value);
        static Color red_white_blue_ramp_01(float val);
        static Color red_ramp_01(const float val, const bool zero_is_white = true);
        static Color normal2rgb(const vec3d & n, bool flip_neg_z);
        static constexpr Color hsv2rgb(float h, float s, float v, float a = 1.0f) 
        {
            if (s == 0.0) return Color(v, v, v, a); // gray color
            if (h == 1.0) h = 0.0;

            int i = static_cast<int>(h * 6.f);
            float f = h * 6.f - static_cast<float>(i);

            float p = v * (1.f - s);
            float q = v * (1.f - s * f);
            float t = v * (1.f - s * (1.f - f));

            switch (i)
            {
                case 0: return Color(v, t, p, a);
                case 1: return Color(q, v, p, a);
                case 2: return Color(p, v, t, a);
                case 3: return Color(p, q, v, a);
                case 4: return Color(t, p, v, a);
                case 5: return Color(v, p, q, a);
                default: assert(false);
            }
            return Color(); // warning killer
        }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


        constexpr float& r() { return rgba[0]; }
        constexpr float& g() { return rgba[1]; }
        constexpr float& b() { return rgba[2]; }
        constexpr float& a() { return rgba[3]; }
        constexpr const float& r() const { return rgba[0]; }
        constexpr const float& g() const { return rgba[1]; }
        constexpr const float& b() const { return rgba[2]; }
        constexpr const float& a() const { return rgba[3]; }

        float rgba[4];
};

CINO_INLINE std::ostream & operator<<(std::ostream & in, const Color & c);

}

#ifndef  CINO_STATIC_LIB
#include "color.cpp"
#endif

#endif // CINO_COLOR_H
