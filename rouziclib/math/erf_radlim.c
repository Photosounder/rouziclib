double erf_radlim_mid_value(double k)
{
	// target error: 1/20600
	if (k <= 1.25161)
		if (k <= 0.8)
		{
			k *= k;
			return ((0.0609*k - 0.241556)*k + 0.49895)*k;		// err: 4.11e-5 (1 in 24311)
		}
		else
			return ((-0.10807052*k + 0.147360567)*k + 0.39700003)*k - 0.120273435;
	else
		if (k <= 2.16236)
			return ((0.0798091417*k - 0.5408175)*k + 1.241718028)*k - 0.467828544;
		else
			return ((0.0127657*k - 0.108662)*k + 0.3090347)*k + 0.2061557;
}

/*double erf_radlim_lim0_approx(double x)	// approx of 1 - ((x*sqrt(1 - x^2) + asin(x))/2*4/pi)^2, the modified limit for k -> 0 in 1-erf_radlim(x, k)^2
{
	ffabs(&x);
	
	if (x >= 1.)
		return 0.;

	if (x <= 0.675)
	{
		x *= x;
		return (0.57267*x - 1.627166)*x + 1.00016;			// err: 1.81e-4 (1 in 5515)
	}

	if (x <= 0.93)
		return ((3.30203*x - 6.919688)*x + 3.361511)*x + 0.24595;	// err: 1.83e-4 (1 in 5471)

	return ((35.568676*x - 97.9614102)*x + 89.031507)*x - 26.638953;	// err: 1.80e-4 (1 in 5541)
}*/

double erf_radlim_lim0_log_approx(double x)	// approx of log(1 - ((x*sqrt(1 - x^2) + asin(x))/2*4/pi)^2), the modified limit for k -> 0 in 1-erf_radlim(x, k)^2
{
	ffabs(&x);
	
	if (x >= 1.)
		return -1000.;

	if (x <= 0.6)
	{
		x *= x;
		return ((-0.95263*x - 0.657815)*x - 1.630125)*x;						// post-exp err: 1.96e-4 (1 in 5104)
	}

	if (x <= 0.85)
		return (((-78.50466*x + 199.313014)*x - 196.290907)*x + 85.200667)*x - 14.049975;		// post-exp err: 1.62e-4 (1 in 6187)

	if (x <= 0.96)
		return (((-5993.11503*x + 20937.47635)*x - 27476.96981)*x + 16041.94669)*x - 3515.35683;	// post-exp err: 1.42e-4 (1 in 7031)

	if (x <= 0.994)
		return ((-50251.51547*x + 145454.25505)*x - 140382.9389)*x + 45172.35043;			// post-exp err: 1.73e-4 (1 in 5788)

	return -431.132*x + 421.829;	// post-exp err: 9.69e-5 (1 in 10322)
}

/*double erf_radlim_liminf_approx(double x)	// approx of 1 - erf(x)^2, the modified limit for k -> +inf in 1-erf_radlim(x, k)^2
{
	ffabs(&x);

	if (x <= 1.)
		if (x <= 0.5)
		{
			x *= x;
			return (0.715*x - 1.26113)*x + 0.999835;				// err: 1.65e-4 (1 in 6054)
		}
		else
			return ((0.574485*x - 1.049292)*x - 0.309837)*x + 1.0747264;		// err: 2.28e-4 (1 in 4388)
	else
		if (x <= 2.)
			return ((-0.197166*x + 1.219302)*x - 2.558627)*x + 1.82658;		// err: 2.33e-4 (1 in 4290)
		else
			if (x < 3.)
				return ((-0.02050837*x + 0.1692938)*x - 0.466051)*x + 0.428151;	// err: 1.76e-4 (1 in 5683)
			else
				return 0.;
}*/

double erf_radlim_liminf_log_approx(double x)	// approx of log(1 - erf(x)^2), the modified limit for k -> +inf in 1-erf_radlim(x, k)^2
{
	x *= x;
	return ((-0.0028207*x + 0.03688)*x - 1.27238)*x;
}

double erf_radlim_lim0_weight(double k)
{
	// target error: 1/1474
	if (k <= 2.056)
		if (k <= 1.6)
		{
			k *= k;
			return (-0.0273679*k - 0.0153797)*k + 0.9998825;		// err: 4.59e-4 (1 in 2179)
		}
		else
			return (-0.26659807*k + 0.35105132)*k + 0.9029234;
	else
		if (k <= 2.43166)
			return (0.1574509*k - 1.39605301)*k + 2.70381;
		else
			return (0.41673568*k - 2.6250933)*k + 4.15970445;
}

double erf_radlim_liminf_weight(double k)
{
	// target error: 1/2291
	if (k <= 1.88705)
		if (k <= 1.38)
		{
			k *= k;
			return (0.0024849*k + 0.103296)*k + 0.4428941;			// err: 1 in 2461
		}
		else
			return (0.01935059*k + 0.26057699)*k + 0.25137187;
	else
		if (k <= 2.44995)
			return (-0.14819569*k + 0.8923482)*k - 0.34505976;
		else
			return (-0.12126571*k + 0.7387307)*k - 0.130267;
}

double erf_radlim_approx(double x, double k)
{
	double xd, y, mid_v;

	if (k > 3.)
		return fasterfrf_d1(x);

	xd = x/k;
	mid_v = erf_radlim_mid_value(k);
	y = fastexp_limited((erf_radlim_lim0_log_approx(xd)*erf_radlim_lim0_weight(k) + erf_radlim_liminf_log_approx(x)*erf_radlim_liminf_weight(k)));
	y = sqrt(1. - y) * sign(x);
	y = y*mid_v + mid_v;

	return y;
}
