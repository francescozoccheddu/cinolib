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
#ifndef CINO_HEX_TRANSITION_SCHEMES_H
#define CINO_HEX_TRANSITION_SCHEMES_H

#include <vector>
#include <sys/types.h>

/* Implementation of the convertion schemes to transform an adaptively refined grid
 * with hanging nodes into a conformin all hexahedral mesh.
 * These schemes are based on ideas contained in:
 *
 *     Advances in Octree-Based All-Hexahedral Mesh Generation: Handling Sharp Features
 *     Loic Marechal
 *     Proceedings of the 18th International Meshing Roundtable, 2009
 *
 * which are further extended and made comprehensible, so that anybody can reproduce them.
 *
 * As in the original paper, schemes are defined in the primal mesh, and are meant to
 * regularize grid vertex valences, bringing them to 6 such that, once dualized,
 * each vertex yields a hexahedron.
 *
 * Marechal's article describes only the basic (flat) scheme, and barely mentions the
 * schemes that are necessary to handle convex and concave edges. Additionally to these
 * three, a fourth scheme is necessary to handle concave corners. Overall, we have:
 *
 *   - Flat_Face_4_to_2 => transitions from a 4x4 grid to a 2x2 grid
 *
 *   - Conv_Edge_4_to_2 => transitions from a convex edge split into 4 parts
 *                         to an edge split into 2 parts
 *
 *   - Conc_Edge_4_to_2 => transitions from a concave edge split into 4 parts
 *                         to an edge split into 2 parts
 *
 *   - Conv_Vert_4_to_2 => transition brick for concave vertices shared between
 *                         three mutually orthogonal 4x4 grids
 *
 *
 * For each scheme we hard coded vertices, faces and cells of the (primal) polyhedral mesh.
 * Each mesh has normal size (i.e. the longest side has length 1), and the vertex with id 0
 * is always the "application point", that is the vertex that must be used for positioning
 * the scheme in the grid. Specifically:
 *
 *   - for Flat_Face_4_to_2 => vertex v0 is the point at the middle of the 4x4 grid
 *
 *   - for Conv_Edge_4_to_2 => vertex v0 is the midpoint of the refined edge
 *
 *   - for Conc_Edge_4_to_2 => vertex v0 is the midpoint of the refined edge
 *
 *   - for Conv_Vert_4_to_2 => vertex v0 is the concave vertex at the intersection of the 4x4 grids
 *
 *
 * USAGE: Schemes are meant to be used as follows:
 *
 *  1 - locate type, size, and application point of the scheme in the adaptive grid
 *
 *  2 - position the scheme in your grid
 *
 *  3 - remove the standard grid elements from your grid, substituting them with the
 *      scheme elements, also merging merging coincident vertices to topologically
 *      weld the two meshes
 *
 *  4 - dualize the so generated polyhedral mesh to obtain a full hexahedral mesh
 *
 * Steps (2,3) can be done with cinolib::hex_transition_install. note that this routine
 * does the installation of the new elements only, and does not take care of removing the
 * old elements from the grid.
 *
 * Step 4 is a standard dualization, and can be done with cinolib::dual_mesh
 *
 * TODO: fix orientation of mesh elements
 *
*/

namespace cinolib
{

namespace Flat_Face_4_to_2
{
    static const std::vector<double> verts =
    {
            0 ,    0 ,     0 ,
            0 ,    0 ,  -0.5 ,
          0.5 ,    0 ,     0 ,
          0.5 ,    0 ,  -0.5 ,
            0 ,  0.5 ,  -0.5 ,
            0 ,  0.5 ,     0 ,
          0.5 ,  0.5 ,     0 ,
          0.5 ,  0.5 ,  -0.5 ,
         -0.5 ,    0 ,  -0.5 ,
         -0.5 ,    0 ,     0 ,
         -0.5 ,  0.5 ,  -0.5 ,
         -0.5 ,  0.5 ,     0 ,
            0 ,    0 ,   0.5 ,
          0.5 ,    0 ,   0.5 ,
            0 ,  0.5 ,   0.5 ,
          0.5 ,  0.5 ,   0.5 ,
         -0.5 ,    0 ,   0.5 ,
         -0.5 ,  0.5 ,   0.5 ,
            0 , 0.15 ,     0 ,
         0.25 ,    0 ,  0.25 ,
        -0.25 ,    0 ,  0.25 ,
            0 ,    0 ,  0.25 ,
         -0.5 ,    0 ,  0.25 ,
          0.5 ,    0 ,  0.25 ,
            0 , 0.25 ,     0 ,
         0.25 ,    0 ,  -0.5 ,
         0.25 ,    0 , -0.25 ,
          0.5 ,    0 , -0.25 ,
         0.25 , 0.15 ,     0 ,
          0.5 , 0.25 ,     0 ,
        -0.25 ,    0 ,  -0.5 ,
        -0.25 ,    0 , -0.25 ,
            0 ,    0 , -0.25 ,
         -0.5 ,    0 , -0.25 ,
            0 , 0.25 ,  -0.5 ,
         0.25 ,    0 ,   0.5 ,
         0.25 ,    0 ,     0 ,
        -0.25 ,    0 ,   0.5 ,
            0 , 0.25 ,   0.5 ,
         -0.5 , 0.25 ,     0 ,
        -0.25 ,    0 ,     0 ,
        -0.25 , 0.15 ,     0 ,
    };

