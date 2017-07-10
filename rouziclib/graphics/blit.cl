bool check_image_bounds(int2 pi, int2 im_dim)
{
	if (pi.x >= 0 && pi.x < im_dim.x)
		if (pi.y >= 0 && pi.y < im_dim.y)
			return true;
	return false;
}

float4 image_interp_nearest(global float4 *im, int2 im_dim, float2 pif)
{
	float4 pv;
	int2 pi;

	pi = convert_int2(pif + 0.5f);

	if (check_image_bounds(pi, im_dim))
		pv = im[pi.y * im_dim.x + pi.x];
	else
		pv = 0.f;

	return pv;
}

float4 get_image_pixel_weighted_linear(global float4 *im, int2 im_dim, float2 pif, float2 pjf)
{
	float4 pv;
	float2 w;
	int2 pj = convert_int2(pjf);

	w = 1.f - fabs(pif-pjf);		// weight for current pixel

	if (check_image_bounds(pj, im_dim))
		pv = im[pj.y * im_dim.x + pj.x] * (w.x*w.y);
	else
		pv = 0.f;

	return pv;
}

float4 image_interp_linear(global float4 *im, int2 im_dim, float2 pif)
{
	float4 pv = 0.f;
	float2 pif00;

	pif00 = floor(pif);

	pv += get_image_pixel_weighted_linear(im, im_dim, pif, pif00);
	pv += get_image_pixel_weighted_linear(im, im_dim, pif, pif00 + (float2)(0.f, 1.f));
	pv += get_image_pixel_weighted_linear(im, im_dim, pif, pif00 + (float2)(1.f, 0.f));
	pv += get_image_pixel_weighted_linear(im, im_dim, pif, pif00 + (float2)(1.f, 1.f));

	return pv;
}

float4 blit_sprite(global uint *lei, global uchar *data_cl, float4 pv)
{
	const int2 p = (int2) (get_global_id(0), get_global_id(1));
	const float2 pf = convert_float2(p);
	global float *lef = lei;
	global ulong *lel = lei;
	global float4 *im;
	int2 im_dim;
	float2 pscale, pos, pif;

	//im = (global float4 *) ((global ulong *) lei)[0];		// global address for the start of the image
	//im = (global float4 *) &data_cl[((global ulong *) lei)[0]];	// *lei has the 64-bit offset (in bytes) in data_cl where the image data starts
	im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];
	//im = data_cl;
	im_dim.x = lei[2];
	im_dim.y = lei[3];
	pscale.x = lef[4];
	pscale.y = lef[5];
	pos.x = lef[6];
	pos.y = lef[7];

	pif = pscale * (pf + pos);
	//pv += image_interp_nearest(im, im_dim, pif);
	pv += image_interp_linear(im, im_dim, pif);

	return pv;
}

float2 set_new_distance_from_point(float2 p0, float2 pc, float dist_mul)
{
	return pc + (p0 - pc) * dist_mul;
}

float inverse_distortion(float a, float x)
{
	float p1, p2, p3;

	p1 = cbrt(2.f / (3.f * a));
	p2 = cbrt( sqrt(3.f*a) * sqrt( 27.f*a*x*x + 4.f ) - 9.f*a*x );
	p3 = cbrt(2.f) * pow(3.f*a, 2.f / 3.f);

	return p1/p2 - p2/p3;	// http://www.wolframalpha.com/input/?i=inverse+of+x+*+(1+%2B+11*x%5E2)
}

float4 blit_photo(global uint *lei, global uchar *data_cl, float4 pv)
{
	const int2 p = (int2) (get_global_id(0), get_global_id(1));
	const float2 pf = convert_float2(p);
	global float *lef = lei;
	global ulong *lel = lei;
	global float4 *im;
	int2 im_dim;
	float2 pscale, pos, pif, pc;
	float distortion, dist_scale, gain, distortion_mul, d;

	im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];
	im_dim.x = lei[2];
	im_dim.y = lei[3];
	pscale.x = lef[4];
	pscale.y = lef[5];
	pos.x = lef[6];
	pos.y = lef[7];
	pc.x = lef[8];
	pc.y = lef[9];
	distortion = lef[10];
	dist_scale = lef[11];
	gain = lef[12];

	pif = pscale * (pf + pos);

	d = fast_distance(pif, pc) * dist_scale;
	if (distortion==0.f)
		distortion_mul = 1.f;
	else
		distortion_mul = inverse_distortion(distortion, d) / d;
	pif = set_new_distance_from_point(pif, pc, distortion_mul);

	//pv += image_interp_nearest(im, im_dim, pif);
	pv += gain * image_interp_linear(im, im_dim, pif);

	return pv;
}
