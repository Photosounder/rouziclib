#ifdef RL_INTEL_INTR

typedef struct
{
	int init;
	int block_size, bits_per_block, quincunx, bits_col, bits_per_pixel;
	int linew0, linew1, pix;
	xyi_t block_pos, block_start;
	size_t di;
	frgb_t col0, col1, pv;
	float pix_mul;
} dqs_comp_decode_t;

static frgb_t dqs_frgb(float r, float g, float b, float a)
{
	frgb_t p;

	// Construct a floating-point linear pixel
	p.r = r;
	p.g = g;
	p.b = b;
	p.a = a;
	return p;
}

static frgb_t dqs_add_frgb_local(frgb_t a, frgb_t b)
{
	// Add corresponding pixel channels
	a.r += b.r;
	a.g += b.g;
	a.b += b.b;
	a.a += b.a;
	return a;
}

static frgb_t dqs_mul_frgb_local(frgb_t p, float v)
{
	// Scale every pixel channel
	p.r *= v;
	p.g *= v;
	p.b *= v;
	p.a *= v;
	return p;
}

static int dqs_check_image_bounds(xyi_t p, xyi_t dim)
{
	// Test both image axes
	return p.x >= 0 && p.y >= 0 && p.x < dim.x && p.y < dim.y;
}

static frgb_t dqs_read_compressed_texture1_pixel(const uint8_t *data, xyi_t dim, xyi_t p, dqs_comp_decode_t *d)
{
	size_t di;
	int line_count0, line_count1, qoff, pix;
	xyi_t block_pos, ib;
	float t, ratio;

	// Initialise the fixed compressed-texture layout
	if (d->init==0)
	{
		d->block_size = 8;
		d->bits_per_block = 240;
		d->quincunx = 1;
		d->bits_col = 8;
		d->bits_per_pixel = 3;
		d->linew0 = (dim.x + d->block_size-1) / d->block_size;
		d->linew1 = (dim.x + (d->block_size>>1) + d->block_size-1) / d->block_size;
		d->pix_mul = 2.f / ((1 << d->bits_per_pixel)-1);
		d->block_pos = set_xyi(-1);
		d->pix = -1;
		d->init = 1;
	}

	// Locate the quincunx-compressed block
	block_pos.y = p.y / d->block_size;
	qoff = (block_pos.y&1) * d->quincunx * (d->block_size>>1);
	block_pos.x = (p.x+qoff) / d->block_size;

	// Decode the endpoints when entering a new block
	if (block_pos.x != d->block_pos.x || block_pos.y != d->block_pos.y)
	{
		line_count0 = (block_pos.y+1) >> 1;
		line_count1 = block_pos.y >> 1;
		di = (size_t) (line_count0*d->linew0 + line_count1*d->linew1 + block_pos.x);
		di = di*d->bits_per_block + 64;
		d->block_start = mul_xyi(block_pos, set_xyi(d->block_size));
		d->block_start.x -= qoff;
		ratio = 1.f / ((1 << d->bits_col)-1);
		d->col0.r = get_bits_in_stream_inc(data, &di, d->bits_col)*ratio;
		d->col0.g = get_bits_in_stream_inc(data, &di, d->bits_col)*ratio;
		d->col0.b = get_bits_in_stream_inc(data, &di, d->bits_col)*ratio;
		d->col0.a = 1.f;
		d->col1.r = get_bits_in_stream_inc(data, &di, d->bits_col)*ratio;
		d->col1.g = get_bits_in_stream_inc(data, &di, d->bits_col)*ratio;
		d->col1.b = get_bits_in_stream_inc(data, &di, d->bits_col)*ratio;
		d->col1.a = 1.f;
		d->di = di;
		d->block_pos = block_pos;
		d->pix = -1;
	}

	// Decode and interpolate this pixel's endpoint index
	ib = sub_xyi(p, d->block_start);
	di = d->di + (size_t) (ib.y*d->block_size + ib.x)*d->bits_per_pixel;
	pix = (int) get_bits_in_stream(data, di, d->bits_per_pixel);
	if (pix != d->pix)
	{
		t = pix*d->pix_mul*0.5f;
		d->pv.r = Lab_L_to_linear(d->col0.r*(1.f-t) + d->col1.r*t);
		d->pv.g = Lab_L_to_linear(d->col0.g*(1.f-t) + d->col1.g*t);
		d->pv.b = Lab_L_to_linear(d->col0.b*(1.f-t) + d->col1.b*t);
		d->pv.a = 1.f;
		d->pix = pix;
	}
	return d->pv;
}