    static const std::vector<std::vector<uint>> faces =
    {
        { 16 , 37 , 20 , 22      },
        { 20 , 37 , 12 , 21      },
        { 21 , 12 , 35 , 19      },
        { 19 , 35 , 13 , 23      },
        { 22 , 20 , 40 ,  9      },
        { 40 , 20 , 21 ,  0      },
        {  0 , 21 , 19 , 36      },
        { 36 , 19 , 23 ,  2      },
        { 33 ,  9 , 40 , 31      },
        { 31 , 40 ,  0 , 32      },
        { 32 ,  0 , 36 , 26      },
        { 26 , 36 ,  2 , 27      },
        {  8 , 33 , 31 , 30      },
        { 31 , 32 ,  1 , 30      },
        {  1 , 32 , 26 , 25      },
        { 25 , 26 , 27 ,  3      },
        { 14 , 15 ,  6 ,  5      },
        {  4 ,  7 ,  6 ,  5      },
        {  4 , 10 , 11 ,  5      },
        { 14 , 17 , 11 ,  5      },
        { 22 ,  9 , 39           },
        {  9 , 33 , 39           },
        { 20 , 40 , 41           },
        { 40 , 31 , 41           },
        { 21 ,  0 , 18           },
        { 32 ,  0 , 18           },
        { 19 , 36 , 28           },
        { 26 , 36 , 28           },
        { 23 ,  2 , 29           },
        { 27 ,  2 , 29           },
        {  9 , 40 , 41 , 39      },
        { 40 ,  0 , 18 , 41      },
        {  0 , 36 , 28 , 18      },
        { 36 ,  2 , 29 , 28      },
        { 22 , 20 , 41 , 39      },
        { 33 , 31 , 41 , 39      },
        { 20 , 21 , 18 , 41      },
        { 31 , 32 , 18 , 41      },
        { 21 , 19 , 28 , 18      },
        { 32 , 26 , 28 , 18      },
        { 19 , 23 , 29 , 28      },
        { 26 , 27 , 29 , 28      },
        {  1 , 25 , 34           },
        { 12 , 35 , 38           },
        {  1 , 30 , 34           },
        { 12 , 37 , 38           },
        { 18 , 28 , 24           },
        { 18 , 41 , 24           },
        { 34 , 24 ,  5 ,  4      },
        { 38 , 24 ,  5 , 14      },
        { 24 , 28 , 29 ,  6 ,  5 },
        { 24 , 41 , 39 , 11 ,  5 },
        { 18 , 32 ,  1 , 34 , 24 },
        { 28 , 26 , 25 , 34 , 24 },
        { 18 , 21 , 12 , 38 , 24 },
        { 28 , 19 , 35 , 38 , 24 },
        { 41 , 31 , 30 , 34 , 24 },
        { 41 , 20 , 37 , 38 , 24 },
        { 16 , 37 , 38 , 14 , 17 },
        { 13 , 35 , 38 , 14 , 15 },
        {  8 , 30 , 34 ,  4 , 10 },
        {  3 , 25 , 34 ,  4 ,  7 },
        { 22 , 16 , 17 , 11 , 39 },
        { 23 , 13 , 15 ,  6 , 29 },
        { 33 ,  8 , 10 , 11 , 39 },
        { 27 ,  3 ,  7 ,  6 , 29 },
    };

    static const std::vector<std::vector<uint>> polys =
    {
        { 20 , 22 , 30 , 34 ,  4                },
        { 21 , 23 , 30 , 35 ,  8                },
        { 22 , 24 , 31 , 36 ,  5                },
        { 23 , 25 , 31 , 37 ,  9                },
        { 24 , 26 , 32 , 38 ,  6                },
        { 25 , 27 , 32 , 39 , 10                },
        { 26 , 28 , 33 , 40 ,  7                },
        { 27 , 29 , 33 , 41 , 11                },
        { 39 , 42 , 52 , 53 , 46 , 14           },
        { 38 , 46 , 43 , 54 , 55 ,  2           },
        { 37 , 52 , 44 , 56 , 47 , 13           },
        { 36 , 47 , 54 , 45 , 57 ,  1           },
        { 41 , 53 , 65 , 61 , 48 , 50 , 17 , 15 },
        { 40 , 55 , 50 , 63 , 59 , 49 , 16 ,  3 },
        { 35 , 56 , 48 , 64 , 60 , 51 , 18 , 12 },
        { 34 , 57 , 49 , 51 , 62 , 58 , 19 ,  0 },
    };

