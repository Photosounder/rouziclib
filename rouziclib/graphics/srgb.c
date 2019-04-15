double lsrgb(double linear)	// converts a [0.0, 1.0] linear value into a [0.0, 1.0] sRGB value
{
	if (linear <= 0.0031308)
		return linear * 12.92;
	else
		return 1.055 * pow(linear, 1.0/2.4) - 0.055;
}

frgb_t lsrgb_fast_frgb(frgb_t v)
{
	v.r = fast_lsrgbf(v.r);
	v.g = fast_lsrgbf(v.g);
	v.b = fast_lsrgbf(v.b);

	return v;
}

double slrgb(double s)	// converts a [0.0, 1.0] sRGB value into a [0.0, 1.0] linear value
{
	if (s <= 0.04045)
		return s / 12.92;
	else
		return pow((s + 0.055) / 1.055, 2.4);
}

double s8lrgb(double s8)
{
	return slrgb(s8 * (1./255.));
}

lut_t get_lut_lsrgb()
{
	int32_t i;
	static int init=1;
	static lut_t lsrgb_l;

	if (init)
	{
		init = 0;

		lsrgb_l.lut_size = ONE+1;
	
		lsrgb_l.lutint = calloc (lsrgb_l.lut_size, sizeof(int32_t));
	
		for (i=0; i<lsrgb_l.lut_size; i++)
			lsrgb_l.lutint[i] = lsrgb((double) i / ONEF) * 8160. + 0.5;	// 8160 = 255 * 32 (8.5 fixed point format)
	}

	return lsrgb_l;
}

lut_t get_lut_slrgb()
{
	int32_t i;
	static int init=1;
	static lut_t slrgb_l;

	if (init)
	{
		init = 0;

		slrgb_l.lut_size = 256;
	
		slrgb_l.lutint = calloc (slrgb_l.lut_size, sizeof(int32_t));
		slrgb_l.flut = calloc (slrgb_l.lut_size, sizeof(float));
	
		for (i=0; i<slrgb_l.lut_size; i++)
		{
			slrgb_l.flut[i] = slrgb(((double) i / 255.));
			slrgb_l.lutint[i] = slrgb(((double) i / 255.)) * ONEF + 0.5;
		}
	}

	return slrgb_l;
}

lut_t get_lut_s16lrgb()
{
	int32_t i;
	static int init=1;
	static lut_t slrgb_l;

	if (init)
	{
		init = 0;

		slrgb_l.lut_size = 65536;
	
		slrgb_l.flut = calloc (slrgb_l.lut_size, sizeof(float));
	
		for (i=0; i<slrgb_l.lut_size; i++)
		{
			slrgb_l.flut[i] = slrgb(((double) i * (1. / 65535.)));
		}
	}

	return slrgb_l;
}

float s16lrgb(uint16_t v16)
{
	float *flut = get_lut_s16lrgb().flut;

	return flut[v16];
}

lut_t get_lut_lsrgb_fl()
{
	int32_t i;
	static int init=1;
	static lut_t lsrgb_fl_l;

	const int lut_size = 10305;
	const int top_mantissa_bits = 11;
	const int offset_power=5;
	const float offset = 1.f / (float) (1<<offset_power);
	const uint32_t offset_cast = *((uint32_t *) &offset);			// 0x3D000000 (0 01111010 0000...) for offset_power==5
	const uint32_t mask = 0x03FFFFFF;					// keeps only the mantissa and the bottom 3 bits of the exponent
	const int table_start_index = (offset_cast & mask) >> (23-top_mantissa_bits);	// 2^12
	uint32_t xint;
	float x;

	if (init)
	{
		init = 0;

		lsrgb_fl_l.lut_size = 1 << 3+top_mantissa_bits;			// this covers the full range of possible input values after masking-shifting
	
		lsrgb_fl_l.lutint = calloc (lsrgb_fl_l.lut_size, sizeof(int32_t));
	
		for (i=0; i<lut_size; i++)
		{
			xint = offset_cast + (i << (23-top_mantissa_bits));
			x = *((float *) &xint);
			x -= offset;
			lsrgb_fl_l.lutint[i+table_start_index] = lsrgb(x) * 8160. + 0.5;	// 8160 = 255 * 32 (8.5 fixed point format)
		}
	}

	return lsrgb_fl_l;
}