static frgb_t dqs_read_fmt_pixel(const uint8_t *im, xyi_t dim, int fmt, xyi_t p, dqs_comp_decode_t *cd)
{
	size_t index, full_size, half_size, uv_index;
	const float *f = (const float *) im;
	const uint16_t *u16 = (const uint16_t *) im;
	const uint8_t *u8 = im;
	lrgb_t l;

	// Calculate the ordinary packed-pixel index
	index = (size_t) p.y*dim.x + p.x;

	// Decode every raster format supported by the OpenCL renderer
	switch (fmt)
	{
		case 0:
			return ((const frgb_t *) im)[index];

		case 1:
			return sqrgb_to_frgb(((const sqrgb_t *) im)[index]);

		case 2:
		case 4:
		{
			// Decode byte RGB according to the queued transfer function
			frgb_t p = rgb8_to_frgb(((const srgb_t *) im)[index], fmt == 4 ? RGB8_TRANSFER_GAMMA22 : RGB8_TRANSFER_SRGB);

			// Match the GPU raster path which treats packed sRGB as opaque
			p.a = 1.f;
			return p;
		}

		case 3:
			l = ((const lrgb_t *) im)[index];
			return dqs_frgb(l.r*(1.f/32768.f), l.g*(1.f/32768.f), l.b*(1.f/32768.f), 1.f);

		case 10:
		case 11:
		case 12:
		case 15:
			full_size = (size_t) dim.x*dim.y;
			half_size = full_size >> 2;
			uv_index = (size_t) (p.y>>1)*(dim.x>>1) + (p.x>>1);
			if (fmt==10)
				return raw_yuv_to_frgb(u8[index], u8[full_size+uv_index], u8[full_size+half_size+uv_index], 1.f);
			if (fmt==11)
				return raw_yuv_to_frgb(u16[index], u16[full_size+uv_index], u16[full_size+half_size+uv_index], 0.25f);
			if (fmt==12)
				return raw_yuv_to_frgb(u16[index], u16[full_size+uv_index], u16[full_size+half_size+uv_index], 0.0625f);
			return raw_yuvj_to_frgb(u8[index], u8[full_size+uv_index], u8[full_size+half_size+uv_index]);

		case 20:
			return dqs_read_compressed_texture1_pixel(im, dim, p, cd);

		case 31:
		case 41:
			return dqs_frgb(f[index], 0.f, 0.f, 1.f);

		case 32:
			return dqs_frgb(f[2*index], f[2*index+1], 0.f, 1.f);

		case 33:
			return dqs_frgb(f[3*index], f[3*index+1], f[3*index+2], 1.f);

		case 42:
			full_size = (size_t) dim.x*dim.y;
			return dqs_frgb(f[index], f[index+full_size], 0.f, 1.f);

		case 43:
			full_size = (size_t) dim.x*dim.y;
			return dqs_frgb(f[index], f[index+full_size], f[index+2*full_size], 1.f);
	}
	return dqs_frgb(0.f, 0.f, 0.f, 0.f);
}

