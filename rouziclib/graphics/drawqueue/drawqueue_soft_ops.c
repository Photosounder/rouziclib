static float dqsb_gaussian(float x)
{
	return expf(-x*x);
}

static frgb_t dqsb_load_pixel(const float *block, int ib, int chan_stride)
{
	frgb_t p;

	// Load one pixel from the four planar channels
	p.r = block[ib];
	p.g = block[chan_stride + ib];
	p.b = block[2*chan_stride + ib];
	p.a = block[3*chan_stride + ib];
	return p;
}

static void dqsb_store_pixel(float *block, int ib, int chan_stride, frgb_t p)
{
	// Store one pixel into the four planar channels
	block[ib] = p.r;
	block[chan_stride + ib] = p.g;
	block[2*chan_stride + ib] = p.b;
	block[3*chan_stride + ib] = p.a;
}

int dqsb_bracket_open(float **block, int brlvl, int chan_stride)
{
	// Start a transparent child layer while preserving its parent
	if (brlvl >= DQS_BRACKET_LEVELS)
		return brlvl;
	brlvl++;
	memset(block[brlvl], 0, 4*chan_stride*sizeof(float));
	return brlvl;
}

int dqsb_bracket_close(float **block, int brlvl, enum dq_blend blendmode, int chan_stride)
{
	int i, ic;
	float bg, fg, alpha;

	// Ignore an unmatched closing bracket
	if (brlvl <= 0)
		return 0;

	// Composite every child channel into its parent with OpenCL semantics
	for (i=0; i < chan_stride; i++)
	{
		alpha = block[brlvl][3*chan_stride + i];
		for (ic=0; ic < 4; ic++)
		{
			bg = block[brlvl-1][ic*chan_stride + i];
			fg = block[brlvl][ic*chan_stride + i];

			// Apply the requested bracket blend mode
			switch (blendmode)
			{
				case DQB_ADD:	bg += fg;					break;
				case DQB_SUB:	bg -= fg;					break;
				case DQB_MUL:	bg *= fg;					break;
				case DQB_DIV:	bg /= fg;					break;
				case DQB_BLEND:	bg = fg*alpha + bg*(1.f-alpha);		break;
				case DQB_SOLID:	bg = fg;					break;
				case DQB_BLEND_PREMUL:	bg = fg + bg*(1.f-alpha);		break;
				default:		bg += fg;					break;
			}
			block[brlvl-1][ic*chan_stride + i] = bg;
		}
	}
	return brlvl-1;
}

void dqsb_draw_black_rect_inv(float *le, float *block, xy_t start_pos, int bs, int chan_stride)
{
	int x, y, ib=0, ic;
	float d0x, d0y, d1x, d1y, weight;

	// Multiply the block by the inverse rectangle coverage
	for (y=0; y < bs; y++)
		for (x=0; x < bs; x++, ib++)
		{
			d0x = (start_pos.x+x-le[0])*le[4];
			d0y = (start_pos.y+y-le[1])*le[4];
			d1x = (start_pos.x+x-le[2])*le[4];
			d1y = (start_pos.y+y-le[3])*le[4];
			weight = 0.25f * (erf_fastf(d0x)-erf_fastf(d1x)) * (erf_fastf(d0y)-erf_fastf(d1y));
			weight = weight*le[5] + (1.f-le[5]);
			for (ic=0; ic < 4; ic++)
				block[ic*chan_stride + ib] *= weight;
		}
}

void dqsb_draw_polygon(float *le, int point_count, float *block, xy_t start_pos, int bs, int chan_stride)
{
	int x, y, i, ib=0, ic;
	float weight;
	xyf_t p[4], pf;

	// Evaluate the analytic polygon coverage at every sector pixel
	for (y=0; y < bs; y++)
		for (x=0; x < bs; x++, ib++)
		{
			pf = xyf(start_pos.x+x, start_pos.y+y);
			for (i=0; i < point_count; i++)
			{
				p[i].x = (le[4+2*i] - pf.x) * le[0];
				p[i].y = (le[5+2*i] - pf.y) * le[0];
			}

			// Sum the oriented subtriangle weights around the pixel
			weight = 0.f;
			for (i=0; i < point_count; i++)
				weight += calc_subtriangle_pixel_weight(p[i], p[(i+1)%point_count]);

			// Add the weighted colour including opaque alpha
			for (ic=0; ic < 3; ic++)
				block[ic*chan_stride + ib] += weight*le[1+ic];
			block[3*chan_stride + ib] += weight;
		}
}