int32_t lsrgb_fl(float v, int32_t *lut)	// converts directly from a linear float value to a 8.5 fixed point sRGB value
{
	uint32_t index;

	v += 0.03125;			// same value as offset in get_lut_lsrgb_fl()
	index = *((uint32_t *) &v);
	index &= 0x03FFFFFF;		// same value as mask in get_lut_lsrgb_fl(), keeps only the mantissa and the bottom 3 bits of the exponent
	index >>= 12;			// shift to make the index. 12 is 23-top_mantissa_bits

	return lut[index];
}

lut_t dither_lut_init()
{
	int32_t i;
	lut_t dither_l;

	dither_l.lut_size = 16384;

	dither_l.lutint = calloc (dither_l.lut_size, sizeof(int32_t));

	for (i=0; i<dither_l.lut_size; i++)
	{		
		// The ratio is 16 instead of 32 because only 0.5x the noise intensity is needed
		dither_l.lutint[i] = nearbyint((0.5 + gaussian_rand()) * 16.);	// in signed 2.5 format
		//dither_l.lutint[i] = nearbyint(randrange(0., 32.));		// in signed 2.5 format
	}

	return dither_l;
}

lut_t bytecheck_lut_init(int border)
{
	int32_t i;
	lut_t bytecheck_l;

	bytecheck_l.lut_size = 256+border*2;

	bytecheck_l.lutb = calloc (bytecheck_l.lut_size, sizeof(uint8_t));
	bytecheck_l.lutb = &bytecheck_l.lutb[border];

	for (i=-border; i<bytecheck_l.lut_size-border; i++)
	{		
		bytecheck_l.lutb[i] = MAXN(MINN(i, 255), 0);
	}

	return bytecheck_l;
}

void convert_lrgb_to_srgb(framebuffer_t fb, int mode)
{
	int32_t i, id, stop, dither;
	uint32_t dith_on;
	lrgb_t p;
	static int init=1;
	static lut_t lsrgb_l, dither_l, bytecheck_l;
	const int32_t black_threshold = (1. / (255.*12.92)) * ONEF + 0.5;	// 10 for LBD==15
	int32_t pixc = fb.w*fb.h;

	if (init)
	{
		init = 0;

		lsrgb_l = get_lut_lsrgb();
		dither_l = dither_lut_init();
		bytecheck_l = bytecheck_lut_init(4);
	}

	id = rand() & 0x3FFF;
	stop = (id + 100 + (rand() & 0x03FF)) & 0x3FFF;

	if (mode==DITHER)	// Dithering takes about 14 extra cycles per full pixel
	{
		for (i=0; i<pixc; i++)
		{
			p = fb.r.l[i];

			#ifdef _DEBUG
			if (p.r > ONE)
				p.r = (rand()&1) << LBD;	// reveals out of range pixels
			if (p.g > ONE)
				p.g = (rand()&1) << LBD;
			if (p.b > ONE)
				p.b = (rand()&1) << LBD;
			#endif

			dith_on = p.r+p.g+p.b >= black_threshold;	// 0 if the pixel is black, 1 otherwise
			dither = dither_l.lutint[id] * dith_on;
			fb.r.srgb[i].r = bytecheck_l.lutb[lsrgb_l.lutint[p.r] + dither >> 5];		// 8.5 + 2.5 >> 5 = 8.0 sRGB
			fb.r.srgb[i].g = bytecheck_l.lutb[lsrgb_l.lutint[p.g] + dither >> 5];
			fb.r.srgb[i].b = bytecheck_l.lutb[lsrgb_l.lutint[p.b] + dither >> 5];
	
			id = (id+1) & 0x3FFF;
	
			if (id==stop)
			{
				id = rand() & 0x3FFF;
				stop = (id + 100 + (rand() & 0x03FF)) & 0x3FFF;
			}
		}
	}
	else	// mode==NODITHER
	{
		for (i=0; i<pixc; i++)
		{
			p = fb.r.l[i];

			fb.r.srgb[i].r = lsrgb_l.lutint[p.r] >> 5;
			fb.r.srgb[i].g = lsrgb_l.lutint[p.g] >> 5;
			fb.r.srgb[i].b = lsrgb_l.lutint[p.b] >> 5;
		}
	}
}

