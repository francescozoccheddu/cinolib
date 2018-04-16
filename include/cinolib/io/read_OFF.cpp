/*********************************************************************************
*  Copyright(C) 2016: Marco Livesu                                               *
*  All rights reserved.                                                          *
*                                                                                *
*  This file is part of CinoLib                                                  *
*                                                                                *
*  CinoLib is dual-licensed:                                                     *
*                                                                                *
*   - For non-commercial use you can redistribute it and/or modify it under the  *
*     terms of the GNU General Public License as published by the Free Software  *
*     Foundation; either version 3 of the License, or (at your option) any later *
*     version.                                                                   *
*                                                                                *
*   - If you wish to use it as part of a commercial software, a proper agreement *
*     with the Author(s) must be reached, based on a proper licensing contract.  *
*                                                                                *
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE       *
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.     *
*                                                                                *
*  Author(s):                                                                    *
*                                                                                *
*     Marco Livesu (marco.livesu@gmail.com)                                      *
*     http://pers.ge.imati.cnr.it/livesu/                                        *
*                                                                                *
*     Italian National Research Council (CNR)                                    *
*     Institute for Applied Mathematics and Information Technologies (IMATI)     *
*     Via de Marini, 6                                                           *
*     16149 Genoa,                                                               *
*     Italy                                                                      *
**********************************************************************************/
#include <cinolib/io/read_OFF.h>


#include <iostream>

namespace cinolib
{

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CINO_INLINE
void read_OFF(const char                     * filename,
              std::vector<vec3d>             & verts,
              std::vector<std::vector<uint>> & polys,
              std::vector<Color>             & poly_colors)
{
    verts.clear();
    polys.clear();
    poly_colors.clear();

    setlocale(LC_NUMERIC, "en_US.UTF-8"); // makes sure "." is the decimal separator

    FILE *f = fopen(filename, "r");

    if(!f)
    {
        std::cerr << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : read_OFF() : couldn't open input file " << filename << std::endl;
        exit(-1);
    }

    int err;
    uint nv, np, ne;
    bool has_colors;

    char header[100];
    err = fscanf(f, "%s", header);
    if(strcmp(header, "OFF" )==0) has_colors = false; else
    if(strcmp(header, "COFF")==0) has_colors = true;  else
    assert(false);

    err = fscanf(f, "%d %d %d\n", &nv, &np, &ne);
    std::cout << nv << " " << np << " " << ne << std::endl;

    for(uint i=0; i<nv; ++i)
    {
        // http://stackoverflow.com/questions/16839658/printf-width-specifier-to-maintain-precision-of-floating-point-value
        //
        double x, y, z;
        err = fscanf(f, "%lf %lf %lf\n", &x, &y, &z);
        verts.push_back(vec3d(x,y,z));
    }

    for(uint i=0; i<np; ++i)
    {
        uint n_corners;
        err = fscanf(f, "%d", &n_corners);

        std::vector<uint> poly;
        for(uint j=0; j<n_corners; ++j)
        {
            uint vid;
            err = fscanf(f, "%d", &vid);
            poly.push_back(vid);
        }
        polys.push_back(poly);

        if(has_colors)
        {
            Color c;
            err = fscanf(f, "%f %f %f %f\n", &c.r, &c.g, &c.b, &c.a);
            poly_colors.push_back(c);
        }
    }
    fclose(f);
}


}
