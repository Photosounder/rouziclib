int32_t fphypot(int32_t x, int32_t y)
{
	return isqrt((int64_t) x*x + (int64_t) y*y);
}

double distance_xy(double dx, double dy)
{
	return sqrt(dx*dx + dy*dy);
}

double gaussian(double x)	// gaussian(x) = e^-x²
{
	return exp(-x*x);
}

double erf(double x)
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
