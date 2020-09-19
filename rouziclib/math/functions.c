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

double erfr(double x)
{
	return 0.5 + 0.5*erf(x);
}

double erfinv(double x)		// inverse of erf(x), approximated to 1e-12
{
	double x2, xm, xm2;

	x2 = x*x;
	if (x2 >= 1.)
		return NAN;

	xm = sqrt(-log(1. - x2));	// map x

	if (xm <= 1.11)
	{
		xm2 = xm*xm;
		return copysign( ((((((-1.0392038e-07*xm2 + 7.9276632e-07)*xm2 + 4.2953432e-06)*xm2
			- 6.2023632e-05)*xm2 - 0.0002964260765)*xm2 + 0.0104569355815)*xm2 + 0.88622692544)*xm , x );
	}

	if (xm <= 2.07)
		return copysign( (((((((((2.6405316556026e-05*xm - 0.0004045232079541)*xm
			+ 0.00265824846577055)*xm - 0.00984620428151234)*xm + 0.0231290538999971)*xm
			- 0.0374103333261109)*xm + 0.0411653353348766)*xm - 0.0207904579814415)*xm
			+ 0.0155551797500123)*xm + 0.881638566592708)*xm + 0.00060912524974 ,  x );

	if (xm <= 3.18)
		return copysign( ((((((((((2.0586163032475e-06*xm - 5.2879069722487e-05)*xm
			+ 0.00058379877870049)*xm - 0.00353367458615604)*xm + 0.0120166935937177)*xm
			- 0.0175964777701147)*xm - 0.0252093462336996)*xm + 0.169748375898182)*xm
			- 0.351073864280184)*xm + 0.414567923904984)*xm + 0.629159176809044)*xm + 0.06808965114012 , x );

	if (xm <= 4.14)
		return copysign( ((((((((1.62966625074884e-06*xm - 5.56020704379194e-05)*xm
			+ 0.000837871882335143)*xm - 0.0072897545138698)*xm + 0.0400562055529556)*xm
			- 0.141997736314436)*xm + 0.313406195632118)*xm - 0.37307524815902)*xm
			+ 1.12152434121452)*xm - 0.0533686959291653 , x );

	return copysign( (((((((((6.0964709493827e-09*xm - 3.44268752867462e-07)*xm
			+ 8.82902658930191e-06)*xm - 0.000135723342600147)*xm + 0.00138915485410945)*xm
			- 0.00993254567772973)*xm + 0.0505280426062)*xm - 0.181959803844724)*xm
			+ 0.447643850057175)*xm + 0.327224792874)*xm + 0.28870737275608 , x );
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

int idiv_ceil(int a, int b)	// 30 / 10 returns 3, 31 / 10 returns 4
{
	int d = a / b;

	if (d*b < a)
		d += 1;

	return d;
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
	return sign(n) * (1ULL << log2_ffo64(llabs(n)));
}

int modulo_euclidian(int a, int b)	// gives a modulo that is never negative, as needed for circular buffers
{
	int m;

	m = a % b;

	if (m < 0)
		m = (b < 0) ? m - b : m + b;

	return m;
}

int find_closest_entry_double(double *array, int n, double v)	// in a double array find the entry with the value closest to v
{
	int i, index=-1;
	double d, min = DBL_MAX;

	for (i=0; i < n; i++)
	{
		d = fabs(v-array[i]);
		if (d < min)
		{
			min = d;
			index = i;
		}
	}

	return index;
}

double mix(double v0, double v1, double t)	// linear interpolation
{
	return v0 + (v1-v0)*t;
}

double get_interpolated_xy_array_value(double x, xy_t *array, size_t array_size)
{
	size_t i, step, smin, smax, smid;

	if (array==NULL || array_size < 1)
		return NAN;

	if (x <= array[0].x)
		return array[0].y;

	if (x >= array[array_size-1].x)
		return array[array_size-1].y;

	// Do a binary search of x
	step = next_power_of_2(array_size) >> 1;
	smin = 0;
	smax = array_size-1;

	while (step > 0)
	{
		smid = smin + step;
		smid = MINN(smid, smax);

		if (x < array[smid].x)
			smax = smid;
		else
			smin = smid;

		step >>= 1;
	}

	// Interpolate value
	for (i=MAXN(1, smin); i < array_size; i++)
		if (array[i].x > x)
			return mix(array[i-1].y, array[i].y, (x-array[i-1].x)/(array[i].x-array[i-1].x));

	return array[array_size-1].y;
}
