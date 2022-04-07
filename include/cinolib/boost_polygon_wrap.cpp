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
#include <cinolib/boost_polygon_wrap.h>

// FIXME (francescozoccheddu)
#ifdef CINOLIB_USES_BOOST

#include <cinolib/vector_serialization.h>

#ifdef CINOLIB_USES_TRIANGLE
#include <cinolib/triangle_wrap.h>
#endif

namespace cinolib
{

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void polygon_get_edges(const std::vector<BoostPoint> & poly,
                             std::vector<vec2d>      & verts,
                             std::vector<unsigned int>       & edges)
{
    unsigned int base = verts.size();
    unsigned int nv   = poly.size()-1; // first and last verts coincide...
    for(unsigned int vid=0; vid<nv; ++vid)
    {
        verts.push_back(vec2d(boost::geometry::get<0>(poly.at(vid)),
                              boost::geometry::get<1>(poly.at(vid))));
        edges.push_back(base + vid);
        edges.push_back(base + (vid+1)%nv);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void polygon_get_edges(const BoostPolygon       & poly,
                             std::vector<vec2d> & verts,
                             std::vector<unsigned int>  & edges)
{
    polygon_get_edges(poly.outer(), verts, edges);
    for(auto hole : poly.inners()) polygon_get_edges(hole, verts, edges);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void polygon_get_edges(const BoostMultiPolygon  & poly,
                             std::vector<vec2d> & verts,
                             std::vector<unsigned int>  & edges)
{
    for(const BoostPolygon & p : poly)
    {
        std::vector<vec2d> v;
        std::vector<unsigned int>  e;
        polygon_get_edges(p, v, e);

        unsigned int base_addr = verts.size();
        std::copy(v.begin(), v.end(), std::back_inserter(verts));
        for(auto vid : e) edges.push_back(base_addr + vid);
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void polygon_get_edges(const BoostPolygon       & poly,
                       const double             & z, // add third coordinate
                             std::vector<vec3d> & verts,
                             std::vector<unsigned int>  & edges)
{
    std::vector<vec2d> v2d;
    polygon_get_edges(poly, v2d, edges);
    verts = vec3d_from_vec2d(v2d, z);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void polygon_get_edges(const BoostMultiPolygon  & poly,
                       const double             & z, // add third coordinate
                             std::vector<vec3d> & verts,
                             std::vector<unsigned int>  & edges)
{
    std::vector<vec2d> v2d;
    polygon_get_edges(poly, v2d, edges);
    verts = vec3d_from_vec2d(v2d, z);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void triangulate_polygon(const std::vector<BoostPoint> & poly,
                         const std::string               flags,
                               std::vector<vec2d>      & verts,
                               std::vector<unsigned int>       & tris)
{
    std::vector<vec2d> v, h;
    std::vector<unsigned int>  e;
    polygon_get_edges(poly, v, e);
    triangle_wrap(v, e, h, flags, verts, tris);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void triangulate_polygon(const BoostPolygon            & poly,
                         const std::string               flags,
                               std::vector<vec2d>      & verts,
                               std::vector<unsigned int>       & tris)
{
    // find one seed per hole (to robustly clear holes from triangulation)
    std::vector<vec2d> h_seeds;
    for(unsigned int hid=0; hid<poly.inners().size(); ++hid)
    {
        std::vector<vec2d> v_in, h, v_out;
        std::vector<unsigned int>  e, t;
        polygon_get_edges(poly.inners().at(hid), v_in, e);
        triangle_wrap(v_in, e, h, "Q", v_out, t);
        unsigned int v0 = t.at(0);
        unsigned int v1 = t.at(1);
        unsigned int v2 = t.at(2);
        h_seeds.push_back((v_out.at(v0)+v_out.at(v1)+v_out.at(v2))/3.0);
    }

    std::vector<vec2d> v;
    std::vector<unsigned int>  e;
    polygon_get_edges(poly, v, e);
    triangle_wrap(v, e, h_seeds, flags, verts, tris);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void triangulate_polygon(const BoostMultiPolygon       & poly,
                         const std::string               flags,
                               std::vector<vec2d>      & verts,
                               std::vector<unsigned int>       & tris)
{
    // find one seed per hole (to robustly clear holes from triangulation)
    std::vector<vec2d> h_seeds;
    for(const BoostPolygon & p : poly)
    {
        for(unsigned int hid=0; hid<p.inners().size(); ++hid)
        {
            std::vector<vec2d> v_in, h, v_out;
            std::vector<unsigned int>  e, t;
            polygon_get_edges(p.inners().at(hid), v_in, e);
            triangle_wrap(v_in, e, h, "Q", v_out, t);
            unsigned int v0 = t.at(0);
            unsigned int v1 = t.at(1);
            unsigned int v2 = t.at(2);
            h_seeds.push_back((v_out.at(v0)+v_out.at(v1)+v_out.at(v2))/3.0);
        }
    }

    std::vector<vec2d> v;
    std::vector<unsigned int>  e;
    polygon_get_edges(poly, v, e);
    triangle_wrap(v, e, h_seeds, flags, verts, tris);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void triangulate_polygon(const std::vector<BoostPoint> & poly,
                         const std::string               flags,
                         const double                  & z, // adds third coordinate
                               std::vector<vec3d>      & verts,
                               std::vector<unsigned int>       & tris)
{
    std::vector<vec2d> v;
    triangulate_polygon(poly, flags, v, tris);
    verts = vec3d_from_vec2d(v, z);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void triangulate_polygon(const BoostPolygon            & poly,
                         const std::string               flags,
                         const double                  & z, // adds third coordinate
                               std::vector<vec3d>      & verts,
                               std::vector<unsigned int>       & tris)
{
    std::vector<vec2d> v;
    triangulate_polygon(poly, flags, v, tris);
    verts = vec3d_from_vec2d(v, z);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void triangulate_polygon(const BoostMultiPolygon       & poly,
                         const std::string               flags,
                         const double                  & z, // adds third coordinate
                               std::vector<vec3d>      & verts,
                               std::vector<unsigned int>       & tris)
{
    std::vector<vec2d> v;
    triangulate_polygon(poly, flags, v, tris);
    verts = vec3d_from_vec2d(v, z);
}

}

#endif // CINOLIB_USES_BOOST