xy_t interpolate_xy(xy_t a, xy_t b, double t)
{
	return mad_xy(set_xy(t), sub_xy(b, a), a);
}

xy_t triangle_find_incentre(triangle_t tr)
{
	xy_t p;
	double la, lb, lc, peri;

	// Calc lengths
	la = hypot_xy(tr.b, tr.c);
	lb = hypot_xy(tr.a, tr.c);
	lc = hypot_xy(tr.a, tr.b);
	peri = la + lb + lc;

	p.x = (la * tr.a.x + lb * tr.b.x + lc * tr.c.x) / peri;
	p.y = (la * tr.a.y + lb * tr.b.y + lc * tr.c.y) / peri;

	return p;
}

double triangle_find_incircle_radius(triangle_t tr)
{
	return point_line_distance(tr.a, tr.b, triangle_find_incentre(tr));
}

triangle_t triangle_dilate(triangle_t tr, double d)	// d is in units, not a proportion
{
	double r0, r1, scale;
	xy_t pc;
	triangle_t trs;

	r0 = triangle_find_incircle_radius(tr);
	r1 = r0 + d;
	scale = r1 / r0;

	pc = triangle_find_incentre(tr);
	trs.a = interpolate_xy(pc, tr.a, scale);
	trs.b = interpolate_xy(pc, tr.b, scale);
	trs.c = interpolate_xy(pc, tr.c, scale);

	return trs;
}

double polygon_signed_area(xy_t *p, int p_count)
{
	int i;
	double sum = 0.;
	xy_t p0, p1;

	p0 = p[p_count-1];

	for (i=0; i < p_count; i++)
	{
		p1 = p[i];
		sum += (p1.x - p0.x) * (p1.y + p0.y);		// double the area below the line
		p0 = p1;
	}

	return sum * 0.5;
}
