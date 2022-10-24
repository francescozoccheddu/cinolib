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
#include <cinolib/io/read_HEDRA.h>
#include <iostream>

namespace cinolib
{

CINO_INLINE
void read_HEDRA(const char                     * filename,
                std::vector<vec3d>             & verts,
                std::vector<std::vector<unsigned int>> & faces,
                std::vector<std::vector<unsigned int>> & polys,
                std::vector<std::vector<bool>> & polys_winding)
{
    setlocale(LC_NUMERIC, "en_US.UTF-8"); // makes sure "." is the decimal separator

    verts.clear();
    faces.clear();
    polys.clear();
    polys_winding.clear();

    FILE *fp = fopen(filename, "r");

    if(!fp)
    {
        std::cerr << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : load_HEDRA() : couldn't open input file " << filename << std::endl;
        exit(-1);
    }

    unsigned int nv, nf, np;
    fscanf(fp, "%d %d %d", &nv, &nf, &np);

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

        std::vector<unsigned int> f;
        for(unsigned int j=0; j<n_verts; ++j)
        {
            unsigned int vid;
            fscanf(fp, "%d", &vid);
            f.push_back(vid-1);
        }
        faces.push_back(f);
    }

    for(unsigned int i=0; i<np; ++i)
    {
        unsigned int nf;
        fscanf(fp, "%d", &nf);

        std::vector<unsigned int> p;
        std::vector<bool> p_winding;
        for(unsigned int j=0; j<nf; ++j)
        {
            int fid;
            fscanf(fp, "%d", &fid);

            if (fid > 0)
            {
                p.push_back(static_cast<unsigned int>(fid-1));
                p_winding.push_back(true);
            }
            else
            {
                p.push_back(static_cast<unsigned int>(std::fabs(fid)-1));
                p_winding.push_back(false);
            }
        }
        polys.push_back(p);
        polys_winding.push_back(p_winding);
    }

    fclose(fp);
}

}