    static const std::vector<std::vector<bool>> winding =
    {
        { true, true, true, true, true                   },
        { true, true, true, true, true                   },
        { true, true, true, true, true                   },
        { true, true, true, true, true                   },
        { true, true, true, true, true                   },
        { true, true, true, true, true                   },
        { true, true, true, true, true                   },
        { true, true, true, true, true                   },
        { true, true, true, true, true, true             },
        { true, true, true, true, true, true             },
        { true, true, true, true, true, true             },
        { true, true, true, true, true, true             },
        { true, true, true, true, true, true, true, true },
        { true, true, true, true, true, true, true, true },
        { true, true, true, true, true, true, true, true },
        { true, true, true, true, true, true, true, true },
    };
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

namespace Conv_Edge_4_to_2
{
    static const std::vector<double> verts =
    {
           0 ,     0 ,    0 ,
         0.5 ,   0.5 ,  0.5 ,
           0 ,  -0.5 ,    0 ,
         0.5 ,   0.5 ,    0 ,
         0.5 ,     0 ,  0.5 ,
           0 ,   0.5 ,  0.5 ,
           0 ,     0 ,  0.5 ,
         0.5 ,     0 ,    0 ,
           0 ,   0.5 ,    0 ,
         0.5 ,  -0.5 ,  0.5 ,
           0 ,  -0.5 ,  0.5 ,
         0.5 ,  -0.5 ,    0 ,
           0 , -0.25 ,    0 ,
           0 ,  0.25 ,    0 ,
        0.25 ,     0 ,    0 ,
           0 ,     0 , 0.25 ,
    };

    static const std::vector<std::vector<uint>> faces =
    {
        { 11 ,  9 ,  4 ,  7       },
        {  7 ,  4 ,  1 ,  3       },
        {  9 , 10 ,  6 ,  4       },
        {  4 ,  6 ,  5 ,  1       },
        {  8 ,  3 ,  1 ,  5       },
        {  2 , 11 ,  9 , 10       },
        { 12 ,  0 , 14            },
        { 12 ,  0 , 15            },
        { 13 ,  0 , 14            },
        { 13 ,  0 , 15            },
        { 12 , 14 , 15            },
        { 13 , 14 , 15            },
        {  0 , 14 , 15            },
        {  6 ,  4 ,  7 , 14 , 15  },
        {  7 ,  3 ,  8 , 13 , 14  },
        {  6 ,  5 ,  8 , 13 , 15  },
        {  7 , 11 ,  2 , 12 , 14  },
        {  6 , 10 ,  2 , 12 , 15  },
    };

    static const std::vector<std::vector<uint>> polys =
    {
        {  6 ,  7 , 10 , 12              },
        { 12 ,  8 ,  9 , 11              },
        { 11 , 13 , 14 , 15 , 1 , 3 , 4  },
        { 10 , 13 , 16 , 17 , 0 , 2 , 5  },
    };

    static const std::vector<std::vector<bool>> winding =
    {
        { true, true, true, true                   },
        { true, true, true, true                   },
        { true, true, true, true, true, true, true },
        { true, true, true, true, true, true, true },
    };
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

namespace Conc_Edge_4_to_2
{
    static const std::vector<double> verts =
    {
           0 ,     0 ,    0 ,
         0.5 ,   0.5 ,    0 ,
           1 ,  0.25 ,    0 ,
        0.75 ,  -0.5 ,    0 ,
         0.5 ,   0.5 ,  0.5 ,
         0.5 ,     0 ,  0.5 ,
         0.5 ,  -0.5 , 0.25 ,
           1 ,     0 , 0.25 ,
           1 ,   0.5 ,    0 ,
           1 ,     0 ,    0 ,
           1 ,   0.5 ,  0.5 ,
           1 ,     0 ,  0.5 ,
         0.5 ,  -0.5 ,    0 ,
        0.75 ,     0 ,    0 ,
         0.5 ,  -0.5 ,  0.5 ,
        0.75 ,     0 , 0.15 ,
           1 ,  -0.5 ,    0 ,
           1 ,  -0.5 ,  0.5 ,
         0.5 ,     0 , 0.15 ,
        0.75 ,   0.5 ,    0 ,
        0.75 , -0.25 ,    0 ,
         0.5 , -0.25 ,    0 ,
           1 , -0.25 ,    0 ,
        0.75 ,  0.25 ,    0 ,
         0.5 ,     0 , 0.25 ,
         0.5 ,   0.5 , 0.25 ,
         0.5 ,  0.25 ,    0 ,
         0.5 ,     0 ,    0 ,
           0 , -0.25 ,  0.5 ,
           0 ,     0 ,  0.5 ,
           0 ,  0.25 ,  0.5 ,
           0 ,   0.5 ,  0.5 ,
           0 ,  -0.5 , 0.25 ,
           0 , -0.25 , 0.25 ,
           0 ,     0 , 0.25 ,
           0 ,  0.25 , 0.25 ,
           0 ,   0.5 , 0.25 ,
           0 ,  -0.5 ,    0 ,
           0 , -0.25 ,    0 ,
           0 ,  -0.5 ,  0.5 ,
           0 ,  0.25 ,    0 ,
           0 ,   0.5 ,    0 ,
        0.25 ,  -0.5 ,    0 ,
        0.25 , -0.25 ,    0 ,
        0.25 ,     0 ,    0 ,
        0.25 ,  0.25 ,    0 ,
        0.25 ,   0.5 ,    0 ,
        0.15 ,     0 ,  0.5 ,
        0.15 ,     0 , 0.25 ,
        0.25 ,     0 , 0.15 ,
        0.25 ,     0 ,  0.5 ,
        0.25 ,  -0.5 ,  0.5 ,
        0.25 ,   0.5 ,  0.5 ,
           0 , -0.25 ,    1 ,
           0 ,   0.5 , 0.75 ,
        0.25 ,     0 ,    1 ,
           0 ,  -0.5 ,    1 ,
           0 ,     0 ,    1 ,
         0.5 ,  -0.5 ,    1 ,
         0.5 ,     0 ,    1 ,
           0 ,     0 , 0.75 ,
        0.15 ,     0 , 0.75 ,
           0 ,   0.5 ,    1 ,
         0.5 ,   0.5 ,    1 ,
           0 ,  -0.5 , 0.75 ,
           0 ,  0.25 , 0.75 ,
           0 ,  0.25 ,    1 ,
           0 , -0.25 , 0.75 ,
    };

