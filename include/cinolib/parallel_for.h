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
#ifndef CINO_PARALLEL_FOR_H
#define CINO_PARALLEL_FOR_H

#include <sys/types.h>
#include <cinolib/cino_inline.h>

namespace cinolib
{

/* OpenMP-like parallel for loop realized in plain C++11
 * Thanks to Jeremy Dumas for his code (https://ideone.com/Z7zldb)
 *
 * DISCLAIMER: this routine is as good as OpenMP with STATIC scheduling.
 * That is, it works best when computational cost is equally distributed
 * across all threads. In case of unbalanced distribution, performances
 * may degrade, and using the DYNAMIC scheduling of a library such as OpenMP
 * will greatly benefit performances.
 *
 * PARALLEL_FOR has three arguments
 *
 *     beg,end             : define a range of indices
 *     serial_if_less_than : avoid paying the overhead if the range is smaller than...
 *     func                : is the function that implements the body of the loop.
 *                           It takes as unique argument the loop index. This will
 *                           typically be a lambda function inlined in the call
 *
 * Example of usage: update normals on a mesh.
 * Given a polygonmesh m, the classical serial loop would be like:
 *
 * for(int pid=0; i<m.num_polys(); ++pid)
 * {
 *     m.update_p_normal(pid);
 * }
 *
 * Normal computation is indeed local to each polygon,
 * therefore this code can be parallelized with the call:
 *
 * PARALLEL_FOR(0, m.num_polys(), 1000, [&m](int pid)
 * {
 *    m.update_p_normal(pid);
 * });
 *
 * NOTE: if symbol SERIALIZE_PARALLEL_FOR is defined at compilation time,
 * the loop will be executed in standard serial mode.
*/

template<typename Func>
inline void PARALLEL_FOR(      unsigned int   beg,
                               unsigned int   end,
                         const unsigned int   serial_if_less_than,
                         const Func & func);
}

#include <thread>
#include <vector>
#include <cmath>

namespace cinolib
{

    template<typename Func>
    inline void PARALLEL_FOR(unsigned int   beg,
        unsigned int   end,
        const unsigned int   serial_if_less_than,
        const Func& func)
    {
#ifndef SERIALIZE_PARALLEL_FOR

        unsigned int n = end - beg + 1;

        if (n < serial_if_less_than)
        {
            for (unsigned int i = beg; i < end; ++i) func(i);
        }
        else
        {
            // estimate number of threads in the pool
            const static unsigned n_threads_hint = std::thread::hardware_concurrency();
            const static unsigned n_threads = (n_threads_hint == 0u) ? 8u : n_threads_hint;

            // split the full range into sub ranges of equal size
            unsigned int slice = (unsigned int)std::round(n / static_cast<double>(n_threads));
            slice = std::max(slice, unsigned int(1));

            // helper function that handles a sub range
            auto subrange_helper = [&func](unsigned int k1, unsigned int k2)
            {
                for (unsigned int k = k1; k < k2; ++k) func(k);
            };

            // create pool and launch jobs
            std::vector<std::thread> pool;
            pool.reserve(n_threads);
            unsigned int i1 = beg;
            unsigned int i2 = std::min(beg + slice, end);
            for (unsigned int i = 0; i + 1 < n_threads && i1 < end; ++i)
            {
                pool.emplace_back(subrange_helper, i1, i2);
                i1 = i2;
                i2 = std::min(i2 + slice, end);
            }
            if (i1 < end) pool.emplace_back(subrange_helper, i1, end);

            // Wait for jobs to finish
            for (std::thread& t : pool)
            {
                if (t.joinable()) t.join();
            }
        }
#else
        for (unsigned int i = beg; i < end; ++i) func(i);
#endif
    }

}

#endif // CINO_PARALLEL_FOR_H
