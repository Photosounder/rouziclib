bool check_image_bounds(int2 pi, int2 im_dim)
{
	if (pi.x >= 0 && pi.x < im_dim.x)
		if (pi.y >= 0 && pi.y < im_dim.y)
			return true;
	return false;
}

/*float4 image_interp_nearest(global float4 *im, int2 im_dim, float2 pif)
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
}*/

float4 read_frgb_pixel(global float4 *im, int index)
{
	return im[index];
}

float4 read_float1_pixel(global float *im, int index)
{
	float4 pv = 0.f;
	pv.x = im[index];
	pv.w = 1.f;
	return pv;
}

float4 read_float2_pixel(global float *im, int index)
{
	float4 pv = 0.f;
	index <<= 1;
	pv.x = im[index];
	pv.y = im[index+1];
	pv.z = 0.f;
	pv.w = 1.f;
	return pv;
}

float4 read_float2_pixel_planar(global float *im, int2 im_dim, int2 i)
{
	int size_full = im_dim.x*im_dim.y;
	int index = i.y * im_dim.x + i.x;
	float4 pv = 0.f;

	pv.x = im[index];
	pv.y = im[index + size_full];
	pv.z = 0.f;
	pv.w = 1.f;
	return pv;
}

float4 read_float3_pixel(global float *im, int index)
{
	float4 pv;
	index *= 3;
	pv.x = im[index];
	pv.y = im[index+1];
	pv.z = im[index+2];
	pv.w = 1.f;
	return pv;
}

float4 read_float3_pixel_planar(global float *im, int2 im_dim, int2 i)
{
	int size_full = im_dim.x*im_dim.y;
	int index = i.y * im_dim.x + i.x;
	float4 pv = 0.f;

	pv.x = im[index];
	pv.y = im[index + size_full];
	pv.z = im[index + size_full*2];
	pv.w = 1.f;
	return pv;
}


float4 read_sqrgb_pixel(global uint *im, int index)
{
	float4 pv;
	uint4 pvi;
	uint v;
	const float mul_rb = 1.f / (1023.f*1023.f);
	const float mul_g = 1.f / (4092.f*4092.f);
	const float4 mul_pvi = (float4) (mul_rb, mul_g, mul_rb, 1.f);

	v = im[index];
	pvi.z = v >> 22;
	pvi.y = (v >> 10) & 4095;
	pvi.x = v & 1023;
	pvi.w = 1;

	pv = convert_float4(pvi*pvi) * mul_pvi;

	return pv;
}

float4 read_srgb_pixel(global uint *im, int index)
{
	float4 pv;
	uint v;

	v = im[index];
	pv.z = (v >> 16) & 255;
	pv.y = (v >> 8)  & 255;
	pv.x = v         & 255;
	pv.xyz = s8lrgb(pv.xyz);
	pv.w = 1.f;

	return pv;
}

float4 read_gamma22_pixel(global uint *im, int index)
{
	// Decode gamma RGB bytes and preserve the packed raster's opaque alpha
	uint value = im[index];
	float3 rgb = (float3)(value & 255u, (value >> 8u) & 255u, (value >> 16u) & 255u);
	return (float4)(pow(rgb*(1.f/255.f), (float3)(2.2f)), 1.f);
}

float4 read_lrgb_pixel(global ushort *im, int index)
{
	float4 pv;

	pv.x = im[index]   * 0.000030517578125f;
	pv.y = im[index+1] * 0.000030517578125f;
	pv.z = im[index+2] * 0.000030517578125f;
	pv.w = 1.f;

	return pv;
}

float4 rec709_to_linear(float4 pv)
{
	float4 l = pv * (1.f/4.5f);
	float4 c = pow((pv+0.09929682f)/1.0992968f, 1.f/0.45f);
	return select(l, c, pv > 0.08125635f);
}

float4 rec709_ycbcr_to_rgb(float3 yuv)
{
	float4 pv;
	pv.x = yuv.x + 1.7927f * yuv.z;
        pv.y = yuv.x - 0.2132f * yuv.y - 0.5329f * yuv.z;
        pv.z = yuv.x + 2.1124f * yuv.y;
	pv.w = 1.f;
	return pv;
}

float4 raw_yuv_to_lrgb(float3 raw, float depth_mul)
{
	float y, u, v, r, g, b;
	float4 pv;
	float3 yuv;

	raw *= depth_mul;
	yuv.x = raw.x - 16.f/255.f;
	yuv.x *= 255.f / 219.f;
	yuv.y = raw.y - 128.f/255.f;
	yuv.z = raw.z - 128.f/255.f;

	pv = rec709_to_linear(rec709_ycbcr_to_rgb(yuv));
	return pv;
}

