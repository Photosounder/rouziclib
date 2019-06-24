int32_t fphypot(int32_t x, int32_t y)
{
	return isqrt((int64_t) x*x + (int64_t) y*y);
}

double sq(double x)
{
	return x*x;
}

float sqf(float x)
{
	return x*x;
}

double gaussian(double x)	// gaussian(x) = e^-x²
{
	return exp(-x*x);
}

// approximation of erf(x) = (sgn(x) * sqrt(1-gaussian(x)) * (1+gaussian(x*0.886)*0.1283792))
// also erf(x) ~= sgn(x) * sqrt(1 - gaussian(x) / (sqrt(x^2+c0)+1-sqrt(c0)) ), c0 = 3.1220878, max subtractive error 0.958e-3
// ~= sqrt( 1. - gaussian(x) / (sqrt(x*x+3.1220878) - 0.76694306642857) )
// gaussian(x) / (1-erf(x)^2)
// sqrt(x^2+3)-sqrt(3)+1

double erfr(double x)
{
	return 0.5 + 0.5*erf(x);
}

double gamma_dist(double x, double a, double b)
{
	return pow(b, a) * pow(x, a-1.) * exp(-b*x) / tgamma(a);
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

	if (isnan(x))
		return x;

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

int ceil_rshift(int v, int sh)	// does the ceiling version of a right shift, for instance ceil_rshift(65, 3) => 9
{
	int mask = (1 << sh) - 1;

	return (v >> sh) + ((v & mask)!=0);
}

int find_largest_prime_factor(int n)
{
	int i = 2;

	while (n > 1)
	{
		if (n % i == 0)
			n /= i;
		else
			i++;
	}

	return i;
}

int is_prime(int n)
{
	int i;

	if (n <= 3)
		return n > 1;
	else if ((n % 2 == 0) || (n % 3) == 0)
		return 0;

	for (i=5; i*i <= n; i+=6)
		if ((n % i == 0) || (n % (i+2) == 0))
			return 0;

	return 1;
}

int next_prime(int n)
{
	while (is_prime(n)==0)
		n++;

	return n;
}

int64_t next_power_of_2(int64_t n)
{
	return sign(n) * (1ULL << log2_ffo64(abs(n)));
}

int modulo_euclidian(int a, int b)	// gives a modulo that is never negative, as needed for circular buffers
{
	int m;
	
	m = a % b;

	if (m < 0)
		m = (b < 0) ? m - b : m + b;

	return m;
}
