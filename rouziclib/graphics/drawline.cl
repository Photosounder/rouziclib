/*float4 draw_line_thin_add(global float *le, float4 pv)
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
			v = (erf_fast(rp.x-r1x) - erf_fast(rp.x-r2x)) * 0.5f;
			v *= gaussian(rp.y-r1y);
			pv += v * col;
		}
	}

	return pv;
}*/

float4 draw_line_thin_add(global float *le, float4 pv)
{
	const int2 p = (int2) (get_global_id(0), get_global_id(1));
	const float2 pf = convert_float2(p);
	const float gl = 4.f;	// gaussian drawing limit
	float v, iradius;
	float2 p1, p2, d12, d12t1p, p4;
	float4 col;
	float line_len, d12s, u, d1p, d2p, d3p;

	p1.x = le[0];
	p1.y = le[1];
	p2.x = le[2];
	p2.y = le[3];
	iradius = le[4];
	col.s0 = le[5];
	col.s1 = le[6];
	col.s2 = le[7];
	col.s3 = le[8];

	d12 = p2-p1;
	d12s = d12.x*d12.x + d12.y*d12.y;	// square of the distance
	if (d12s==0.f)
		return pv;
	line_len = native_sqrt(d12s);
	
	d12t1p = (pf-p1) * d12;
	u = (d12t1p.x + d12t1p.y) / d12s;
	p4 = p1 + u * d12;

	d1p = u * line_len;
	d2p = d1p - line_len;
	d3p = fast_distance(p4, pf);
	d1p *= iradius;
	d2p *= iradius;
	d3p *= iradius;

	if (d1p <= -gl)
		return pv;

	if (d2p >= gl)
		return pv;

	v = (erf_fast(d1p) - erf_fast(d2p)) * 0.5f;
	v *= gaussian(d3p);
	pv += v * col;

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
