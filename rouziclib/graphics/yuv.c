frgb_t yuv_to_frgb(xyz_t yuv)
{
	double y, u, v, r, g, b;
	frgb_t pv;

	y = yuv.x - 16.;
	y *= 255. / 219.;
	u = yuv.y - 128.;
	v = yuv.z - 128.;

	r = y + 1.596 * v;
        g = y - 0.813 * v - 0.391 * u;
        b = y + 2.018 * u;

	pv.r = s8lrgb(r);
	pv.g = s8lrgb(g);
	pv.b = s8lrgb(b);
	pv.a = 1.;

	return pv;
}

static v128_t ff_transfer_bt709_to_linear_vec4(v128_t x)	// error around 5.2e-5
{
	x = f32x4_mul(x, f32x4_splat(1.f/255.f));

	// Selection mask
	v128_t sel = f32x4_lt(x, f32x4_splat(0.081));

	// Curve
	v128_t r = f32x4_splat(-0.04154f);
	r = f32x4_relaxed_madd(r, x, f32x4_splat(0.21208));
	r = f32x4_relaxed_madd(r, x, f32x4_splat(0.72644));
	r = f32x4_relaxed_madd(r, x, f32x4_splat(0.097881));
	r = f32x4_relaxed_madd(r, x, f32x4_splat(0.005135));

	// Line
	v128_t l = f32x4_mul(x, f32x4_splat(1.f/4.5f));

	// Select either the line or the curve
	return v128_bitselect(l, r, sel);
}

static frgb_t raw_yuv_to_frgb(float y, float u, float v, float depth_mul)
{
	v128_t pv;

	// Scale to 8-bit code values and apply the Rec. 709 YCbCr matrix
	y = (y * depth_mul - 16.f) * (255.f / 219.f);
	u = u * depth_mul - 128.f;
	v = v * depth_mul - 128.f;
	pv.f32[0] = y + 1.7927f * v;
	pv.f32[1] = y - 0.2132f * u - 0.5329f * v;
	pv.f32[2] = y + 2.1124f * u;
	pv.f32[3] = 255.f;

	// Convert all components to linear light with the vector polynomial
	pv = ff_transfer_bt709_to_linear_vec4(pv);
	pv.f32[3] = 1.f;

	return *(frgb_t *) &pv.f32;
}

static frgb_t raw_yuvj_to_frgb(float y, float u, float v)
{
	frgb_t pv;

	// Apply the full-range YUV matrix and convert its sRGB result to linear light
	u -= 128.f;
	v -= 128.f;
	pv.r = s8lrgb(y + 1.596f * v);
	pv.g = s8lrgb(y - 0.813f * v - 0.391f * u);
	pv.b = s8lrgb(y + 2.018f * u);
	pv.a = 1.f;

	return pv;
}

int buf_fmt_is_yuv420p(int buf_fmt)
{
	// Recognize the planar YUV formats supported by the renderers
	return buf_fmt==10 || buf_fmt==11 || buf_fmt==12 || buf_fmt==15;
}

frgb_t get_yuv420p_pixel_in_frgb(const raster_t *r, xyi_t p)
{
	const uint8_t *buf8 = r->buf;
	const uint16_t *buf16 = (const uint16_t *) r->buf;
	size_t full_size, half_size, y_index, uv_index;
	int half_width;
	float y, u, v;

	// Calculate the planar 4:2:0 indices without materializing an RGB pixel buffer
	full_size = (size_t) r->dim.x * r->dim.y;
	half_size = full_size >> 2;
	half_width = r->dim.x >> 1;
	y_index = (size_t) p.y * r->dim.x + p.x;
	uv_index = (size_t) (p.y >> 1) * half_width + (p.x >> 1);

	// Read and convert the sample according to its stored bit depth and range
	switch (r->buf_fmt)
	{
		case 10:
			y = buf8[y_index];
			u = buf8[full_size + uv_index];
			v = buf8[full_size + half_size + uv_index];
			return raw_yuv_to_frgb(y, u, v, 1.f);

		case 11:
			y = buf16[y_index];
			u = buf16[full_size + uv_index];
			v = buf16[full_size + half_size + uv_index];
			return raw_yuv_to_frgb(y, u, v, 0.25f);

		case 12:
			y = buf16[y_index];
			u = buf16[full_size + uv_index];
			v = buf16[full_size + half_size + uv_index];
			return raw_yuv_to_frgb(y, u, v, 0.0625f);

		case 15:
			y = buf8[y_index];
			u = buf8[full_size + uv_index];
			v = buf8[full_size + half_size + uv_index];
			return raw_yuvj_to_frgb(y, u, v);
	}

	return make_colour_frgb(NAN, NAN, NAN, NAN);
}
double frgb_to_yuv_y(frgb_t pv)
{
	return 0.2569*pv.r + 0.5044*pv.g + 0.0979*pv.b + 16.;
}

double frgb_to_yuv_u(frgb_t pv)
{
	return -0.1483*pv.r - 0.2911*pv.g + 0.4394*pv.b + 128.;
}

double frgb_to_yuv_v(frgb_t pv)
{
	return 0.4394*pv.r - 0.3679*pv.g - 0.0715*pv.b + 128.;
}

xyz_t frgb_to_yuv(frgb_t pv)
{
	return xyz( frgb_to_yuv_y(pv), frgb_to_yuv_u(pv), frgb_to_yuv_v(pv) );
}

// the frgb_t input here is linear
void frgb_2x2_to_yuv420(frgb_t *l0, frgb_t *l1, float *y, float *u, float *v)
{
	frgb_t av;

	av = mul_scalar_frgb(add_frgb(add_frgb(l0[0], l0[1]), add_frgb(l1[0], l1[1])), 0.25f);
	av = mul_scalar_frgb(lsrgb_fast_frgb(av), 255.f);

	y[0] = frgb_to_yuv_y(mul_scalar_frgb(lsrgb_fast_frgb(l0[0]), 255.));
	y[1] = frgb_to_yuv_y(mul_scalar_frgb(lsrgb_fast_frgb(l0[1]), 255.));
	y[2] = frgb_to_yuv_y(mul_scalar_frgb(lsrgb_fast_frgb(l1[0]), 255.));
	y[3] = frgb_to_yuv_y(mul_scalar_frgb(lsrgb_fast_frgb(l1[1]), 255.));
	*u = frgb_to_yuv_u(av);
	*v = frgb_to_yuv_v(av);
}
