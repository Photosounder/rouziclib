static frgb_t draw_effect_apply_pixel_direct(frgb_t p, enum dq_type effect_type, float v, const float *matrix)
{
	float grey0_lin, grey0_perc, grey1_perc, grey1_lin, white1_perc, white1_lin, ratio;
	frgb_t r;

	// Apply the OpenCL draw effect formulas to one direct framebuffer pixel
	switch (effect_type)
	{
		case DQT_GAIN:
			// Scale all colour and alpha channels linearly
			p.r *= v;
			p.g *= v;
			p.b *= v;
			p.a *= v;
			break;

		case DQT_GAIN_PARAB:
			// Apply the highlight-preserving parabolic gain curve
			p.r = 1.f - fastpow(1.f - MINN(p.r, 1.f), v);
			p.g = 1.f - fastpow(1.f - MINN(p.g, 1.f), v);
			p.b = 1.f - fastpow(1.f - MINN(p.b, 1.f), v);
			//p.a = 1.f - fastpow(1.f - MINN(p.a, 1.f), v);
			break;

		case DQT_LUMA_COMPRESS:
			// Scale the pixel using its perceptual lightness shift
			grey0_lin = WEIGHT_R*p.r + WEIGHT_G*p.g + WEIGHT_B*p.b;
			if (grey0_lin==0.f)
				break;
			grey0_perc = linear_to_Lab_L(grey0_lin);
			grey1_perc = grey0_perc + linear_to_Lab_L(v);
			grey1_lin = Lab_L_to_linear(grey1_perc) - v;
			white1_perc = 1.f + linear_to_Lab_L(v);
			white1_lin = Lab_L_to_linear(white1_perc) - v;
			ratio = (grey1_lin / grey0_lin) / white1_lin;
			p.r *= ratio;
			p.g *= ratio;
			p.b *= ratio;
			//p.a *= ratio;
			break;

		case DQT_COL_MATRIX:
			// Transform RGB with the draw queue's column-major matrix layout
			r.r = matrix[0]*p.r + matrix[3]*p.g + matrix[6]*p.b;
			r.g = matrix[1]*p.r + matrix[4]*p.g + matrix[7]*p.b;
			r.b = matrix[2]*p.r + matrix[5]*p.g + matrix[8]*p.b;
			r.a = p.a;
			p = r;
			break;

		case DQT_CLIP:
			// Limit every channel to the requested upper bound
			p.r = MINN(p.r, v);
			p.g = MINN(p.g, v);
			p.b = MINN(p.b, v);
			p.a = MINN(p.a, v);
			break;

		case DQT_CLAMP:
			// Clamp every channel to the displayable linear range
			p.r = rangelimitf(p.r, 0.f, 1.f);
			p.g = rangelimitf(p.g, 0.f, 1.f);
			p.b = rangelimitf(p.b, 0.f, 1.f);
			p.a = rangelimitf(p.a, 0.f, 1.f);
			break;

		case DQT_GAMMA_BANDAID:
			// Apply the gamma workaround to RGB while preserving alpha
			p.r = slrgb(fastpow(p.r, v));
			p.g = slrgb(fastpow(p.g, v));
			p.b = slrgb(fastpow(p.b, v));
			break;

		default:
			// Leave non-effect draw queue entries unchanged
			break;
	}

	return p;
}

static lrgb_t draw_effect_frgb_to_lrgb_direct(frgb_t p)
{
	// Clamp both ends before converting to the unsigned fixed-point framebuffer
	p.r = rangelimitf(p.r, 0.f, 1.f);
	p.g = rangelimitf(p.g, 0.f, 1.f);
	p.b = rangelimitf(p.b, 0.f, 1.f);
	p.a = rangelimitf(p.a, 0.f, 1.f);
	return frgb_to_lrgb(p);
}