    static const std::vector<std::vector<uint>> faces =
    {
        { 16 ,  3 , 20 , 22           },
        { 20 ,  3 , 12 , 21           },
        { 18 , 15 , 24                },
        { 22 , 16 , 17 , 11 ,  7      },
        { 22 , 20 , 13 ,  9           },
        { 13 , 20 , 21 , 27           },
        { 20 , 21 , 18 , 15           },
        { 18 , 21 , 12 ,  6 , 24      },
        {  2 ,  9 , 13 , 23           },
        { 23 , 13 , 27 , 26           },
        { 12 ,  3 ,  6                },
        {  8 , 19 , 25 ,  4 , 10      },
        {  8 ,  2 , 23 , 19           },
        { 23 , 26 ,  1 , 19           },
        { 23 , 26 , 18 , 15           },
        { 15 , 20 ,  3 ,  6 , 24      },
        { 16 ,  3 ,  6 , 14 , 17      },
        {  2 ,  8 , 10 , 11 ,  7      },
        {  4 , 10 , 11 ,  5           },
        { 14 , 17 , 11 ,  5           },
        { 22 ,  9 ,  7                },
        {  9 ,  2 ,  7                },
        { 20 , 13 , 15                },
        { 13 , 23 , 15                },
        { 21 , 27 , 18                },
        { 26 , 27 , 18                },
        {  1 , 19 , 25                },
        { 15 , 23 , 19 , 25 , 24      },
        { 25 , 24 ,  5 ,  4           },
        { 24 , 15 ,  7 , 11 ,  5      },
        {  9 , 13 , 15 ,  7           },
        { 13 , 27 , 18 , 15           },
        {  6 , 24 ,  5 , 14           },
        { 18 , 26 ,  1 , 25 , 24      },
        { 22 , 20 , 15 ,  7           },
        {  2 , 23 , 15 ,  7           },
        { 28 , 33 , 32 , 39           },
        { 29 , 34 , 33 , 28           },
        { 30 , 35 , 34 , 29           },
        { 31 , 36 , 35 , 30           },
        { 33 , 38 , 37 , 32           },
        { 34 ,  0 , 38 , 33           },
        { 35 , 40 ,  0 , 34           },
        { 36 , 41 , 40 , 35           },
        { 38 , 43 , 42 , 37           },
        {  0 , 44 , 43 , 38           },
        { 40 , 45 , 44 ,  0           },
        { 41 , 46 , 45 , 40           },
        { 43 , 21 , 12 , 42           },
        { 44 , 27 , 21 , 43           },
        { 45 , 26 , 27 , 44           },
        { 46 ,  1 , 26 , 45           },
        { 28 , 29 , 47                },
        { 29 , 30 , 47                },
        { 33 , 34 , 48                },
        { 34 , 35 , 48                },
        { 43 , 44 , 49                },
        { 45 , 44 , 49                },
        { 39 , 32 , 51                },
        { 31 , 36 , 52                },
        { 12 , 42 ,  6                },
        {  1 , 46 , 25                },
        { 29 , 34 , 48 , 47           },
        { 44 , 27 , 18 , 49           },
        { 28 , 33 , 48 , 47           },
        { 30 , 35 , 48 , 47           },
        { 43 , 21 , 18 , 49           },
        { 45 , 26 , 18 , 49           },
        { 47 , 48 , 50                },
        { 18 , 49 , 24                },
        { 49 , 48 , 34 ,  0 , 44      },
        { 49 , 48 , 33 , 38 , 43      },
        { 49 , 48 , 35 , 40 , 45      },
        { 49 , 48 , 50 ,  5 , 24      },
        { 51 , 32 , 33 , 48 , 50      },
        { 52 , 36 , 35 , 48 , 50      },
        {  6 , 42 , 43 , 49 , 24      },
        { 25 , 46 , 45 , 49 , 24      },
        { 39 , 51 , 50 , 47 , 28      },
        { 31 , 52 , 50 , 47 , 30      },
        { 50 , 51 , 14 ,  5           },
        { 50 , 52 ,  4 ,  5           },
        { 51 , 32 , 37 , 42 ,  6 , 14 },
        { 52 , 36 , 41 , 46 , 25 ,  4 },
        { 62 , 54 , 65 , 66           },
        { 65 , 54 , 31 , 30           },
        { 47 , 61 , 50                },
        { 66 , 62 , 63 , 59 , 55      },
        { 66 , 65 , 60 , 57           },
        { 60 , 65 , 30 , 29           },
        { 65 , 30 , 47 , 61           },
        { 53 , 57 , 60 , 67           },
        { 67 , 60 , 29 , 28           },
        { 31 , 54 , 52                },
        { 56 , 64 , 51 , 14 , 58      },
        { 56 , 53 , 67 , 64           },
        { 67 , 28 , 39 , 64           },
        { 67 , 28 , 47 , 61           },
        { 61 , 65 , 54 , 52 , 50      },
        { 62 , 54 , 52 ,  4 , 63      },
        { 53 , 56 , 58 , 59 , 55      },
        { 14 , 58 , 59 ,  5           },
        {  4 , 63 , 59 ,  5           },
        { 66 , 57 , 55                },
        { 57 , 53 , 55                },
        { 65 , 60 , 61                },
        { 60 , 67 , 61                },
        { 39 , 64 , 51                },
        { 61 , 67 , 64 , 51 , 50      },
        { 50 , 61 , 55 , 59 ,  5      },
        { 57 , 60 , 61 , 55           },
        { 60 , 29 , 47 , 61           },
        { 66 , 65 , 61 , 55           },
        { 53 , 67 , 61 , 55           },
    };

