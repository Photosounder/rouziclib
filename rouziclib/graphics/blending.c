// The following functions blend a whole foreground pixel with a background pixel by a ratio of p (1.15 fixed-point format)

void blend_solid(lrgb_t *bg, lrgb_t fg, int32_t p)
{
	*bg = fg;
}

void blend_add(lrgb_t *bg, lrgb_t fg, int32_t p)
{
	int32_t r, g, b;

	r = (fg.r * p >> 15) + bg->r;	if (r>ONE) bg->r = ONE; else bg->r = r;
	g = (fg.g * p >> 15) + bg->g;	if (g>ONE) bg->g = ONE; else bg->g = g;
	b = (fg.b * p >> 15) + bg->b;	if (b>ONE) bg->b = ONE; else bg->b = b;
}

void blend_add_fl(frgb_t *bg, frgb_t fg, float p)
{
	float r, g, b;

	bg->r += fg.r * p;
	bg->g += fg.g * p;
	bg->b += fg.b * p;
}

void blend_add_limit_fl(frgb_t *bg, frgb_t fg, float p)
{
	blend_add_fl(bg, fg, p);
	rangelimit_frgb(bg);
}

void blend_sub(lrgb_t *bg, lrgb_t fg, int32_t p)
{
	int32_t r, g, b;

	r = -(fg.r * p >> 15) + bg->r;	if (r<0) bg->r = 0; else bg->r = r;
	g = -(fg.g * p >> 15) + bg->g;	if (g<0) bg->g = 0; else bg->g = g;
	b = -(fg.b * p >> 15) + bg->b;	if (b<0) bg->b = 0; else bg->b = b;
}

void blend_mul(lrgb_t *bg, lrgb_t fg, int32_t p)
{
	int32_t pinv;

	pinv = 32768 - p;
	bg->r = (bg->r*fg.r >> LBD) * p + bg->r * pinv >> 15;
	bg->g = (bg->g*fg.g >> LBD) * p + bg->g * pinv >> 15;
	bg->b = (bg->b*fg.b >> LBD) * p + bg->b * pinv >> 15;
}

void blend_mul4(lrgb_t *bg, lrgb_t fg, int32_t p)	// multiply by a quarter intensity image (1.0 == sRGB value of 137)
{
	int32_t r, g, b, pinv;

	pinv = 32768 - p;
	r = (bg->r*fg.r >> (LBD-2)) * p + bg->r * pinv >> 15;	if (r>ONE) bg->r = ONE; else bg->r = r;
	g = (bg->g*fg.g >> (LBD-2)) * p + bg->g * pinv >> 15;	if (g>ONE) bg->g = ONE; else bg->g = g;
	b = (bg->b*fg.b >> (LBD-2)) * p + bg->b * pinv >> 15;	if (b>ONE) bg->b = ONE; else bg->b = b;
}

void blend_blend(lrgb_t *bg, lrgb_t fg, int32_t p)
{
	int32_t pinv;

	pinv = 32768 - p;
	bg->r = fg.r * p + bg->r * pinv >> 15;
	bg->g = fg.g * p + bg->g * pinv >> 15;
	bg->b = fg.b * p + bg->b * pinv >> 15;
}

void blend_alphablend(lrgb_t *bg, lrgb_t fg, int32_t p)	// proper alpha blending when both pixels might have any possible alpha value
{
	int32_t r, g, b, a, pinv, fba;

	pinv = 32768 - p;

	#if LBD == 15
	a = bg->a;
	#else
	a = 32768 * bg->a >> LBD;		// 1.15
	#endif

	bg->a = ONE * p + bg->a * pinv >> 15;	// FIXME fg.a is assumed to be 1.0
	if (bg->a > 0)
	{
		#if LBD == 15
		fba = bg->a;
		#else
		fba = 32768 * bg->a >> LBD;
		#endif

		r = (fg.r*p + (bg->r * a >> 15)*pinv) / fba;
		g = (fg.g*p + (bg->g * a >> 15)*pinv) / fba;
		b = (fg.b*p + (bg->b * a >> 15)*pinv) / fba;

		if (r>ONE) bg->r = ONE; else bg->r = r;
		if (g>ONE) bg->g = ONE; else bg->g = g;
		if (b>ONE) bg->b = ONE; else bg->b = b;
	}
	else
	{
		bg->r = 0;
		bg->g = 0;
		bg->b = 0;
	}
}

void blend_alphablendfg(lrgb_t *bg, lrgb_t fg, int32_t p)	// alpha blending (doesn't take framebuffer's alpha into account though, assumed to be 1.0)
{
	int32_t pinv;

	p = p * fg.a >> LBD;
	pinv = 32768 - p;

	bg->r = fg.r * p + bg->r * pinv >> 15;
	bg->g = fg.g * p + bg->g * pinv >> 15;
	bg->b = fg.b * p + bg->b * pinv >> 15;
}

void blend_blendalphaonly(lrgb_t *bg, lrgb_t fg, int32_t p)
{
	int32_t pinv;

	pinv = 32768 - p;
	bg->a = fg.a * p + bg->a * pinv >> 15;
}

blend_func_fl_t get_blend_fl_equivalent(const blend_func_t bf)
{
	if (bf==blend_add)
		return blend_add_limit_fl;

	return blend_add_limit_fl;
}
