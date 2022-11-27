#ifndef H_CMATHUTILS
#define H_CMATHUTILS

#include <stdint.h>
#include <cassert>
#include <cmath>

namespace MathUtils
{
	inline int round_int(double x)
	{
		return (x > 0) ? (int)floor(x + 0.5) : (int)ceil(x - 0.5);
	}

	inline double rint(double x)
	{
		return floor(x+.5);
	}

	inline int truncate_int(double x)
	{
		assert(x > static_cast<double>(INT_MIN / 2) - 1.0);
		assert(x < static_cast<double>(INT_MAX / 2) + 1.0);
		int i;

#if 1 //DISABLE_MATHUTILS_ASM_TRUNCATE_INT
		return i = (int)x;
#endif

		if (x < 0)
			i = -i;

		return (i);
	}

	inline void hack()
	{
		// Stupid hack to keep compiler from dropping these
		// functions as unused
		MathUtils::round_int(0.0);
		MathUtils::truncate_int(0.0);
		MathUtils::rint(0.0);
	}

} // CMathUtils namespace

#endif //H_CMATHUTILS