    static const std::vector<std::vector<uint>> polys =
    {
        {  20 ,  22 ,  30 ,  34 ,   4                       },
        {  21 ,  23 ,  30 ,  35 ,   8                       },
        {  22 ,  24 ,  31 ,   6 ,   5                       },
        {  23 ,  25 ,  31 ,  14 ,   9                       },
        {   6 ,   2 ,   7 ,  10 ,  15 ,   1                 },
        {  14 ,  33 ,  26 ,  27 ,   2 ,  13                 },
        {  34 ,  15 ,  32 ,  29 ,   3 ,  16 , 19 ,  0       },
        {  35 ,  27 ,  28 ,  17 ,  11 ,  29 , 18 , 12       },
        {  37 ,  52 ,  54 ,  62 ,  64                       },
        {  38 ,  53 ,  55 ,  62 ,  65                       },
        {  41 ,  45 ,  54 ,  56 ,  70 ,  71                 },
        {  42 ,  46 ,  55 ,  57 ,  70 ,  72                 },
        {  49 ,  56 ,  24 ,  63 ,  66                       },
        {  50 ,  57 ,  25 ,  63 ,  67                       },
        {  36 ,  78 ,  64 ,  68 ,  58 ,  74                 },
        {  39 ,  79 ,  65 ,  68 ,  59 ,  75                 },
        {  48 ,   7 ,  66 ,  69 ,  60 ,  76                 },
        {  51 ,  33 ,  67 ,  69 ,  61 ,  77                 },
        {  40 ,  44 ,  74 ,  76 ,  80 ,  32 ,  82 , 73 , 71 },
        {  43 ,  47 ,  75 ,  77 ,  81 ,  28 ,  83 , 73 , 72 },
        { 103 , 105 , 110 , 112 ,  88                       },
        { 104 , 106 , 110 , 113 ,  91                       },
        { 105 ,  53 , 111 ,  90 ,  89                       },
        { 106 ,  52 , 111 ,  97 ,  92                       },
        {  90 ,  86 ,  79 ,  93 ,  98 ,  85                 },
        {  97 ,  78 , 107 , 108 ,  86 ,  96                 },
        { 112 ,  98 ,  81 , 109 ,  87 ,  99 , 102 , 84      },
        { 113 , 108 ,  80 , 100 ,  94 , 109 , 101 , 95      },
    };

