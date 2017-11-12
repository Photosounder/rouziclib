int32_t fphypot(int32_t x, int32_t y)
{
	return isqrt((int64_t) x*x + (int64_t) y*y);
}

double sq(double x)
{
	return x*x;
}

double gaussian(double x)	// gaussian(x) = e^-x²
{
	return exp(-x*x);
}

double sinc(double x, double fc)		// fc is the cutoff frequency as a multiple of the Nyquist frequency
{
	double a;

	if (x==0.)
		return 1.;

	a = pi*x * fc;
	return sin(a)/(a);
}

double blackman(double x, double range)		// spans [-range , +range]
{
	double a;

	a = x / range;
	ffabs(&a);

	if (a >= 1.)
		return 0.;

	a = 2.*pi * (a*0.5 + 0.5);

	return 0.42 - 0.5*cos(a) + 0.08*cos(2.*a);
}

/*double erf(double x)
{
	// constants
	double t, y;
	double a1 =  0.254829592;
	double a2 = -0.284496736;
	double a3 =  1.421413741;
	double a4 = -1.453152027;
	double a5 =  1.061405429;
	double p  =  0.3275911;

	// Save the sign of x
	int sign = 1;
	if (x < 0)
		sign = -1;
	x = fabs(x);

	// Abramowitz & Stegun formula 7.1.26
	t = 1. / (1. + p*x);
	y = 1. - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);

	return sign*y;
}*/

double erfr(double x)
{
	return 0.5 + 0.5*erf(x);
}

double roundaway(double x)	// round away from 0
{
	if (x < 0.)
		return x-0.5;
	else
		return x+0.5;
}

double rangewrap(double x, double low, double high)
{
	double range;

	range = high-low;

	while (x < low)
		x += range;

	while (x > high)
		x -= range;

	return x;
}

double rangelimit(double x, double min, double max)
{
	if (x < min)
		x = min;

	if (x > max)
		x = max;

	return x;
}

float rangelimitf(float x, float min, float max)
{
	if (x < min)
		x = min;

	if (x > max)
		x = max;

	return x;
}

int32_t rangelimit_i32(int32_t x, int32_t min, int32_t max)
{
	if (x < min)
		x = min;

	if (x > max)
		x = max;

	return x;
}

void swap_double(double *a, double *b)
{
	double c = *a;

	*a = *b;
	*b = c;
}

void swap_i32(int32_t *a, int32_t *b)
{
	int32_t c = *a;

	*a = *b;
	*b = c;
}

void minmax_double(double *a, double *b)
{
	if (*a > *b)
		swap_double(a, b);
}

void minmax_i32(int32_t *a, int32_t *b)
{
	if (*a > *b)
		swap_i32(a, b);
}

double double_add_ulp(double x, int ulp)	// add an integer to the mantissa of a double
{
	uint64_t xi = *((uint64_t *) &x);
	double r;

	xi += ulp;
	r = *((double *) &xi);

	return r;
}

double normalised_notation_split(double number, double *m)	// splits number into m * 10^n
{
	int logv, neg=0;
	double vm, vexp;

	if (number < 0.)
	{
		neg = 1;
		number = -number;
	}

	logv = log10(number);			// 16 million -> 7
	if (number < 1.)
		logv--;

	vexp = pow(10., (double) logv);		// 16 million -> 10,000,000.
	vm = number / vexp;			// 16 million -> 1.6
	if (neg)
		vm = -vm;

	if (m)
		*m = vm;

	return vexp;
}

double fabs_min(double a, double b)
{
	if (sign(a) != sign(b))
		return 0.;		// this gives the absolute minimum over the range

	if (fabs(a) < fabs(b))
		return a;
	else
		return b;
}

double fabs_max(double a, double b)
{
	if (fabs(a) > fabs(b))
		return a;
	else
		return b;
}
