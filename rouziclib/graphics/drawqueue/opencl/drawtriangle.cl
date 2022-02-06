float erf_right_triangle_acute_integral(float x, float y)
{
	float x2 = x*x, y2 = y*y;
	return y * ( (((((((-1.66388446e-7f*x2 +
		6.129351421e-7f*y2 + 6.6395547e-6f)*x2 +
		-2.1565553e-05f*y2 - 0.000113187232f)*x2 +
		0.00030509557f*y2 + 0.0010927653f)*x2 +
		-0.0021953036f*y2 - 0.0067291297f)*x2 +
		0.00819899f*y2 + 0.028311972f)*x2 +
		(-0.0002794981f*y2 - 0.012583451f)*y2 - 0.08343339f)*x2 +
		(0.00391418f*y2 - 0.009656898f)*y2 + 0.16202268f)*x2 +
		(-0.00082307062f*y2 + 0.0013133343f)*y2 - 0.00037333403f );	// 20 FR
}

float calc_right_triangle_pixel_weight(float2 rp)
{
	float2 rpa;
	int use_obtuse;
	float slope, acute, obtuse;

	rpa = fabs(rp);

	// Pick method
	use_obtuse = rpa.y > rpa.x;
	if (use_obtuse)			// if we use the obtuse method
	{
		// Swap axes
		float t = rp.x;
		rp.x = rp.y;
		rp.y = t;
	}

	// Prepare the arguments (slope and clamped x)
	slope = fabs(rp.x) < 1e-5f ? 0.f : rp.y / rp.x;
	slope = clamp(slope, -1.f, 1.f);
	rp.x = clamp(rp.x, -3.f, 3.f);

	acute = erf_right_triangle_acute_integral(rp.x, slope);
	obtuse = 0.25f * erf_fast(rp.y) * erf_fast(rp.x) - acute;
	acute = copysign(acute, slope);
	obtuse = copysign(obtuse, slope);

	return use_obtuse ? obtuse : acute;
}

float calc_subtriangle_pixel_weight(float2 p0, float2 p1)
{
	float2 rot, r0, r1, np;
	float weight;

	// Rotate points
	rot = fast_normalize(p1 - p0);
	r0.x = rot.x*p0.y - rot.y*p0.x;
	r0.y = rot.x*p0.x + rot.y*p0.y;
	r1.x = rot.x*p1.y - rot.y*p1.x;
	r1.y = rot.x*p1.x + rot.y*p1.y;

	// Calc weights
	weight = calc_right_triangle_pixel_weight(r1);
	weight -= calc_right_triangle_pixel_weight(r0);

	return weight;
}

float4 draw_triangle(global float *le, float4 pv, const float2 pf)
{
	float rad, weight;
	float2 p0, p1, p2;
	float4 col;

	// Load parameters
	rad = le[0];
	col.s0 = le[1];
	col.s1 = le[2];
	col.s2 = le[3];
	col.s3 = 1.f;
	p0.x = le[4];
	p0.y = le[5];
	p1.x = le[6];
	p1.y = le[7];
	p2.x = le[8];
	p2.y = le[9];

	// Transform triangle coordinates
	p0 = (p0 - pf) * rad;
	p1 = (p1 - pf) * rad;
	p2 = (p2 - pf) * rad;

	// Calculate weight for each subtriangle
	weight = calc_subtriangle_pixel_weight(p0, p1);
	weight += calc_subtriangle_pixel_weight(p1, p2);
	weight += calc_subtriangle_pixel_weight(p2, p0);

	// Apply weight to colour
	pv += weight * col;

	return pv;
}

float4 draw_tetragon(global float *le, float4 pv, const float2 pf)
{
	float rad, weight;
	float2 p0, p1, p2, p3;
	float4 col;

	// Load parameters
	rad = le[0];
	col.s0 = le[1];
	col.s1 = le[2];
	col.s2 = le[3];
	col.s3 = 1.f;
	p0.x = le[4];
	p0.y = le[5];
	p1.x = le[6];
	p1.y = le[7];
	p2.x = le[8];
	p2.y = le[9];
	p3.x = le[10];
	p3.y = le[11];

	// Transform triangle coordinates
	p0 = (p0 - pf) * rad;
	p1 = (p1 - pf) * rad;
	p2 = (p2 - pf) * rad;
	p3 = (p3 - pf) * rad;

	// Calculate weight for each subtriangle
	weight = calc_subtriangle_pixel_weight(p0, p1);
	weight += calc_subtriangle_pixel_weight(p1, p2);
	weight += calc_subtriangle_pixel_weight(p2, p3);
	weight += calc_subtriangle_pixel_weight(p3, p0);

	// Apply weight to colour
	pv += weight * col;

	return pv;
}