void dqsb_draw_effect(enum dq_type type, float *le, float *block, int chan_stride)
{
	int i, ic;
	float v=le[0], grey0, lvl_perc, ratio;
	frgb_t p, r;

	// Apply the selected whole-layer effect one pixel at a time
	for (i=0; i < chan_stride; i++)
	{
		p = dqsb_load_pixel(block, i, chan_stride);

		// Match the component-wise OpenCL effect formulas
		switch (type)
		{
			case DQT_GAIN:
				p.r *= v; p.g *= v; p.b *= v; p.a *= v;
				break;

			case DQT_GAIN_PARAB:
				p.r = 1.f-powf(1.f-MINN(p.r, 1.f), v);
				p.g = 1.f-powf(1.f-MINN(p.g, 1.f), v);
				p.b = 1.f-powf(1.f-MINN(p.b, 1.f), v);
				p.a = 1.f-powf(1.f-MINN(p.a, 1.f), v);
				break;

			case DQT_LUMA_COMPRESS:
				grey0 = 0.16f*p.r + 0.73f*p.g + 0.11f*p.b;
				if (grey0 != 0.f)
				{
					// Derive the perceptual-lightness scale applied by OpenCL
					lvl_perc = linear_to_Lab_L(v);
					ratio = ((Lab_L_to_linear(linear_to_Lab_L(grey0)+lvl_perc)-v) / grey0)
						/ (Lab_L_to_linear(1.f+lvl_perc)-v);
					p.r *= ratio; p.g *= ratio; p.b *= ratio; p.a *= ratio;
				}
				break;

			case DQT_COL_MATRIX:
				r.r = le[0]*p.r + le[3]*p.g + le[6]*p.b;
				r.g = le[1]*p.r + le[4]*p.g + le[7]*p.b;
				r.b = le[2]*p.r + le[5]*p.g + le[8]*p.b;
				r.a = p.a;
				p = r;
				break;

			case DQT_CLIP:
				p.r = MINN(p.r, v); p.g = MINN(p.g, v);
				p.b = MINN(p.b, v); p.a = MINN(p.a, v);
				break;

			case DQT_CLAMP:
				p.r = rangelimitf(p.r, 0.f, 1.f);
				p.g = rangelimitf(p.g, 0.f, 1.f);
				p.b = rangelimitf(p.b, 0.f, 1.f);
				p.a = rangelimitf(p.a, 0.f, 1.f);
				break;

			case DQT_GAMMA_BANDAID:
				p.r = slrgb(powf(p.r, v));
				p.g = slrgb(powf(p.g, v));
				p.b = slrgb(powf(p.b, v));
				break;

			default:
				break;
		}
		dqsb_store_pixel(block, i, chan_stride, p);
	}
}

void dqsb_draw_circle(enum dq_type type, float *le, float *block, xy_t start_pos, int bs, int chan_stride)
{
	int x, y, ib=0, ic;
	float dx, dy, dc, dn, df, weight;

	// Evaluate radial coverage and apply the selected circle operation
	for (y=0; y < bs; y++)
		for (x=0; x < bs; x++, ib++)
		{
			dx = start_pos.x+x-le[0];
			dy = start_pos.y+y-le[1];
			dc = sqrtf(dx*dx + dy*dy);
			dn = (le[2]-dc)*le[3];
			df = -(le[2]+dc)*le[3];

			// Select filled, black, or hollow coverage
			if (type==DQT_CIRCLE_HOLLOW)
				weight = dqsb_gaussian(dn) + dqsb_gaussian(df);
			else
				weight = 0.5f*(erf_fastf(dn)-erf_fastf(df));

			// Add colour or multiply the existing pixel
			if (type==DQT_CIRCLE_BLACK)
			{
				weight = 1.f-weight*le[4];
				for (ic=0; ic < 4; ic++)
					block[ic*chan_stride + ib] *= weight;
			}
			else
			{
				for (ic=0; ic < 3; ic++)
					block[ic*chan_stride + ib] += weight*le[4+ic];
				block[3*chan_stride + ib] += weight;
			}
		}
}

void dqsb_draw_gradient_test(float *block, xy_t start_pos, int bs, int chan_stride, xyi_t frame_dim)
{
	int x, y, ib=0, ic;
	float v;

	// Replace the sector with the OpenCL diagnostic gradient
	for (y=0; y < bs; y++)
		for (x=0; x < bs; x++, ib++)
		{
			v = -(start_pos.x+x-frame_dim.x*0.5f)/(frame_dim.x*0.1f);
			for (ic=0; ic < 4; ic++)
				block[ic*chan_stride + ib] = v;
		}
}