float4 raw_yuvj_to_lrgb(float3 raw, float depth_mul)
{
	float y, u, v, r, g, b;
	float4 pv;

	raw *= depth_mul;
	y = raw.x;
	u = raw.y - 128.f;
	v = raw.z - 128.f;

	pv.x = y + 1.596f * v;
        pv.y = y - 0.813f * v - 0.391f * u;
        pv.z = y + 2.018f * u;

	pv.xyz = s8lrgb(pv.xyz);
	pv.w = 1.f;

	return pv;
}

float4 read_yuv420p8_pixel(global uchar *im, int2 im_dim, int2 i)
{
	float4 pv;
	float y, u, v, r, g, b;
	int2 im_dimh = im_dim / 2;
	int size_full = im_dim.x*im_dim.y, size_half = size_full/4, y_index, uv_index;
	global uchar *u_plane, *v_plane;

	u_plane = &im[size_full];
	v_plane = &im[size_full + size_half];
	y_index = i.y * im_dim.x + i.x;
	uv_index = i.y/2 * im_dimh.x + i.x/2;	// TODO fix for MPEG-2 layout

	pv = raw_yuv_to_lrgb( (float3) (im[y_index], u_plane[uv_index], v_plane[uv_index]), 1.f/255.f );

	return pv;
}

float4 read_yuvj420p8_pixel(global uchar *im, int2 im_dim, int2 i)
{
	float4 pv;
	float y, u, v, r, g, b;
	int2 im_dimh = im_dim / 2;
	int size_full = im_dim.x*im_dim.y, size_half = size_full/4, y_index, uv_index;
	global uchar *u_plane, *v_plane;

	u_plane = &im[size_full];
	v_plane = &im[size_full + size_half];
	y_index = i.y * im_dim.x + i.x;
	uv_index = i.y/2 * im_dimh.x + i.x/2;	// TODO fix for MPEG-2 layout

	pv = raw_yuvj_to_lrgb( (float3) (im[y_index], u_plane[uv_index], v_plane[uv_index]), 1.f );

	return pv;
}

float4 read_yuv420pN_pixel(global ushort *im, int2 im_dim, int2 i, float depth_mul)	// yuv420p formats above 8 bpc
{
	float4 pv;
	float y, u, v, r, g, b;
	int2 im_dimh = im_dim / 2;
	int size_full = im_dim.x*im_dim.y, size_half = size_full/4, y_index, uv_index;
	global ushort *u_plane, *v_plane;

	u_plane = &im[size_full];
	v_plane = &im[size_full + size_half];
	y_index = i.y * im_dim.x + i.x;
	uv_index = i.y/2 * im_dimh.x + i.x/2;

	pv = raw_yuv_to_lrgb( (float3) (im[y_index], u_plane[uv_index], v_plane[uv_index]), depth_mul );

	return pv;
}

float4 read_compressed_texture1_pixel(global uchar *d8, int2 im_dim, int2 i)
{
	// Locate the fixed 30-byte block after the eight-byte header and alternating row offset
	int block_y = i.y >> 3;
	int shifted_x = i.x + ((block_y & 1)*4);
	int block_x = shifted_x >> 3;
	int line_width_zero = (im_dim.x+7) >> 3;
	int line_width_one = (im_dim.x+11) >> 3;
	global uchar *block = d8+8+30*(((block_y+1) >> 1)*line_width_zero+(block_y >> 1)*line_width_one+block_x);

	// Read the six byte-aligned endpoint components directly
	float3 colour_zero = convert_float3(vload3(0, block))*(1.f/255.f);
	float3 colour_one = convert_float3(vload3(0, block+3))*(1.f/255.f);

	// Extract the MSB-first three-bit index without reading beyond the block
	uint bit = ((i.y & 7)*8+(shifted_x & 7))*3;
	global uchar *offset = block+6+(bit >> 3);
	uint shift = bit & 7;
	uint packed = ((uint) offset[0]) << 8;
	if (shift > 5)
		packed |= offset[1];
	uint pixel_index = (packed >> (13-shift)) & 7;

	// Interpolate endpoints in the encoded lightness space before converting to linear RGB
	float3 pixel = mix(colour_zero, colour_one, convert_float(pixel_index)*(1.f/7.f));
	return (float4) (Lab_L_to_linear(pixel.x), Lab_L_to_linear(pixel.y), Lab_L_to_linear(pixel.z), 1.f);
}

