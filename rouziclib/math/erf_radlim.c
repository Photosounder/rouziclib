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

double erf_radlim_lim0_approx(double x)	// approx of 1 - ((x*sqrt(1 - x^2) + asin(x))/2*4/pi)^2, the modified limit for k -> 0 in 1-erf_radlim(x, k)^2
{
	double x2;

	ffabs(&x);
	
	if (x >= 1.)
		return 0.;

	if (x <= 0.675)
	{
		x2 = x*x;
		return (0.57267*x2 - 1.627166)*x2 + 1.00016;			// err: 1.81e-04 (1 in 5515)
	}

	if (x <= 0.93)
		return ((3.30203*x - 6.919688)*x + 3.361511)*x + 0.24595;	// err: 1.83e-04 (1 in 5471)

	return ((35.568676*x - 97.9614102)*x + 89.031507)*x - 26.638953;	// err: 1.80e-04 (1 in 5541)
}

double erf_radlim_lim0_log_approx(double x)	// approx of log(1 - ((x*sqrt(1 - x^2) + asin(x))/2*4/pi)^2), the modified limit for k -> 0 in 1-erf_radlim(x, k)^2
{
	double x2;

	ffabs(&x);
	
	if (x >= 1.)
		return 0.;

	if (x <= 0.6)
	{
		x2 = x*x;
		return ((-0.95263*x2 - 0.657815)*x2 - 1.630125)*x2;						// post-exp err: 1.96e-4 (1 in 5104)
	}

	if (x <= 0.85)
		return (((-78.50466*x + 199.313014)*x - 196.290907)*x + 85.200667)*x - 14.049975;		// post-exp err: 1.62e-4 (1 in 6187)

	if (x <= 0.96)
		return (((-5993.11503*x + 20937.47635)*x - 27476.96981)*x + 16041.94669)*x - 3515.35683;	// post-exp err: 1.42e-4 (1 in 7031)

	if (x <= 0.994)
		return ((-50251.51547*x + 145454.25505)*x - 140382.9389)*x + 45172.35043;			// post-exp err: 1.73e-4 (1 in 5788)

	return -431.132*x + 421.829;	// post-exp err: 9.69e-5 (1 in 10322)
}

double erf_radlim_liminf_approx(double x)	// approx of 1 - erf(x)^2, the modified limit for k -> +inf in 1-erf_radlim(x, k)^2
{
	double x2;

	ffabs(&x);

	if (x <= 1.)
		if (x <= 0.5)
		{
			x2 = x*x;
			return (0.715*x2 - 1.26113)*x2 + 0.999835;				// err: 1.65e-04 (1 in 6054)
		}
		else
			return ((0.574485*x - 1.049292)*x - 0.309837)*x + 1.0747264;		// err: 2.28e-04 (1 in 4388)
	else
		if (x <= 2.)
			return ((-0.197166*x + 1.219302)*x - 2.558627)*x + 1.82658;		// err: 2.33e-04 (1 in 4290)
		else
			if (x < 3.)
				return ((-0.02050837*x + 0.1692938)*x - 0.466051)*x + 0.428151;	// err: 1.76e-04 (1 in 5683)
			else
				return 0.;
}

double erf_radlim_liminf_log_approx(double x)	// approx of log(1 - erf(x)^2), the modified limit for k -> +inf in 1-erf_radlim(x, k)^2
{
	double x2 = x*x;

	return ((-0.0028207*x2 + 0.03688)*x2 - 1.27238)*x2;
}

double erf_radlim_lim0_weight(double k)
{
	// target error: 1/337.21
	if (k <= 1.70934)
		if (k <= 0.93951)
			return (-0.11246*k + 0.037)*k + 1.9978;
		else
			return (-0.58987*k + 0.94431)*k + 1.56086;
	else
		if (k <= 2.22854)
			return (-0.305691*k - 0.154436)*k + 2.6089;
		else
			return (0.804468*k - 5.091863)*k + 8.10453;
}

double erf_radlim_liminf_weight(double k)
{
	// target error: 1/768.584
	if (k <= 1.76863)
		if (k <= 1.12855)
			return (0.22099*k - 0.01024)*k + 0.886984;
		else
			return (0.13291*k + 0.22598)*k + 0.732265;
	else
		if (k <= 2.35951)
			return (-0.244429*k + 1.56298)*k - 0.45466;
		else
			return (-0.262188*k + 1.58664)*k - 0.411596;
}