void convert_frgb_to_srgb(framebuffer_t fb, int mode)
{
	int32_t i, id, stop, dither;
	uint32_t dith_on;
	frgb_t p;
	static int init=1;
	static lut_t lsrgb_fl_l, dither_l, bytecheck_l;
	const float black_threshold = (1.f / (255.f*12.92f));
	int32_t pixc = fb.w*fb.h;

	if (init)
	{
		init = 0;

		lsrgb_fl_l = get_lut_lsrgb_fl();
		dither_l = dither_lut_init();
		bytecheck_l = bytecheck_lut_init(4);
	}

	id = rand() & 0x3FFF;
	stop = (id + 100 + (rand() & 0x03FF)) & 0x3FFF;

	if (mode==DITHER)	// Dithering takes about 14 extra cycles per full pixel
	{
		for (i=0; i<pixc; i++)
		{
			p = fb.r.f[i];

			#ifdef _DEBUG
			if (p.r > 1.f)
				p.r = randrange(0., 1.);	// reveals out of range pixels
			if (p.g > 1.f)
				p.g = randrange(0., 1.);
			if (p.b > 1.f)
				p.b = randrange(0., 1.);
			#endif

			dith_on = p.r+p.g+p.b >= black_threshold;	// 0 if the pixel is black, 1 otherwise
			dither = dither_l.lutint[id] * dith_on;
			fb.r.srgb[i].r = bytecheck_l.lutb[lsrgb_fl(p.r, lsrgb_fl_l.lutint) + dither >> 5];		// 8.5 + 2.5 >> 5 = 8.0 sRGB
			fb.r.srgb[i].g = bytecheck_l.lutb[lsrgb_fl(p.g, lsrgb_fl_l.lutint) + dither >> 5];
			fb.r.srgb[i].b = bytecheck_l.lutb[lsrgb_fl(p.b, lsrgb_fl_l.lutint) + dither >> 5];
	
			id = (id+1) & 0x3FFF;
	
			if (id==stop)
			{
				id = rand() & 0x3FFF;
				stop = (id + 100 + (rand() & 0x03FF)) & 0x3FFF;
			}
		}
	}
	else	// mode==NODITHER
	{
		for (i=0; i<pixc; i++)
		{
			p = fb.r.f[i];

			fb.r.srgb[i].r = lsrgb_fl(p.r, lsrgb_fl_l.lutint) >> 5;
			fb.r.srgb[i].g = lsrgb_fl(p.g, lsrgb_fl_l.lutint) >> 5;
			fb.r.srgb[i].b = lsrgb_fl(p.b, lsrgb_fl_l.lutint) >> 5;
		}
	}
}

void blit_lrgb_on_srgb(framebuffer_t fb, srgb_t *srgb0, srgb_t *srgb1)
{
	int32_t i;
	lrgb_t p, l0;
	static int init=1;
	static lut_t lsrgb_l, slrgb_l;
	int32_t pixc = fb.w*fb.h;
	srgb_t s0, s1;

	if (init)
	{
		init = 0;
		lsrgb_l = get_lut_lsrgb();
		slrgb_l = get_lut_slrgb();
	}

	for (i=0; i < pixc; i++)
	{
		p = fb.r.l[i];

		if (p.a > 0)
		{
			if (p.a < ONE)
			{
				s0 = srgb0[i];
				l0.r = slrgb_l.lutint[s0.r];
				l0.g = slrgb_l.lutint[s0.g];
				l0.b = slrgb_l.lutint[s0.b];
				p = blend_alphablend_sep_alpha(l0, p, 32768, LBD_TO_Q15(p.a));
			}

			s1.r = lsrgb_l.lutint[p.r] >> 5;
			s1.g = lsrgb_l.lutint[p.g] >> 5;
			s1.b = lsrgb_l.lutint[p.b] >> 5;

			srgb1[i] = s1;
		}
		else
			srgb1[i] = srgb0[i];
	}
}

