#ifndef DROID_SANS_H
#define DROID_SANS_H

#ifdef CINO_STATIC_LIB
namespace cinolib
{

	extern const unsigned int droid_sans_size;

	extern const unsigned int droid_sans_data[134348 / 4];

}
#else
#include "droid_sans.cpp"
#endif

#endif // DROID_SANS_H