    static const std::vector<std::vector<bool>> winding =
    {
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true , true , true , true        },
        { true , true , true , true , true , true , true , true        },
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true , true , true , true , true },
        { true , true , true , true , true , true , true , true , true },
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true , true , true , true        },
        { true , true , true , true , true , true , true , true        },
    };
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

namespace Conv_Vert_4_to_2
{
    static const std::vector<double> verts =
    {
          0 ,     0 ,    0 ,
          0 ,   0.5 , 0.25 ,
          0 ,   0.5 ,  0.5 ,
          0 ,  0.25 ,    0 ,
          0 ,  0.25 , 0.25 ,
          0 ,  0.25 ,  0.5 ,
          0 ,   0.5 ,    0 ,
          0 ,     0 , 0.25 ,
          0 ,     0 ,  0.5 ,
        0.25 ,    0 ,    0 ,
        0.25 ,    0 , 0.25 ,
        0.25 ,    0 ,  0.5 ,
         0.5 ,    0 ,    0 ,
         0.5 ,    0 , 0.25 ,
         0.5 ,    0 ,  0.5 ,
        0.25 , 0.25 ,    0 ,
         0.5 , 0.25 ,    0 ,
        0.25 ,  0.5 ,    0 ,
         0.5 ,  0.5 ,    0 ,
         0.5 ,  0.5 ,  0.5 ,
        0.15 ,  0.5 ,  0.5 ,
        0.25 ,  0.5 ,  0.5 ,
         0.5 , 0.15 ,  0.5 ,
         0.5 , 0.25 ,  0.5 ,
         0.5 ,  0.5 , 0.15 ,
         0.5 ,  0.5 , 0.25 ,
        0.15 , 0.25 ,  0.5 ,
         0.5 , 0.15 , 0.25 ,
        0.25 ,  0.5 , 0.15 ,
         0.5 , 0.15 , 0.75 ,
        0.25 ,  0.5 ,    1 ,
         0.5 ,    0 , 0.75 ,
        0.25 ,    0 ,    1 ,
           0 ,  0.5 , 0.75 ,
           0 ,    0 ,    1 ,
           0 , 0.25 , 0.75 ,
           0 ,    0 , 0.75 ,
           0 ,  0.5 ,    1 ,
         0.5 ,  0.5 ,    1 ,
         0.5 , 0.25 ,    1 ,
        0.25 ,    0 , 0.75 ,
         0.5 ,    0 ,    1 ,
           0 , 0.25 ,    1 ,
        0.15 , 0.75 ,  0.5 ,
         0.5 ,    1 , 0.25 ,
           0 , 0.75 ,  0.5 ,
           0 ,    1 , 0.25 ,
         0.5 , 0.75 ,    0 ,
           0 ,    1 ,    0 ,
        0.25 , 0.75 ,    0 ,
           0 , 0.75 ,    0 ,
         0.5 ,    1 ,    0 ,
         0.5 ,    1 ,  0.5 ,
        0.25 ,    1 ,  0.5 ,
           0 , 0.75 , 0.25 ,
           0 ,    1 ,  0.5 ,
        0.25 ,    1 ,    0 ,
        0.75 ,  0.5 , 0.15 ,
           1 , 0.25 ,  0.5 ,
        0.75 ,  0.5 ,    0 ,
           1 , 0.25 ,    0 ,
        0.75 ,    0 ,  0.5 ,
           1 ,    0 ,    0 ,
        0.75 ,    0 , 0.25 ,
        0.75 ,    0 ,    0 ,
           1 ,    0 ,  0.5 ,
           1 ,  0.5 ,  0.5 ,
           1 ,  0.5 , 0.25 ,
        0.75 , 0.25 ,    0 ,
           1 ,  0.5 ,    0 ,
           1 ,    0 , 0.25 ,
        0.75 ,    0 , 0.75 ,
           1 ,    0 , 0.75 ,
        0.75 ,    0 ,    1 ,
           1 ,    0 ,    1 ,
           1 ,  0.5 ,    1 ,
           0 , 0.75 , 0.75 ,
           0 , 0.75 ,    1 ,
           0 ,    1 , 0.75 ,
           0 ,    1 ,    1 ,
         0.5 ,    1 ,    1 ,
        0.75 , 0.75 ,    0 ,
        0.75 ,    1 ,    0 ,
           1 , 0.75 ,    0 ,
           1 ,    1 ,    0 ,
           1 ,    1 ,  0.5 ,
    };