void convert_linear_rgb_to_srgb(framebuffer_t fb, int mode)
{
	if (fb.use_cl)
		return ;
	else if (fb.r.use_frgb)
		convert_frgb_to_srgb(fb, mode);
	else
		convert_lrgb_to_srgb(fb, mode);
}

void convert_srgb_to_lrgb(framebuffer_t fb)
{
	int32_t i;
	static int init=1;
	static lut_t slrgb_l;
	int32_t pixc = fb.w*fb.h;

	if (init)
	{
		init = 0;

		slrgb_l = get_lut_slrgb();
	}

	for (i=0; i<pixc; i++)
	{
		fb.r.l[i].r = slrgb_l.lutint[fb.r.srgb[i].r];
		fb.r.l[i].g = slrgb_l.lutint[fb.r.srgb[i].g];
		fb.r.l[i].b = slrgb_l.lutint[fb.r.srgb[i].b];
	}
}

void convert_frgb_to_lrgb(framebuffer_t *fb)
{
	int32_t i, pixc = fb->w*fb->h*4;
	const float offset = (float) (1UL << 23-LBD);		// 23 (mantissa) - 15 (fractional bits of the result) = 8 (offset)
	float *pf, v;
	uint16_t *pl;
	uint32_t *vint = (uint32_t *) &v;

	if (fb->r.l==NULL)
		fb->r.l = calloc (fb->w*fb->h, sizeof(lrgb_t));

	pf = (float *) fb->r.f;
	pl = (uint16_t *) fb->r.l;

	for (i=0; i<pixc; i++)
	{
		v = pf[i] + offset;	// adding the offset puts the correct integer value in the mantissa
		pl[i] = *vint;		// keeps mantissa, no need for a mask
	}
}

void convert_frgb_to_lrgb_ratio(framebuffer_t *fb, const float ratio)
{
	int32_t i, pixc = fb->w*fb->h*4;
	const float offset = (float) (1UL << 23-LBD);		// 23 (mantissa) - 15 (fractional bits of the result) = 8 (offset)
	float *pf, v;
	uint16_t *pl;
	uint32_t *vint = (uint32_t *) &v;

	if (fb->r.l==NULL)
		fb->r.l = calloc (fb->w*fb->h, sizeof(lrgb_t));

	pf = (float *) fb->r.f;
	pl = (uint16_t *) fb->r.l;

	for (i=0; i<pixc; i++)
	{
		v = pf[i]*ratio + offset;	// adding the offset puts the correct integer value in the mantissa
		pl[i] = *vint;			// keeps mantissa, no need for a mask
	}
}

srgb_t srgb_change_order_pixel(const srgb_t in, const int order)
{
	srgb_t out={0};

	switch (order)
	{
		case ORDER_RGBA:
			return in;

		case ORDER_ABGR:
			out.r = in.a;
			out.g = in.b;
			out.b = in.g;
			out.a = in.r;
			break;

		case ORDER_BGRA:
			out.r = in.b;
			out.g = in.g;
			out.b = in.r;
			out.a = in.a;
			break;

		case ORDER_ARGB:
			out.r = in.a;
			out.g = in.r;
			out.b = in.g;
			out.a = in.b;
			break;
	}

	return out;
}

void srgb_change_order(srgb_t *in, srgb_t *out, const size_t count, const int order)
{
	if (order == ORDER_RGBA)
		if (in == out)
			return ;
		else
			memcpy(out, in, count * sizeof(srgb_t));

	for (size_t i=0; i < count; i++)
		out[i] = srgb_change_order_pixel(in[i], order);
}
