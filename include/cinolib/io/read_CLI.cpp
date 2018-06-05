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
#include <cinolib/io/read_CLI.h>
#include <string>
#include <sstream>
#include <fstream>

namespace cinolib
{

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

enum
{
    INTERNAL = 0, // inner slice boundary (i.e., slice holes)
    EXTERNAL = 1, // external slice boundary
    OPEN     = 2  // open curve (typically used for support structures)
};

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

std::vector<vec3d> read_polyline(std::string line, const double z)
{
    std::replace(line.begin(), line.end(), '$', ' ');
    std::replace(line.begin(), line.end(), '/', ' ');
    std::replace(line.begin(), line.end(), ',', ' ');

    uint        n_points, dummy;
    std::string dummy_str;

    std::istringstream ss(line);
    ss >> dummy_str >> dummy >> dummy >> n_points;

    std::vector<vec3d> polyline;
    for(uint i=0; i<n_points; ++i)
    {
        vec3d p(0,0,z);
        ss >> p.x() >> p.y();
        polyline.push_back(p);
    }
    return polyline;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

// Reference for COMMON LAYER INTERFACE (CLI) file format:
// http://www.hmilch.net/downloads/cli_format.html
//
// NOTE: output vectors have as many entries as the number of slices. Each
// slice contains a list of internal, external and open polylines. Finally,
// each polyline is a vector of points in 3D.
//
CINO_INLINE
void read_CLI(const char                                   * filename,
              std::vector<std::vector<std::vector<vec3d>>> & internal_polylines, // outer slice boundary
              std::vector<std::vector<std::vector<vec3d>>> & external_polylines, // inner holes
              std::vector<std::vector<std::vector<vec3d>>> & open_polylines,     // support structures
              std::vector<std::vector<std::vector<vec3d>>> & hatches)            // supports/infills
{
    setlocale(LC_NUMERIC, "en_US.UTF-8"); // makes sure "." is the decimal separator

    internal_polylines.clear();
    external_polylines.clear();
    open_polylines.clear();
    hatches.clear();

    std::ifstream f(filename);
    std::string   line;
    uint          n_layers, type;
    int           layer = -1;
    double        z;

    if(!f.is_open())
    {
        std::cerr << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : read_CLI() : couldn't open input file " << filename << std::endl;
        exit(-1);
    }

    while(getline(f, line, '\n'))
    {
        if(sscanf(line.c_str(), "$$LAYERS/%d", &n_layers) == 1)
        {
            internal_polylines.resize(n_layers);
            external_polylines.resize(n_layers);
            open_polylines.resize(n_layers);
            hatches.resize(n_layers);
        }
        else if(sscanf(line.c_str(), "$$LAYER/%lf", &z) == 1)
        {
            ++layer;
        }
        else if(sscanf(line.c_str(), "$$POLYLINE/%*d,%d,%*d,%*s", &type) == 1)
        {
            // NOTE: for INTERNAL and EXTERNAL, the last point is a duplication of the first one
            std::vector<vec3d> pl = read_polyline(line, z);

            switch(type)
            {
                case EXTERNAL : pl.pop_back(); external_polylines.at(layer).push_back(pl); break;
                case INTERNAL : pl.pop_back(); internal_polylines.at(layer).push_back(pl); break;
                case OPEN     : open_polylines.at(layer).push_back(pl); break;
                default       : std::cerr << "WARNING! Unknown polyline type: discarded." << std::endl;
            }            
        }
    }
    f.close();
}

}