float4 read_fmt_pixel(const int fmt, global float4 *im, int2 im_dim, int2 i)
{
	switch (fmt)
	{
		case 0:		// frgb_t
			return read_frgb_pixel((global float4 *) im, i.y * im_dim.x + i.x);

		case 1:		// sqrgb_t
			return read_sqrgb_pixel((global uint *) im, i.y * im_dim.x + i.x);

		case 2:		// srgb_t
			return read_srgb_pixel((global uint *) im, i.y * im_dim.x + i.x);

		case 3:		// lrgb_t
			return read_lrgb_pixel((global ushort *) im, 4*(i.y * im_dim.x + i.x));

		// Decode packed gamma 2.2 RGB before interpolation
		case 4:
			return read_gamma22_pixel((global uint *) im, i.y * im_dim.x + i.x);

		case 10:	// YCbCr 420 planar 8-bit (AV_PIX_FMT_YUV420P)
			return read_yuv420p8_pixel((global uchar *) im, im_dim, i);

		case 11:	// YCbCr 420 planar 10-bit LE (AV_PIX_FMT_YUV420P10LE)
			return read_yuv420pN_pixel((global ushort *) im, im_dim, i, 0.25f/255.f);

		case 12:	// YCbCr 420 planar 12-bit LE (AV_PIX_FMT_YUV420P12LE)
			return read_yuv420pN_pixel((global ushort *) im, im_dim, i, 0.0625f/255.f);

		case 15:	// YCbCr 420 planar full 0-255 range 8-bit (AV_PIX_FMT_YUVJ420P)
			return read_yuvj420p8_pixel((global uchar *) im, im_dim, i);

		case 20:	// Compressed texture format
			return read_compressed_texture1_pixel((global uchar *) im, im_dim, i);

		case 31:	// 1 channel float
		case 41:
			return read_float1_pixel((global float *) im, i.y * im_dim.x + i.x);

		case 32:	// 2 channel float
			return read_float2_pixel((global float *) im, i.y * im_dim.x + i.x);

		case 33:	// 3 channel float
			return read_float3_pixel((global float *) im, i.y * im_dim.x + i.x);

		case 42:	// 2 channel float planar
			return read_float2_pixel_planar((global float *) im, im_dim, i);

		case 43:	// 3 channel float planar
			return read_float3_pixel_planar((global float *) im, im_dim, i);
	}

	return 0.f;
}

float4 read_fmt_pixel_checked(global float4 *im, int2 im_dim, const int fmt, int2 pi)
{
	float4 pv;

	if (check_image_bounds(pi, im_dim))
		pv = read_fmt_pixel(fmt, im, im_dim, pi);
	else
		pv = 0.f;

	return pv;
}

float calc_flattop_weight(float2 pif, float2 i, float2 knee, float2 slope, float2 pscale)
{
	float2 d, w;

	d = fabs(pif - i);
	d = max(d, knee);
	w = slope * (d - pscale);

	return w.x * w.y;
}

float4 image_filter_flattop(global float4 *im, int2 im_dim, const int fmt, float2 pif, float2 pscale, float2 slope)
{
	float4 pv = 0.f;
	float2 knee, i, start, end;

	knee = 0.5f - fabs(fmod(pscale, 1.f) - 0.5f);

	start = max(0.f, ceil(pif - pscale));
	end = min(convert_float2(im_dim - 1), floor(pif + pscale));

	for (i.y = start.y; i.y <= end.y; i.y+=1.f)
		for (i.x = start.x; i.x <= end.x; i.x+=1.f)
			pv += read_fmt_pixel(fmt, im, im_dim, convert_int2(i)) * calc_flattop_weight(pif, i, knee, slope, pscale);

	return pv;
}

float2 calc_aa_nearest_weights(float2 pif, float2 i, float2 pscale)
{
	float2 d, w;

	d = fabs(pif - i);
	w = clamp(native_divide(0.5f - d, pscale) + 0.5f, 0.f, 1.f);

	return w;
}

