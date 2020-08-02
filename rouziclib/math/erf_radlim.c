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

// GPU versions
float erf_radlim_mid_value_gpu(float k)		// 7 FR
{
	//k *= k;
	return 0.5f*(1.f - expf(-k));
}

float erf_radlim_lim0_weight_gpu(float k)	// 10 FR
{
	//k *= k;
	return ((((-5.0363143e-05f*k + 0.00089379837f)*k - 0.0018122363f)*k - 0.03032174f)*k - 0.00882f)*k + 0.9986833f;
	//return (((((6.934e-06f*k - 0.00023759f)*k + 0.00278945f)*k - 0.0106586f)*k - 0.01166f)*k - 0.023219f)*k + 1.000483f;
}

float erf_radlim_liminf_weight_gpu(float k)	// 10 FR
{
	//k *= k;
	return ((((-7.92e-06f*k + 0.0003287f)*k - 0.004255f)*k + 0.013734f)*k + 0.0946345f)*k + 0.44397133f;
}

float erf_radlim_lim0_log_approx_gpu(float x)	// 11 FR
{
	//x *= x;
	return logf(((0.0124677f*x + 0.06112589f)*x - 1.0734751f)*x + 1.f) * 1.509421f;
}

float erf_radlim_liminf_log_approx_gpu(float x)	// 5 FR
{
	//x *= x;
	return ((-0.0028207f*x + 0.03688f)*x - 1.27238f)*x;
}

float erf_fast_gpu(float x)			// 17 FR or more (copysign?)
{
	float y, xa = fabsf(x);

	// erf(x) ~= 1 - 1/( x*(x*(x*0.0038004543 + 0.020338153) + 0.03533611) + 1.0000062 )^32 for x >= 0, max error 0.35e-3
	y = xa*(xa*(xa * 0.0038004543f + 0.020338153f) + 0.03533611f) + 1.0000062f;

	// y = y^32
	y = y*y;
	y = y*y;
	y = y*y;
	y = y*y;
	y = y*y;

	y = 1.f - 1.f/y;
	y = copysignf(y, x);

	return y;
}

float erf_radlim_approx_gpuf(float x, float k)
{
	float y, mid_v, x2, k2, xd2;

	// is this part needed? Probably not
	if (k > 3.f)					// 16 FR?
		return 0.5f+0.5f*erf_fast_gpu(x);	// 19 FR or more

	// rest is >=64 FR
	x2 = x*x;
	k2 = k*k;
	xd2 = x2/k2;
	xd2 = rangelimitf(xd2, 0.f, 1.f);

	mid_v = erf_radlim_mid_value_gpu(k2);	// 7 FR

	y = expf((erf_radlim_lim0_log_approx_gpu(xd2)*erf_radlim_lim0_weight_gpu(k2) + erf_radlim_liminf_log_approx_gpu(x2)*erf_radlim_liminf_weight_gpu(k2)));	// 43 FR
	y = copysignf(sqrtf(1. - y), x);	// 6 FR or more (copysign?)
	y = y*mid_v + mid_v;			// 2 FR

	return y;
}

double erf_radlim_approx_gpu(double x, double k)
{
	return erf_radlim_approx_gpuf(x, k);
}

// Calculating pixel weights for triangles

double point_line_distance_signed_presub(xy_t l1, xy_t l2)	// distance to the nearest point on the line
{
	return (l2.x*l1.y - l1.x*l2.y) / hypot_xy(l1, l2);	// double of the area of the triangle / length of line
}

double calc_triangle_pixel_weight(triangle_t tr, xy_t p, double drawing_thickness)	// triangle points should be clockwise
{
	xyz_t ld, lda;
	xy_t thick_mul = set_xy(1./drawing_thickness);
	int influence_count=0;
	double ld0, ld1, pd;

	// Offset and scale points
	tr.a = mul_xy(sub_xy(tr.a, p), thick_mul);
	tr.b = mul_xy(sub_xy(tr.b, p), thick_mul);
	tr.c = mul_xy(sub_xy(tr.c, p), thick_mul);

	// Line distance
	ld.x = point_line_distance_signed_presub(tr.a, tr.b);
	ld.y = point_line_distance_signed_presub(tr.b, tr.c);
	ld.z = point_line_distance_signed_presub(tr.c, tr.a);

	if (ld.x < -3. || ld.y < -3. || ld.z < -3.)			// if we're outside of the triangle's influence
		return 0.;

	// Absolute line distance
	lda = ld;
	ffabs(&lda.x);
	ffabs(&lda.y);
	ffabs(&lda.z);

	// Count influencing lines
	influence_count  = (lda.x <= 3.);
	influence_count += (lda.y <= 3.);
	influence_count += (lda.z <= 3.);

	switch (influence_count)
	{
		case 0:							// if we're inside the triangle far from every line
			return 1.;
			
		case 1:							// if only one line is close to the point
			if (lda.x <= 3.)	ld0 = ld.x;		// find the distance of that line
			if (lda.y <= 3.)	ld0 = ld.y;
			if (lda.z <= 3.)	ld0 = ld.z;
			return fasterfrf_d1(ld0);			// return the raised error function of that signed distance

		case 2:							// if two lines influence the point
			if (lda.x <= 3. && lda.y <= 3.)	{ ld0 = ld.x;	ld1 = ld.y;	pd = hypot_d_xy(tr.b);	}	// find the distance to each line and their intersection
			if (lda.y <= 3. && lda.z <= 3.)	{ ld0 = ld.y;	ld1 = ld.z;	pd = hypot_d_xy(tr.c);	}
			if (lda.z <= 3. && lda.x <= 3.)	{ ld0 = ld.z;	ld1 = ld.x;	pd = hypot_d_xy(tr.a);	}

			// Weighted area is full limited-radius area minus areas each line excludes
			return erf_radlim_approx(pd, pd) - erf_radlim_approx(-ld0, pd) - erf_radlim_approx(-ld1, pd);

		case 3:
			return 0.;
	}

	return NAN;
}