    static const std::vector<std::vector<uint>> faces =
    {
        {  6 ,  1 ,  4 ,  3           },
        {  1 ,  2 ,  5 ,  4           },
        {  3 ,  4 ,  7 ,  0           },
        {  4 ,  5 ,  8 ,  7           },
        {  0 ,  7 , 10 ,  9           },
        {  7 ,  8 , 11 , 10           },
        {  9 , 10 , 13 , 12           },
        { 10 , 11 , 14 , 13           },
        { 16 , 18 , 17 , 15           },
        { 16 , 15 ,  9 , 12           },
        { 15 ,  3 ,  0 ,  9           },
        { 15 , 17 ,  6 ,  3           },
        {  5 , 26 , 20 ,  2           },
        { 14 , 22 , 27 , 13           },
        { 18 , 24 , 28 , 17           },
        {  5 , 26 ,  4                },
        {  2 , 20 ,  1                },
        { 10 , 27 , 13                },
        { 14 , 22 , 11                },
        { 15 , 28 , 17                },
        { 16 , 24 , 18                },
        {  1 , 20 , 26 ,  4           },
        { 11 , 22 , 27 , 10           },
        { 16 , 24 , 28 , 15           },
        { 20 , 21 , 26                },
        { 22 , 23 , 27                },
        { 24 , 25 , 28                },
        { 21 , 20 ,  1 ,  6 , 17 , 28 },
        { 23 , 22 , 11 ,  8 ,  5 , 26 },
        { 25 , 24 , 16 , 12 , 13 , 27 },
        { 21 , 19 , 25 , 28           },
        { 21 , 19 , 23 , 26           },
        { 23 , 19 , 25 , 27           },
        { 26 ,  4 ,  7 , 10 , 27 , 23 },
        { 27 , 10 ,  9 , 15 , 28 , 25 },
        { 28 , 15 ,  3 ,  4 , 26 , 21 },
        { 22 , 29 , 40 , 11           },
        { 29 , 31 , 40                },
        { 20 , 26 , 35 , 33           },
        { 36 , 40 , 32 , 34           },
        { 30 , 37 , 33 , 20 , 21      },
        { 40 , 31 , 41 , 32           },
        { 22 , 29 , 31 , 14           },
        { 30 , 42 , 35 , 26 , 21      },
        { 21 , 19 , 38 , 30           },
        {  2 ,  5 , 35 , 33           },
        { 30 , 37 , 42                },
        { 23 , 22 , 29                },
        { 39 , 32 , 41                },
        { 19 , 38 , 39 , 29 , 23      },
        { 11 , 14 , 31 , 40           },
        { 32 , 40 , 29 , 39           },
        { 31 , 41 , 39 , 29           },
        { 23 , 29 , 40 , 36 , 35 , 26 },
        { 42 , 30 , 38 , 39 , 32 , 34 },
        {  8 , 11 , 40 , 36           },
        { 33 , 35 , 42 , 37           },
        { 20 ,  2 , 33                },
        { 35 , 36 , 34 , 42           },
        { 26 ,  5 , 35                },
        {  8 ,  5 , 35 , 36           },
        { 20 , 43 , 54 ,  1           },
        { 43 , 45 , 54                },
        { 24 , 28 , 49 , 47           },
        { 50 , 54 , 46 , 48           },
        { 44 , 51 , 47 , 24 , 25      },
        { 54 , 45 , 55 , 46           },
        { 20 , 43 , 45 ,  2           },
        { 44 , 56 , 49 , 28 , 25      },
        { 25 , 19 , 52 , 44           },
        { 18 , 17 , 49 , 47           },
        { 44 , 51 , 56                },
        { 21 , 20 , 43                },
        { 53 , 46 , 55                },
        { 19 , 52 , 53 , 43 , 21      },
        {  1 ,  2 , 45 , 54           },
        { 46 , 54 , 43 , 53           },
        { 45 , 55 , 53 , 43           },
        { 21 , 43 , 54 , 50 , 49 , 28 },
        { 56 , 44 , 52 , 53 , 46 , 48 },
        {  6 ,  1 , 54 , 50           },
        { 47 , 49 , 56 , 51           },
        { 24 , 18 , 47                },
        { 49 , 50 , 48 , 56           },
        { 28 , 17 , 49                },
        {  6 , 17 , 49 , 50           },
        { 24 , 57 , 68 , 16           },
        { 57 , 59 , 68                },
        { 22 , 27 , 63 , 61           },
        { 64 , 68 , 60 , 62           },
        { 58 , 65 , 61 , 22 , 23      },
        { 68 , 59 , 69 , 60           },
        { 24 , 57 , 59 , 18           },
        { 58 , 70 , 63 , 27 , 23      },
        { 23 , 19 , 66 , 58           },
        { 14 , 13 , 63 , 61           },
        { 58 , 65 , 70                },
        { 25 , 24 , 57                },
        { 67 , 60 , 69                },
        { 19 , 66 , 67 , 57 , 25      },
        { 16 , 18 , 59 , 68           },
        { 60 , 68 , 57 , 67           },
        { 59 , 69 , 67 , 57           },
        { 25 , 57 , 68 , 64 , 63 , 27 },
        { 70 , 58 , 66 , 67 , 60 , 62 },
        { 12 , 16 , 68 , 64           },
        { 61 , 63 , 70 , 65           },
        { 22 , 14 , 61                },
        { 63 , 64 , 62 , 70           },
        { 27 , 13 , 63                },
        { 12 , 13 , 63 , 64           },
        { 14 , 61 , 71 , 31           },
        { 61 , 65 , 72 , 71           },
        { 31 , 71 , 73 , 41           },
        { 71 , 72 , 74 , 73           },
        { 71 , 61 , 22 , 29           },
        { 29 , 71 , 31                },
        { 65 , 72 , 58                },
        { 72 , 71 , 29 , 23 , 58      },
        { 41 , 73 , 39                },
        { 39 , 73 , 71 , 29           },
        { 73 , 74 , 75 , 38 , 39      },
        { 74 , 72 , 58 , 66 , 75      },
        { 19 , 66 , 75 , 38           },
        {  2 , 33 , 76 , 45           },
        { 33 , 37 , 77 , 76           },
        { 45 , 76 , 78 , 55           },
        { 76 , 77 , 79 , 78           },
        { 76 , 33 , 20 , 43           },
        { 43 , 76 , 45                },
        { 37 , 77 , 30                },
        { 77 , 76 , 43 , 21 , 30      },
        { 55 , 78 , 53                },
        { 53 , 78 , 76 , 43           },
        { 78 , 79 , 80 , 52 , 53      },
        { 79 , 77 , 30 , 38 , 80      },
        { 19 , 38 , 80 , 52           },
        { 18 , 47 , 81 , 59           },
        { 47 , 51 , 82 , 81           },
        { 59 , 81 , 83 , 69           },
        { 81 , 82 , 84 , 83           },
        { 81 , 47 , 24 , 57           },
        { 57 , 81 , 59                },
        { 51 , 82 , 44                },
        { 82 , 81 , 57 , 25 , 44      },
        { 69 , 83 , 67                },
        { 67 , 83 , 81 , 57           },
        { 83 , 84 , 85 , 66 , 67      },
        { 84 , 82 , 44 , 52 , 85      },
        { 19 , 52 , 85 , 66           },
    };