float4 image_filter_aa_nearest(global float4 *im, int2 im_dim, const int fmt, float2 pif, float2 pscale)
{
	float4 pv = 0.f;
	float2 pif00, w00;
	int2 pi00;
	float w;

	pif00 = floor(pif);
	pi00 = convert_int2(pif00);
	w00 = calc_aa_nearest_weights(pif, pif00, pscale);

	w = w00.x * w00.y;
	pv  = read_fmt_pixel_checked(im, im_dim, fmt, pi00) * w;

	if (w < 1.f)
	{
		pv += read_fmt_pixel_checked(im, im_dim, fmt, pi00 + (int2)(0, 1)) * w00.x * (1.f - w00.y);
		pv += read_fmt_pixel_checked(im, im_dim, fmt, pi00 + (int2)(1, 0)) * (1.f - w00.x) * w00.y;
		pv += read_fmt_pixel_checked(im, im_dim, fmt, pi00 + (int2)(1, 1)) * (1.f - w00.x) * (1.f - w00.y);
	}

	return pv;
}

/*float4 blit_sprite_bilinear(global uint *lei, global uchar *data_cl, float4 pv)
{
	const int2 p = (int2) (get_global_id(0), get_global_id(1));
	const float2 pf = convert_float2(p);
	global float *lef = (global float *) lei;
	global float4 *im;
	int2 im_dim;
	float2 pscale, pos, pif;

	im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];
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
}*/

float4 blit_sprite_flattop(global uint *lei, global uchar *data_cl, float4 pv, const float2 pf)
{
	global float *lef = (global float *) lei;
	global float4 *im;
	int2 im_dim;
	int fmt;
	float2 pscale, pos, pif, slope;

	im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];
	im_dim.x = lei[2];
	im_dim.y = lei[3];
	pscale.x = lef[4];
	pscale.y = lef[5];
	pos.x = lef[6];
	pos.y = lef[7];
	fmt = lei[8];
	slope.x = lef[9];
	slope.y = lef[10];

	pif = pscale * (pf + pos);
	pscale = max(1.f, pscale);
	pv += image_filter_flattop(im, im_dim, fmt, pif, pscale, slope);

	return pv;
}

float4 blit_sprite_flattop_rot(global uint *lei, global uchar *data_cl, float4 pv, const float2 pf)
{
	global float *lef = (global float *) lei;
	global float4 *im;
	int2 im_dim;
	int fmt;
	float2 pscale, pos, pif, pifo, slope;
	float costh, sinth;

	im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];
	im_dim.x = lei[2];
	im_dim.y = lei[3];
	pscale.x = lef[4];
	pscale.y = pscale.x;
	pos.x = lef[5];
	pos.y = lef[6];
	fmt = lei[7];
	slope.x = lef[8];
	slope.y = slope.x;
	costh = lef[9];
	sinth = lef[10];

	pifo = pscale * (pf + pos);
	pif.x = pifo.x * costh - pifo.y * sinth;
	pif.y = pifo.x * sinth + pifo.y * costh;
	pscale = max(1.f, pscale);
	pv += image_filter_flattop(im, im_dim, fmt, pif, pscale, slope);

	return pv;
}

float4 blit_sprite_aa_nearest(global uint *lei, global uchar *data_cl, float4 pv, const float2 pf)
{
	global float *lef = (global float *) lei;
	global float4 *im;
	int2 im_dim;
	int fmt;
	float2 pscale, pos, pif;

	im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];
	im_dim.x = lei[2];
	im_dim.y = lei[3];
	pscale.x = lef[4];
	pscale.y = lef[5];
	pos.x = lef[6];
	pos.y = lef[7];
	fmt = lei[8];

	pif = pscale * (pf + pos);
	pv += image_filter_aa_nearest(im, im_dim, fmt, pif, pscale);

	return pv;
}

float4 blit_sprite_aa_nearest_rot(global uint *lei, global uchar *data_cl, float4 pv, const float2 pf)
{
	global float *lef = (global float *) lei;
	global float4 *im;
	int2 im_dim;
	int fmt;
	float2 pscale, pos, pif, pifo;
	float costh, sinth;

	im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];
	im_dim.x = lei[2];
	im_dim.y = lei[3];
	pscale.x = lef[4];
	pscale.y = pscale.x;
	pos.x = lef[5];
	pos.y = lef[6];
	fmt = lei[7];
	costh = lef[8];
	sinth = lef[9];

	pifo = pscale * (pf + pos);
	pif.x = pifo.x * costh - pifo.y * sinth;
	pif.y = pifo.x * sinth + pifo.y * costh;
	pv += image_filter_aa_nearest(im, im_dim, fmt, pif, pscale);

	return pv;
}

/*float2 set_new_distance_from_point(float2 p0, float2 pc, float dist_mul)
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
	global float *lef = (global float *) lei;
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
}*/
