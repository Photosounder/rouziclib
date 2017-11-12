float4 draw_rect_full_add(global float *le, float4 pv)
{
	const int2 p = (int2) (get_global_id(0), get_global_id(1));
	const float2 pf = convert_float2(p);
	float rad, d;
	float2 p0, p1, d0, d1, gv;
	float4 col;

	p0.x = le[0];
	p0.y = le[1];
	p1.x = le[2];
	p1.y = le[3];
	rad = le[4];
	col.s0 = le[5];
	col.s1 = le[6];
	col.s2 = le[7];
	col.s3 = 1.;

	d0 = (pf - p0) * rad;
	d1 = (pf - p1) * rad;

	gv.x = erfr(d0.x) - erfr(d1.x);
	gv.y = erfr(d0.y) - erfr(d1.y);

	pv += gv.x*gv.y * col;

	return pv;
}

float4 draw_black_rect(global float *le, float4 pv)
{
	const int2 p = (int2) (get_global_id(0), get_global_id(1));
	const float2 pf = convert_float2(p);
	float rad, d;
	float2 p0, p1, d0, d1, gv;

	p0.x = le[0];
	p0.y = le[1];
	p1.x = le[2];
	p1.y = le[3];
	rad = le[4];

	d0 = (pf - p0) * rad;
	d1 = (pf - p1) * rad;

	gv.x = erfr(d0.x) - erfr(d1.x);
	gv.y = erfr(d0.y) - erfr(d1.y);

	pv *= 1.f - gv.x*gv.y;

	return pv;
}