static frgb_t dqs_read_fmt_pixel_checked(const uint8_t *im, xyi_t dim, int fmt, xyi_t p, dqs_comp_decode_t *cd)
{
	// Return transparent black beyond the source raster
	if (dqs_check_image_bounds(p, dim)==0)
		return dqs_frgb(0.f, 0.f, 0.f, 0.f);
	return dqs_read_fmt_pixel(im, dim, fmt, p, cd);
}

static __m128 dqs_transfer_bt709_to_linear_ps(__m128 x)
{
	__m128 curve, line, selection;

	// Scale code values and select the linear Rec. 709 segment
	x = _mm_mul_ps(x, _mm_set1_ps(1.f/255.f));
	selection = _mm_cmplt_ps(x, _mm_set1_ps(0.081f));

	// Match the shared vector BT.709 polynomial
	curve = _mm_set1_ps(-0.04154f);
	curve = _mm_add_ps(_mm_mul_ps(curve, x), _mm_set1_ps(0.21208f));
	curve = _mm_add_ps(_mm_mul_ps(curve, x), _mm_set1_ps(0.72644f));
	curve = _mm_add_ps(_mm_mul_ps(curve, x), _mm_set1_ps(0.097881f));
	curve = _mm_add_ps(_mm_mul_ps(curve, x), _mm_set1_ps(0.005135f));
	line = _mm_mul_ps(x, _mm_set1_ps(1.f/4.5f));

	// Select both transfer segments without scalar channel branches
	return _mm_or_ps(_mm_and_ps(selection, line), _mm_andnot_ps(selection, curve));
}

static __m128 dqs_raw_yuv_to_frgb_ps(float y, float u, float v, float depth_mul)
{
	__m128 p;

	// Apply the same Rec. 709 YCbCr matrix as the other renderers
	y = (y*depth_mul-16.f)*(255.f/219.f);
	u = u*depth_mul-128.f;
	v = v*depth_mul-128.f;
	p = _mm_set_ps(255.f,
		y+2.1124f*u,
		y-0.2132f*u-0.5329f*v,
		y+1.7927f*v);

	// Convert all channels through the shared BT.709 approximation
	p = dqs_transfer_bt709_to_linear_ps(p);
	return p;
	//return _mm_blend_ps(p, _mm_set1_ps(1.f), 8);	// sets the alpha to 1 instead of something close
}

static __m128 dqs_read_yuv420p_ps(const uint8_t *im, xyi_t dim, xyi_t p, int fmt)
{
	size_t full_size, half_size, index, uv_index;
	const uint16_t *u16=(const uint16_t *) im;

	// Calculate the shared planar YUV indices
	full_size = (size_t) dim.x*dim.y;
	half_size = full_size>>2;
	index = (size_t) p.y*dim.x+p.x;
	uv_index = (size_t) (p.y>>1)*(dim.x>>1)+(p.x>>1);

	// Decode the supported limited-range YUV depths
	if (fmt==10)
		return dqs_raw_yuv_to_frgb_ps(im[index], im[full_size+uv_index], im[full_size+half_size+uv_index], 1.f);
	if (fmt==11)
		return dqs_raw_yuv_to_frgb_ps(u16[index], u16[full_size+uv_index], u16[full_size+half_size+uv_index], 0.25f);
	return dqs_raw_yuv_to_frgb_ps(u16[index], u16[full_size+uv_index], u16[full_size+half_size+uv_index], 0.0625f);
}

static __m128 dqs_read_fmt_pixel_ps(const uint8_t *im, xyi_t dim, int fmt, xyi_t p, dqs_comp_decode_t *cd)
{
	frgb_t pixel;
	size_t index=(size_t) p.y*dim.x+p.x;

	// Use direct SIMD decoding for the hot packed and YUV formats
	switch (fmt)
	{
		case 0:
			return _mm_loadu_ps((const float *) &((const frgb_t *) im)[index]);
		case 1:
			return _mm_sqrgb_to_ps(((const sqrgb_t *) im)[index]);
		case 2:
			return _mm_blend_ps(_mm_srgb_to_ps(((const srgb_t *) im)[index]), _mm_set1_ps(1.f), 8);
		case 10:
		case 11:
		case 12:
			return dqs_read_yuv420p_ps(im, dim, p, fmt);
	}

	// Fall back to the complete scalar format decoder
	pixel = dqs_read_fmt_pixel(im, dim, fmt, p, cd);
	return _mm_loadu_ps((const float *) &pixel);
}