static void draw_effect_direct(enum dq_type effect_type, float v, const float *matrix)
{
	size_t i, pixel_count;

	// Ignore effects while drawing is explicitly discarded
	if (fb->discard)
		return;
	pixel_count = mul_x_by_y_xyi(fb->r.dim);

	// Transform the active direct bracket layer or base FRGB framebuffer in place
	if (fb->r.use_frgb)
	{
		for (i=0; i < pixel_count; i++)
			fb->r.f[i] = draw_effect_apply_pixel_direct(fb->r.f[i], effect_type, v, matrix);
		return;
	}

	// Transform LRGB through the same formulas and quantise back to fixed point
	for (i=0; i < pixel_count; i++)
	{
		frgb_t p = lrgb_to_frgb(fb->r.l[i]);
		p = draw_effect_apply_pixel_direct(p, effect_type, v, matrix);
		fb->r.l[i] = draw_effect_frgb_to_lrgb_direct(p);
	}
}

// Drawqueue
void draw_effect_noarg_dq(enum dq_type effect_type)
{
	float *df = drawq_add_to_main_queue(effect_type);
	if (df==NULL)
		return;

	drawq_add_sectors_for_already_set_sectors();
}

void draw_effect_arg_double_dq(enum dq_type effect_type, double v)
{
	float *df = drawq_add_to_main_queue(effect_type);
	if (df==NULL)
		return;
	df[0] = v;

	drawq_add_sectors_for_already_set_sectors();
}

void draw_colour_matrix_dq(double *matrix)
{
	float *df = drawq_add_to_main_queue(DQT_COL_MATRIX);
	if (df==NULL)
		return;

	for (int i=0; i < 9; i++)
		df[i] = isnan(matrix[i]) ? 0.f : matrix[i];

	drawq_add_sectors_for_already_set_sectors();
}

// Drawqueue-enqueue
void draw_effect_noarg_dqnq(enum dq_type effect_type)
{
	const enum dqnq_type type = DQNQT_EFFECT_NOARG;

	// Get pointer to data buffer
	uint8_t *p = (uint8_t *) dqnq_new_entry(type);

	// Write arguments to buffer
	write_LE32(&p, effect_type);

	dqnq_finish_entry(type);
}

void draw_effect_arg_double_dqnq(enum dq_type effect_type, double v)
{
	const enum dqnq_type type = DQNQT_EFFECT_FL1;

	// Get pointer to data buffer
	uint8_t *p = (uint8_t *) dqnq_new_entry(type);

	// Write arguments to buffer
	write_LE32(&p, effect_type);
	write_LE32(&p, float_as_u32(v));

	dqnq_finish_entry(type);
}

void draw_colour_matrix_dqnq(double *matrix)
{
	const enum dqnq_type type = DQNQT_COL_MATRIX;

	// Get pointer to data buffer
	uint8_t *p = (uint8_t *) dqnq_new_entry(type);

	// Write arguments to buffer
	for (int i=0; i < 9; i++)
		write_LE32(&p, float_as_u32(matrix[i]));

	dqnq_finish_entry(type);
}

// Generic
void draw_effect_noarg(enum dq_type effect_type)
{
	// Execute framebuffer effects immediately when no draw queue is active
	if (fb->use_drawq==0)
	{
		draw_effect_direct(effect_type, 0.f, NULL);
		return;
	}

	if (fb->use_dqnq)
		draw_effect_noarg_dqnq(effect_type);
	else
		draw_effect_noarg_dq(effect_type);
}

void draw_effect_arg_double(enum dq_type effect_type, double v)
{
	// Execute framebuffer effects immediately when no draw queue is active
	if (fb->use_drawq==0)
	{
		draw_effect_direct(effect_type, (float) v, NULL);
		return;
	}

	if (fb->use_dqnq)
		draw_effect_arg_double_dqnq(effect_type, v);
	else
		draw_effect_arg_double_dq(effect_type, v);
}

void draw_colour_matrix(double *matrix)
{
	float matrix_f[9];

	// Execute the colour matrix immediately when no draw queue is active
	if (fb->use_drawq==0)
	{
		// Match queued matrix storage precision and NaN handling
		for (int i=0; i < 9; i++)
			matrix_f[i] = isnan(matrix[i]) ? 0.f : matrix[i];
		draw_effect_direct(DQT_COL_MATRIX, 0.f, matrix_f);
		return;
	}

	if (fb->use_dqnq)
		draw_colour_matrix_dqnq(matrix);
	else
		draw_colour_matrix_dq(matrix);
}
