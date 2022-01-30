float erf_fast_gpu(float x)
{
	float y, xa = MINN(fabsf(x), 4.45f);

	y = ((((-0.001133515f*xa + 0.00830125f)*xa - 0.003218f)*xa - 0.06928657f)*xa - 0.14108256f)*xa + 1.f;

	// y = y^8
	y = y*y;
	y = y*y;
	y = y*y;

	y = 1.f - y;
	y = copysign(y, x);

	return y;
}

// Calculating pixel weights for triangles

double point_line_distance_signed_presub(xy_t l1, xy_t l2)	// distance to the nearest point on the line
{
	return (l2.x*l1.y - l1.x*l2.y) / hypot_xy(l1, l2);	// double of the area of the triangle / length of line
}

double calc_triangle_pixel_weight_old(triangle_t tr, xy_t p, double drawing_thickness)	// triangle points should be clockwise
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
			/*if (lda.x <= 3. && lda.y <= 3.)	{ ld0 = ld.x;	ld1 = ld.y;	pd = hypot_d_xy(tr.b);	}	// find the distance to each line and their intersection
			if (lda.y <= 3. && lda.z <= 3.)	{ ld0 = ld.y;	ld1 = ld.z;	pd = hypot_d_xy(tr.c);	}
			if (lda.z <= 3. && lda.x <= 3.)	{ ld0 = ld.z;	ld1 = ld.x;	pd = hypot_d_xy(tr.a);	}

			// Weighted area is full limited-radius area minus areas each line excludes
			return erf_radlim_approx(pd, pd) - erf_radlim_approx(-ld0, pd) - erf_radlim_approx(-ld1, pd);*/

		case 3:
			return 0.;
	}

	return NAN;
}
