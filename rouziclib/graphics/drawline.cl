float gaussian(float x)
{
	return native_exp(-x*x);
}

float erfr(float x)
{
	return 0.5f + 0.5f * erf(x);
}

float4 draw_line_thin_add(global float *le, float4 pv)
{
	const int2 p = (int2) (get_global_id(0), get_global_id(1));
	const float2 pf = convert_float2(p);
	const float gl = 4.f;	// gaussian drawing limit
	float v, r1x, r1y, r2x, costh, sinth;
	float2 rp;
	float4 col;

	r1x = le[0];
	r1y = le[1];
	r2x = le[2];
	costh = le[3];
	sinth = le[4];
	col.s0 = le[5];
	col.s1 = le[6];
	col.s2 = le[7];
	col.s3 = 1.;

	rp.y = pf.x * sinth + pf.y * costh;

	if (rp.y > r1y - gl)
	if (rp.y < r1y + gl)
	{
		rp.x = pf.x * costh - pf.y * sinth;

		if (rp.x > r1x - gl)
		if (rp.x < r2x + gl)
		{
			v = erfr(rp.x-r1x) - erfr(rp.x-r2x);
			v *= gaussian(rp.y-r1y);
			pv += v * col;
		}
	}

	return pv;
}

float4 draw_point_add(global float *le, float4 pv)
{
	const int2 p = (int2) (get_global_id(0), get_global_id(1));
	const float2 pf = convert_float2(p);
	const float gl = 4.f;	// gaussian drawing limit
	float rad, d;
	float2 dp;
	float4 col;

	dp.x = le[0];
	dp.y = le[1];
	rad = le[2];
	col.s0 = le[3];
	col.s1 = le[4];
	col.s2 = le[5];
	col.s3 = 1.;

	d = fast_distance(dp, pf) * rad;	// distance of the pixel from the centre of the dot, scaled

	if (d < gl)
		pv += gaussian(d) * col;

	return pv;
}