static __m128 dqs_calc_flattop_weight_ps(__m128 pif, __m128 p, __m128 knee, __m128 slope, __m128 pscale)
{
	__m128 d, weight;

	// Calculate both separable distances in parallel
	d = _mm_abs_ps(_mm_sub_ps(pif, p));
	d = _mm_max_ps(d, knee);
	d = _mm_mul_ps(slope, _mm_sub_ps(d, pscale));

	// Multiply the x and y weights and broadcast the result
	weight = _mm_hlomul_ss(d);
	return _mm_shuffle_ps(weight, weight, 0);
}

static __m128 dqs_image_filter_flattop_ps(const uint8_t *im, xyi_t dim, __m128 im_limit, int fmt, __m128 pif, __m128 pscale, __m128 slope, __m128 knee)
{
	dqs_comp_decode_t cd={0};
	__m128 sum, pixel, weight, p, startv, endv, increment;
	xyi_t ip, start, end;

	// Calculate the two-dimensional integer filter bounds with SIMD
	sum = _mm_setzero_ps();
	startv = _mm_ceil_ps(_mm_max_ps(sum, _mm_sub_ps(pif, pscale)));
	endv = _mm_floor_ps(_mm_min_ps(im_limit, _mm_add_ps(pif, pscale)));
	_mm_storel_epi64((__m128i *) &start, _mm_cvtps_epi32(startv));
	_mm_storel_epi64((__m128i *) &end, _mm_cvtps_epi32(endv));
	increment = _mm_set_ps(0.f, 0.f, 0.f, 1.f);

	// Restore the original SIMD filter traversal
	for (ip.y=start.y; ip.y<=end.y; ip.y++)
	{
		p = _mm_set_ps(0.f, 0.f, ip.y, start.x);
		for (ip.x=start.x; ip.x<=end.x; ip.x++)
		{
			weight = dqs_calc_flattop_weight_ps(pif, p, knee, slope, pscale);
			pixel = dqs_read_fmt_pixel_ps(im, dim, fmt, ip, &cd);
			sum = _mm_add_ps(sum, _mm_mul_ps(pixel, weight));
			p = _mm_add_ps(p, increment);
		}
	}
	return sum;
}

static frgb_t dqs_image_filter_flattop(const uint8_t *im, xyi_t dim, int fmt, xy_t pif, xy_t pscale, xy_t slope)
{
	dqs_comp_decode_t cd={0};
	frgb_t sum=dqs_frgb(0.f, 0.f, 0.f, 0.f), p;
	xy_t knee;
	xyi_t i, start, end;
	float dx, dy, weight;

	// Derive the flat-top integration bounds and fractional knee
	knee.x = 0.5f-fabsf(fmodf(pscale.x, 1.f)-0.5f);
	knee.y = 0.5f-fabsf(fmodf(pscale.y, 1.f)-0.5f);
	start.x = MAXN(0, (int) ceil(pif.x-pscale.x));
	start.y = MAXN(0, (int) ceil(pif.y-pscale.y));
	end.x = MINN(dim.x-1, (int) floor(pif.x+pscale.x));
	end.y = MINN(dim.y-1, (int) floor(pif.y+pscale.y));

	// Accumulate all source pixels covered by the separable kernel
	for (i.y=start.y; i.y<=end.y; i.y++)
		for (i.x=start.x; i.x<=end.x; i.x++)
		{
			dx = MAXN(fabsf((float) (pif.x-i.x)), (float) knee.x);
			dy = MAXN(fabsf((float) (pif.y-i.y)), (float) knee.y);
			weight = slope.x*(dx-pscale.x) * slope.y*(dy-pscale.y);

			// Avoid decoding zero-contribution samples around exact-scale pixels
			if (weight==0.f)
				continue;
			p = dqs_read_fmt_pixel(im, dim, fmt, i, &cd);
			sum = dqs_add_frgb_local(sum, dqs_mul_frgb_local(p, weight));
		}
	return sum;
}

