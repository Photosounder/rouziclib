#ifdef RL_INTEL_INTR

__m128 dqsb_read_frgb_pixel(frgb_t *im, const size_t index)
{
	__m128 v = _mm_loadu_ps((float *) &im[index]);
	return v;
}

__m128 dqsb_read_sqrgb_pixel(sqrgb_t *im, const size_t index)
{
	__m128 v = _mm_sqrgb_to_ps(im[index]);
	return v;
}

__m128 dqsb_read_srgb_pixel(srgb_t *im, const size_t index)
{
	__m128 v = _mm_srgb_to_ps(im[index]);
	return v;
}

__m128 dqsb_raw_yuv_to_lrgb(float y, float u, float v, float depth_mul)
{
	float r, g, b;
	__m128 pv;

	y = y*depth_mul - 16.f;
	y *= 255.f / 219.f;
	u = u*depth_mul - 128.f;
	v = v*depth_mul - 128.f;

	r = y + 1.596f * v;
        g = y - 0.813f * v - 0.391f * u;
        b = y + 2.018f * u;

	pv = _mm_set_ps(1.f, b, g, r);
	pv = _mm_mul_ps(pv, _mm_set_ps1(1.f/255.f));
	pv = _mm_mul_ps(pv, pv);

	// TODO proper conversion
	/*pv.x = s8lrgb(r);
	pv.y = s8lrgb(g);
	pv.z = s8lrgb(b);
	pv.w = 1.;*/

	return pv;
}

__m128 dqsb_read_yuv420p8_pixel(uint8_t *im, xyi_t im_dim, xyi_t i)
{
	__m128 pv;
	float y, u, v, r, g, b;
	xyi_t im_dimh = rshift_xyi(im_dim, 1);
	int size_full = im_dim.x*im_dim.y, size_half = size_full>>2, y_index, uv_index;
	uint8_t *u_plane, *v_plane;

	u_plane = &im[size_full];
	v_plane = &im[size_full + size_half];
	y_index = i.y * im_dim.x + i.x;
	uv_index = (i.y>>1) * im_dimh.x + (i.x>>1);	// TODO fix for MPEG-2 layout

	pv = dqsb_raw_yuv_to_lrgb((float) im[y_index], (float) u_plane[uv_index], (float) v_plane[uv_index], 1.f );

	return pv;
}

typedef struct
{
	int init;
	int block_size, bits_per_block, quincunx, bits_ch, bits_cs, bits_cl, bits_per_pixel;
	int linew0, linew1, pix;
	xyi_t block_pos, block_start;
	uint64_t di;
	frgb_t col0, col1, colm, pv;
	float pix_mul;
} comp_decode_t;

__m128 dqsb_read_fmt_pixel(const int fmt, uint8_t *im, xyi_t im_dim, xyi_t i, comp_decode_t *cd1)
{
	switch (fmt)
	{
		case 0:		// frgb_t
			return dqsb_read_frgb_pixel((frgb_t *) im, i.y * im_dim.x + i.x);

		case 1:		// sqrgb_t
			return dqsb_read_sqrgb_pixel((sqrgb_t *) im, i.y * im_dim.x + i.x);

		case 2:		// srgb_t
			return dqsb_read_srgb_pixel((srgb_t *) im, i.y * im_dim.x + i.x);

		case 10:	// YCbCr 420 planar 8-bit (AV_PIX_FMT_YUV420P)
			return dqsb_read_yuv420p8_pixel(im, im_dim, i);

		/*case 11:	// YCbCr 420 planar 10-bit LE (AV_PIX_FMT_YUV420P10LE)
			return dqsb_read_yuv420pN_pixel(im, im_dim, i, 0.25f);

		case 12:	// YCbCr 420 planar 12-bit LE (AV_PIX_FMT_YUV420P12LE)
			return dqsb_read_yuv420pN_pixel(im, im_dim, i, 0.0625f);

		case 20:	// Compressed texture format
			return dqsb_read_compressed_texture1_pixel(im, im_dim, i, cd1);*/
	}

	return _mm_setzero_ps();
}

double dqsb_calc_flattop_weight(xy_t pif, xy_t i, xy_t knee, xy_t slope, xy_t pscale)
{
	xy_t d, w;

	d = abs_xy(sub_xy(pif, i));
	d = max_xy(d, knee);
	w = mul_xy(slope, sub_xy(d, pscale));

	return mul_x_by_y_xy(w);
}

__m128 dqsb_image_filter_flattop(uint8_t *im, xyi_t im_dim, const int fmt, xy_t pif, xy_t pscale, xy_t slope, xy_t knee, comp_decode_t *cd1)
{
	__m128 sum, pv, wv;
	xy_t i, start, end;

	sum = _mm_setzero_ps();

	start = max_xy(XY0, ceil_xy(sub_xy(pif, pscale)));
	end = min_xy(xyi_to_xy(sub_xyi(im_dim, set_xyi(1))), floor_xy(add_xy(pif, pscale)));

	for (i.y = start.y; i.y <= end.y; i.y+=1.)
		for (i.x = start.x; i.x <= end.x; i.x+=1.)
		{
			wv = _mm_set_ps1(dqsb_calc_flattop_weight(pif, i, knee, slope, pscale));
			pv = dqsb_read_fmt_pixel(fmt, im, im_dim, xy_to_xyi(i), cd1);
			sum = _mm_add_ps(sum, _mm_mul_ps(pv, wv));
		}

	return sum;
}

void dqsb_blit_sprite_flattop(float *lef, uint8_t *data, float *block, xy_t start_pos, const int bs, int chan_stride)
{
	comp_decode_t cd1={0};
	uint8_t *im;
	uint32_t *lei = (uint32_t *) lef;
	int ic, ib=0;
	xy_t pf, pscale, pscale_capped, pos, pif, slope, knee;
	xyi_t im_dim;
	int fmt;
	__m128 pv;
	float p4[4];

	// Load parameters
	im = &data[(uint64_t) lei[0]+((uint64_t) lei[1]<<32)];
	im_dim.x = lei[2];
	im_dim.y = lei[3];
	pscale.x = lef[4];
	pscale.y = lef[5];
	pos.x = lef[6];
	pos.y = lef[7];
	fmt = lei[8];
	slope.x = lef[9];
	slope.y = lef[10];

	pscale_capped = max_xy(XY1, pscale);
	knee = sub_xy(set_xy(0.5), abs_xy(sub_xy(fmod_xy(pscale_capped, XY1), set_xy(0.5))));

	for (pf.y=start_pos.y; pf.y < start_pos.y+bs; pf.y+=1.)
		for (pf.x=start_pos.x; pf.x < start_pos.x+bs; pf.x+=1., ib++)
		{
			pif = mul_xy(pscale, add_xy(pf, pos));
			pv = dqsb_image_filter_flattop(im, im_dim, fmt, pif, pscale_capped, slope, knee, &cd1);

			// Store each channel
			_mm_storeu_ps(p4, pv);
			for (ic=0; ic < 4; ic++)
				block[ic*chan_stride + ib] += p4[ic];
		}
}

#endif
