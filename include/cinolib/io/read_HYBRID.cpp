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
#include <cinolib/io/read_HYBRID.h>
#include <iostream>
#include <cassert>

namespace cinolib
{

CINO_INLINE
void read_HYBDRID(const char                     * filename,
                  std::vector<vec3d>             & verts,
                  std::vector<std::vector<unsigned int>> & faces,
                  std::vector<std::vector<unsigned int>> & polys,
                  std::vector<std::vector<bool>> & polys_face_winding)
{
    setlocale(LC_NUMERIC, "en_US.UTF-8"); // makes sure "." is the decimal separator

    verts.clear();
    faces.clear();
    polys.clear();
    polys_face_winding.clear();

    FILE *fp = fopen(filename, "r");

    if(!fp)
    {
        std::cerr << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : load_HYBRID() : couldn't open input file " << filename << std::endl;
        exit(-1);
    }

    unsigned int nv, nf, nc;
    fscanf(fp, "%d %d %d", &nv, &nf, &nc);
    nc /= 3; // hack, bug in files?

    for(unsigned int i=0; i<nv; ++i)
    {
        double x, y, z;
        fscanf(fp, "%lf %lf %lf", &x, &y, &z);
        verts.push_back(vec3d{x,y,z});
    }

    for(unsigned int i=0; i<nf; ++i)
    {
        unsigned int n_verts;
        fscanf(fp, "%d", &n_verts);

        std::vector<unsigned int> face;
        for(unsigned int j=0; j<n_verts; ++j)
        {
            unsigned int vid;
            fscanf(fp, "%d", &vid);
            face.push_back(vid);
        }
        faces.push_back(face);
    }

    for(unsigned int i=0; i<nc; ++i)
    {
        unsigned int nf;
        fscanf(fp, "%d", &nf);

        std::vector<unsigned int> poly;
        std::vector<bool> cell_winding;
        for(unsigned int j=0; j<nf; ++j)
        {
            unsigned int fid;
            fscanf(fp, "%d", &fid);
            poly.push_back(fid);
        }
        polys.push_back(poly);

        unsigned int dummy; fscanf(fp, "%d", &dummy);
        for(unsigned int j=0; j<nf; ++j)
        {
            unsigned int winding;
            fscanf(fp, "%d", &winding);
            assert(winding==0 || winding==1);
            cell_winding.push_back(winding);
        }
        polys_face_winding.push_back(cell_winding);
    }

    fclose(fp);
}

}