static void dqsb_blit_sprite_flattop_simd(float *lef, uint8_t *data, float *block, xy_t start_pos, int bs, int chan_stride)
{
	uint32_t *lei=(uint32_t *) lef;
	uint64_t data_index;
	uint8_t *im;
	xyi_t dim;
	xy_t pscale, pscale_capped, pos, slope, knee, pif;
	__m128 pixel, pscalev, slopev, kneev, im_limitv;
	float channels[4];
	int x, y, ib=0, ic;

	// Load the axis-aligned flat-top parameters
	data_index = (uint64_t) lei[0] | ((uint64_t) lei[1]<<32);
	im = &data[data_index];
	dim = xyi(lei[2], lei[3]);
	pscale = xy(lef[4], lef[5]);
	pos = xy(lef[6], lef[7]);
	slope = xy(lef[9], lef[10]);
	pscale_capped = max_xy(pscale, XY1);
	knee = sub_xy(set_xy(0.5), abs_xy(sub_xy(fmod_xy(pscale_capped, XY1), set_xy(0.5))));
	pscalev = _mm_xy_to_ps(pscale_capped);
	slopev = _mm_xy_to_ps(slope);
	kneev = _mm_xy_to_ps(knee);
	im_limitv = _mm_xyi_to_ps(sub_xyi(dim, set_xyi(1)));

	// Filter each destination pixel through the SIMD source path
	for (y=0; y<bs; y++)
		for (x=0; x<bs; x++, ib++)
		{
			pif.x = pscale.x*(start_pos.x+x+pos.x);
			pif.y = pscale.y*(start_pos.y+y+pos.y);
			pixel = dqs_image_filter_flattop_ps(im, dim, im_limitv, lei[8], _mm_xy_to_ps(pif), pscalev, slopev, kneev);
			_mm_storeu_ps(channels, pixel);
			for (ic=0; ic<4; ic++)
				block[ic*chan_stride+ib] += channels[ic];
		}
}

static frgb_t dqs_image_filter_aa_nearest(const uint8_t *im, xyi_t dim, int fmt, xy_t pif, xy_t pscale)
{
	dqs_comp_decode_t cd={0};
	frgb_t sum=dqs_frgb(0.f, 0.f, 0.f, 0.f), p;
	xyi_t p00;
	float wx, wy, weight;

	// Calculate the lower sample and its antialiasing weights
	p00.x = (int) floor(pif.x);
	p00.y = (int) floor(pif.y);
	wx = rangelimitf((0.5f-fabsf((float) (pif.x-p00.x)))/(float) pscale.x+0.5f, 0.f, 1.f);
	wy = rangelimitf((0.5f-fabsf((float) (pif.y-p00.y)))/(float) pscale.y+0.5f, 0.f, 1.f);

	// Accumulate the four neighboring source pixels
	weight = wx*wy;
	p = dqs_read_fmt_pixel_checked(im, dim, fmt, p00, &cd);
	sum = dqs_add_frgb_local(sum, dqs_mul_frgb_local(p, weight));
	p = dqs_read_fmt_pixel_checked(im, dim, fmt, add_xyi(p00, xyi(0, 1)), &cd);
	sum = dqs_add_frgb_local(sum, dqs_mul_frgb_local(p, wx*(1.f-wy)));
	p = dqs_read_fmt_pixel_checked(im, dim, fmt, add_xyi(p00, xyi(1, 0)), &cd);
	sum = dqs_add_frgb_local(sum, dqs_mul_frgb_local(p, (1.f-wx)*wy));
	p = dqs_read_fmt_pixel_checked(im, dim, fmt, add_xyi(p00, xyi(1, 1)), &cd);
	sum = dqs_add_frgb_local(sum, dqs_mul_frgb_local(p, (1.f-wx)*(1.f-wy)));
	return sum;
}

