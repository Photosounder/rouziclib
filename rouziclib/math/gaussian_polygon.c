double **erf_polygon_ref_polynomial(xyi_t deg2d, xy_t range, int opt_transformation)
{
	int i, slope_count = 400, p_count = 400, degree;
	xyi_t id;
	ddouble_t x, slope, v;
	const ddouble_t integral_scale = {0.28209479177387814, 3.8338649032914703e-18};	// the line integral is scaled by 1 / (2*sqrt(pi))

	degree = 60;
	ddouble_t *line_v = calloc(p_count, sizeof(ddouble_t));
	ddouble_t *cm = calloc(degree+2, sizeof(ddouble_t));
	ddouble_t *cmi = calloc(degree+3, sizeof(ddouble_t));
	double **im = (double **) calloc_2d_contig(slope_count, p_count, sizeof(double));
	double **cm2d = (double **) calloc_2d_contig(deg2d.y, deg2d.x, sizeof(double));

	// Make the integral of the polynomial for every slope
	for (id.y=0; id.y < slope_count; id.y++)
	{
		// Slope in the range ]-range.y , range.y[
		slope = mul_qd(chebyshev_node_q(slope_count, id.y), range.y);

		// Compute the derivative's points for the given slope
		for (i=0; i < p_count; i++)
		{
			x = mul_qd(chebyshev_node_q(p_count, i), range.x);
			line_v[i] = mul_qq(mul_qq(erf_q(mul_qq(x, slope)), gaussian_q(x)), integral_scale);
		}

		// Compute the Chebyshev coefficients to fit the derivative
		for (id.x=0; id.x <= degree+1; id.x++)
			cm[id.x] = chebyshev_multiplier_by_dct_q(line_v, p_count, id.x);	// get the Chebyshev multiplier for degree id

		// Make Chebyshev polynomial of integral for the given slope
		cmi[0] = Q_ZERO;
		integrate_chebyshev_coefs_q(cm, degree+1, cmi, ddouble(2.*range.x));

		// Make the polynomial be 0 at x=0
		cmi[0] = neg_q(eval_chebyshev_polynomial_q(Q_ZERO, cmi, degree));

		// Evaluate the polynomial of the integral
		for (i=0; i < p_count; i++)
		{
			// Evaluate
			x = chebyshev_node_q(p_count, i);
			v = eval_chebyshev_polynomial_q(x, cmi, degree);

			// Apply optional transformations (in this case the transformations for the limits interpolation approach)
			ddouble_t x2, y2, e0, e1;
			if (opt_transformation == 1 || opt_transformation == 2)
			{
				x = mul_qd(x, range.x);
				x2 = sq_q(x);
				y2 = sq_q(slope);
				v = div_qq(v, slope);
				e0 = mul_qq(sub_dq(1., gaussian_q(x)), Q_1_2PI);
				e1 = mul_qd_simple(sq_q(erf_q(x)), 1./8.);
				v = div_qq(sub_qq(v, e0), sub_qq(e1, e0));
			}

			if (opt_transformation == 2)
			{
				v = sub_qd(div_qq(v, y2), 1.);
				v = div_qq(v, x2);
			}

			if (opt_transformation == 3)
			{
				x2 = sq_q(x);
				v = div_qq(v, x2);
				v = div_qq(v, slope);
			}

			// Store
			im[id.y][i] = v.hi;
		}
	}

	// Make a 2D Chebyshev polynomial of the whole image
	cm2d = chebyshev_fit_on_points_by_dct_2d(im, xyi(p_count, slope_count), deg2d);

	free(line_v);
	free(cm);
	free(cmi);
	free_2d(im, 1);
	
	return cm2d;
}

double eval_erf_polygon_ref_polynomial(double x, double y, int opt_transformation)
{
	static double **c = NULL;
	double **cm;
	xyi_t degree = xyi(40, 21);
	xy_t range = xy(3.1, 1.1);

	if (c == NULL)
	{
		// Compute Chebyshev coefs of integral
		cm = erf_polygon_ref_polynomial(degree, range, opt_transformation);
		c = (double **) calloc_2d_contig(degree.y+1, degree.x+1, sizeof(double));

		// Convert Chebyshev coefs to regular 2D polynomial coefs
		chebyshev_coefs_to_polynomial_2d(cm, degree, neg_xy(range), range, c);
		free_2d(cm, 1);
	}

	// Evaluate polynomial
	double z = eval_polynomial_2d(xy(x, y), c, degree);

	return z;
}