    static const std::vector<std::vector<uint>> polys =
    {
        {   1 ,  12 ,  15 ,  16 ,  21                         },
        {   7 ,  13 ,  17 ,  18 ,  22                         },
        {   8 ,  14 ,  19 ,  20 ,  23                         },
        {   0 ,  11 ,  24 ,  35 ,  21 ,  19 ,  27             },
        {   3 ,   5 ,  25 ,  33 ,  22 ,  15 ,  28             },
        {   9 ,   6 ,  26 ,  34 ,  23 ,  17 ,  29             },
        {   2 ,  10 ,   4 ,  33 ,  34 ,  35 ,  30 ,  31 ,  32 },
        {  42 ,  37 ,  18 ,  50 ,  36                         },
        {  41 ,  52 ,  37 ,  48 ,  51                         },
        {  39 ,  58 ,  51 ,  53 ,  43 ,  31 ,  44 ,  49 ,  54 },
        {  60 ,  55 ,  36 ,  47 ,  53 ,  28 ,  59             },
        {  56 ,  38 ,  40 ,  46 ,  43 ,  24                   },
        {  45 ,  57 ,  59 ,  12 ,  38                         },
        {  67 ,  62 ,  16 ,  75 ,  61                         },
        {  66 ,  77 ,  62 ,  73 ,  76                         },
        {  64 ,  83 ,  76 ,  78 ,  68 ,  30 ,  69 ,  74 ,  79 },
        {  85 ,  80 ,  61 ,  72 ,  78 ,  27 ,  84             },
        {  81 ,  63 ,  65 ,  71 ,  68 ,  26                   },
        {  70 ,  82 ,  84 ,  14 ,  63                         },
        {  92 ,  87 ,  20 , 100 ,  86                         },
        {  91 , 102 ,  87 ,  98 , 101                         },
        {  89 , 108 , 101 , 103 ,  93 ,  32 ,  94 ,  99 , 104 },
        { 110 , 105 ,  86 ,  97 , 103 ,  29 , 109             },
        { 106 ,  88 ,  90 ,  96 ,  93 ,  25                   },
        {  95 , 107 , 109 ,  13 ,  88                         },
        { 115 , 116 , 111 , 107 ,  42                         },
        { 117 , 118 , 112 ,  90 , 115 ,  47                   },
        {  52 , 113 , 116 , 119 , 120                         },
        { 121 , 120 , 118 , 114 ,  49 ,  94 , 122 , 123       },
        { 128 , 129 , 124 ,  57 ,  67                         },
        { 130 , 131 , 125 ,  40 , 128 ,  72                   },
        {  77 , 126 , 129 , 132 , 133                         },
        { 134 , 133 , 131 , 127 ,  74 ,  44 , 135 , 136       },
        { 141 , 142 , 137 ,  82 ,  92                         },
        { 143 , 144 , 138 ,  65 , 141 ,  97                   },
        { 102 , 139 , 142 , 145 , 146                         },
        { 147 , 146 , 144 , 140 ,  99 ,  69 , 148 , 149       },
    };

    static const std::vector<std::vector<bool>> winding =
    {
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true , true , true               },
        { true , true , true , true , true , true , true               },
        { true , true , true , true , true , true , true               },
        { true , true , true , true , true , true , true , true , true },
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true , true , true , true , true },
        { true , true , true , true , true , true , true               },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true , true , true , true , true },
        { true , true , true , true , true , true , true               },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true , true , true , true , true },
        { true , true , true , true , true , true , true               },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true                             },
        { true , true , true , true , true                             },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true                             },
        { true , true , true , true , true , true , true , true        },
        { true , true , true , true , true                             },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true                             },
        { true , true , true , true , true , true , true , true        },
        { true , true , true , true , true                             },
        { true , true , true , true , true , true                      },
        { true , true , true , true , true                             },
        { true , true , true , true , true , true , true , true        },
    };
}

}
#endif // CINO_HEX_TRANSITION_SCHEMES_H
