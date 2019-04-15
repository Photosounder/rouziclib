xy_t interpolate_xy(xy_t a, xy_t b, double t)
{
	return add_xy(a, mul_xy(set_xy(t), sub_xy(b, a)));
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
