double erf_radlim_end_x(double k)	// gives the end x value for a given k (radlim) for erf_radlim
{
	static const double lut[] = 
	#include "erf_radlim/end_value_lut.h"
	const int64_t ish = 52-lutsp;
	int lutind;

	if (k < 0.)
		return 0.;

	if (k > 3.999)
		return 1.;

	lutind = double_get_mantissa(k + 4.) >> ish;
	return polynomial_from_lut(lut, lutind, order, k);
}