static void dqsb_blit_sprite(float *lef, uint8_t *data, float *block, xy_t start_pos, int bs, int chan_stride, int rotated, int aa_nearest)
{
	uint32_t *lei=(uint32_t *) lef;
	uint64_t data_index;
	uint8_t *im;
	xyi_t dim;
	xy_t pscale, pos, slope, pf, pifo, pif;
	float costh=1.f, sinth=0.f;
	frgb_t p;
	int fmt, x, y, ib=0;

	// Load the shared data reference and image dimensions
	data_index = (uint64_t) lei[0] | ((uint64_t) lei[1]<<32);
	im = &data[data_index];
	dim = xyi(lei[2], lei[3]);

	// Load the rotated or axis-aligned parameter layout
	if (rotated)
	{
		pscale = set_xy(lef[4]);
		pos = xy(lef[5], lef[6]);
		fmt = lei[7];
		if (aa_nearest)
		{
			costh = lef[8];
			sinth = lef[9];
		}
		else
		{
			slope = set_xy(lef[8]);
			costh = lef[9];
			sinth = lef[10];
		}
	}
	else
	{
		pscale = xy(lef[4], lef[5]);
		pos = xy(lef[6], lef[7]);
		fmt = lei[8];
		if (aa_nearest==0)
			slope = xy(lef[9], lef[10]);
	}

	// Filter and add every destination pixel in the sector
	for (y=0; y<bs; y++)
		for (x=0; x<bs; x++, ib++)
		{
			pf = xy(start_pos.x+x, start_pos.y+y);
			pifo = mul_xy(pscale, add_xy(pf, pos));
			pif.x = pifo.x*costh - pifo.y*sinth;
			pif.y = pifo.x*sinth + pifo.y*costh;
			if (aa_nearest)
				p = dqs_image_filter_aa_nearest(im, dim, fmt, pif, pscale);
			else
				p = dqs_image_filter_flattop(im, dim, fmt, pif, max_xy(pscale, XY1), slope);
			block[ib] += p.r;
			block[chan_stride+ib] += p.g;
			block[2*chan_stride+ib] += p.b;
			block[3*chan_stride+ib] += p.a;
		}
}

void dqsb_blit_sprite_flattop(float *lef, uint8_t *data, float *block, xy_t start_pos, const int bs, int chan_stride)
{
	// Run the axis-aligned flat-top blit
	dqsb_blit_sprite_flattop_simd(lef, data, block, start_pos, bs, chan_stride);
}

void dqsb_blit_sprite_flattop_rot(float *lef, uint8_t *data, float *block, xy_t start_pos, const int bs, int chan_stride)
{
	// Run the rotated flat-top blit
	dqsb_blit_sprite(lef, data, block, start_pos, bs, chan_stride, 1, 0);
}

void dqsb_blit_sprite_aa_nearest(float *lef, uint8_t *data, float *block, xy_t start_pos, const int bs, int chan_stride)
{
	// Run the axis-aligned antialiased-nearest blit
	dqsb_blit_sprite(lef, data, block, start_pos, bs, chan_stride, 0, 1);
}

void dqsb_blit_sprite_aa_nearest_rot(float *lef, uint8_t *data, float *block, xy_t start_pos, const int bs, int chan_stride)
{
	// Run the rotated antialiased-nearest blit
	dqsb_blit_sprite(lef, data, block, start_pos, bs, chan_stride, 1, 1);
}

#endif
