float *make_windowing_block(compression_param1_t cp)
{
	float *block_coef;
	xyi_t ip;
	int ref = cp.window_base_count * cp.block_size;

	block_coef = calloc(cp.block_size*cp.block_size, sizeof(float));

	// Make reference line
	for (ip.x=0; ip.x < cp.window_base_count; ip.x++)
		block_coef[ref+ip.x] = cp.window_base[ip.x];
	for (; ip.x < cp.block_size+1>>1; ip.x++)
		block_coef[ref+ip.x] = 1.f;
	for (; ip.x < cp.block_size; ip.x++)
		block_coef[ref+ip.x] = block_coef[ref+cp.block_size-1-ip.x];

	// Do the rest based on the reference line
	for (ip.y=0; ip.y < cp.block_size; ip.y++)
		if (ip.y*cp.block_size != ref)
			for (ip.x=0; ip.x < cp.block_size; ip.x++)
				block_coef[ip.y*cp.block_size+ip.x] = block_coef[ref+ip.y] * block_coef[ref+ip.x];

	return block_coef;
}

xyz_t compr_frgb_to_hsl(frgb_t col)	// input is linear
{
	//return frgb_to_xyz(col);
	xyz_t hsl;
	double wr=0.124, wg=0.686, wb=0.19;

	rgb_to_hsl_cw(wr, wg, wb, col.r, col.g, col.b, &hsl.x, &hsl.y, &hsl.z, HUERAD);
	hsl.x *= 1./(2.*pi);				// H in turns
	hsl.z = linear_to_Lab_L(hsl.z);			// L in perceptual scale

	//if (hsl.z < 0.2)				// reduce S in the darks
	//	hsl.y *= hsl.z / 0.2;

	return hsl;
}

frgb_t compr_hsl_to_frgb(xyz_t hsl)
{
	//return linear_to_Lab_L_f(xyz_to_frgb(hsl));
	xyz_t rgb;
	double wr=0.124, wg=0.686, wb=0.19;

	hsl.x *= 3.;					// H in HUE03
	/*if (hsl.z < 0.2)
	{
		hsl.y *= 0.2 / hsl.z;			// unreduce S in the darks
		hsl.y = MINN(1., hsl.y);
	}*/
	hsl.z = Lab_L_to_linear(hsl.z);			// linear L
	hsl_to_rgb_cw(wr, wg, wb, hsl.x, hsl.y, hsl.z, &rgb.x, &rgb.y, &rgb.z, HUE03, 0);

	return linear_to_Lab_L_f(xyz_to_frgb(rgb));
}

uint32_t bits_to_mask(const int bits)	// 7 becomes 0x7F
{
	return (1UL << bits) - 1;
}

double bits_to_mul(const int bits)	// 7 becomes 127.
{
	return (double) ((1ULL << bits) - 1);
}

void make_compr_hsl(compression_param1_t cp, xyz_t hsl, int *ih, int *is, int *il)
{
	*ih = hsl.x * (bits_to_mul(cp.bits_ch)-1.) + 0.5;	// hue is 0 to 126, 127 is reserved for no saturation
	*is = hsl.y * (bits_to_mul(cp.bits_cs)+1.) + 0.5;	// hue is temporarily 0 to 17
	*il = hsl.z * bits_to_mul(cp.bits_cl) + 0.5;

	if (*is==0 || *il==0)					// if there's no saturation or luma
	{
		*is = 0;
		*ih = bits_to_mask(cp.bits_ch);		// the hue is the reserved value of 127
	}
	else
		*is -= 1;					// otherwise since is 0 is never used move every value one position down
}

xyz_t compr_hsl_to_xyz(compression_param1_t cp, int ih, int is, int il)
{
	xyz_t hsl;

	hsl.x = (double) ih / (bits_to_mul(cp.bits_ch)-1.);
	hsl.y = (double) (is+1) / (bits_to_mul(cp.bits_cs)+1.);
	hsl.z = (double) il / bits_to_mul(cp.bits_cl);

	if (ih == bits_to_mask(cp.bits_ch))		// if the hue is the reserved value of no saturation
	{
		hsl.x = 0.;
		hsl.y = 0.;
	}

	return hsl;
}

void calc_compression_param1(compression_param1_t *cp)
{
	cp->bits_per_block = (cp->bits_ch+cp->bits_cs)*3+cp->bits_cl*2 + cp->bits_erf_th + cp->bits_erf_off + cp->bits_erf_width + cp->bits_per_pixel*cp->block_size*cp->block_size;
	cp->block_step = cp->block_size - cp->window_base_count;
	cp->odd_offset = cp->quincunx ? -cp->block_size/2 : 0;
	cp->block_centre = set_xy(((double) cp->block_size-1.) / 2.);
}

double erf_block_rms(compression_param1_t cp, float *block_coef, frgb_t *block, double *c)
{
	int i;
	xyi_t ip;
	xy_t rp;
	double verf, sum=0.;
	static double costh, sinth, prev_c4=-100.;

	if (prev_c4 != c[4])
	{
		costh = fastcos_tr_d2(c[4]);
		sinth = fastsin_tr_d2(c[4]);
		prev_c4 = c[4];
	}

	for (ip.y=0; ip.y < cp.block_size; ip.y++)
	{
		rp.y = ((double) ip.y-cp.block_centre.y) * costh;
		for (ip.x=0; ip.x < cp.block_size; ip.x++)
		{
			i = ip.y*cp.block_size + ip.x;
			if (isnan(block[i].a)==0)
			{
				rp.x = ((double) ip.x-cp.block_centre.x) * sinth;

				verf = c[2] + c[3] * fasterfrf_d0((rp.x + rp.y + c[1]) * c[0]);
				sum += sq(verf - block[i].a) * block_coef[i];
			}
		}
	}

	return sum;
}

void fit_erf_block_bruteforce(compression_param1_t cp, float *block_coef, frgb_t *block, double lum0, double lum1, double *c)
{
	int i, print=0, stage=0;
	double rms, min_rms, min_c[5];

	if (lum0 < 0.3 && lum1 > 0.7)
		print = 1;

	c[0] = 1.;		// erf width
	c[1] = 0.;		// erf side offset
	c[2] = lum0;
	c[3] = lum1-lum0;

	min_rms=1e9;
	for (c[4] = 0.; c[4] < 1.; c[4]+=1./16.)
	for (c[2] = lum0-0.05; c[2] <= lum0+0.05; c[2]+=0.025)
	for (c[3] = lum1-lum0-0.05; c[3] <= lum1-lum0+0.05; c[3]+=0.025)
	for (c[0] = 1.25; c[0] > 1.2/*0.5/(double) cp.block_size*/; c[0]*=0.7)
	for (c[1] = -0.5*cp.block_size; c[1] <= 0.5*cp.block_size; c[1]+=0.5)
	{
		rms = erf_block_rms(cp, block_coef, block, c);
		if (rms < min_rms)
		{
				min_rms = rms;
			memcpy(min_c, c, 5*sizeof(double));
			if (print) fprintf_rl(stdout, "rms for width %.2f  side offset %.1f  angle %.3f: %.3f\n", c[0], c[1], c[4], rms);
		}
	}
	memcpy(c, min_c, 5*sizeof(double));

#if 0
	// Test angles
test_angle:
	/*for (min_rms=1e9, c[4]=0.; c[4] < 1.; c[4]+=1./16.)
	{
		rms = erf_block_rms(cp, block_coef, block, c);
		if (rms < min_rms)
		{
			min_rms = rms;
			min_c = c[4];
		}
		if (print) fprintf_rl(stdout, "rms for angle %.0f/16: %.3f\n", c[4]*16., rms);
	}
	c[4] = min_c;*/

	// Test side offset
	for (min_rms=1e9, c[1] = -1.4*cp.block_size; c[1] <= 1.4*cp.block_size; c[1]+=1.)
	{
		rms = erf_block_rms(cp, block_coef, block, c);
		if (rms < min_rms)
		{
			min_rms = rms;
			min_c = c[1];
		}
		if (print) fprintf_rl(stdout, "rms for side offset %.1f: %.3f\n", c[1], rms);
	}
	c[1] = min_c;

	if (stage==0)
	{
		stage++;
		goto test_angle;
	}

	// Test erf width
	for (min_rms=1e9, c[0] = 1.25; c[0] > 0.1/(double) cp.block_size; c[0]*=0.7)
	{
		rms = erf_block_rms(cp, block_coef, block, c);
		if (rms < min_rms)
		{
			min_rms = rms;
			min_c = c[0];
		}
		if (print) fprintf_rl(stdout, "rms for erf width %.2f: %.3f\n", c[0], rms);
	}
	c[0] = min_c;

	if (stage<=3)
	{
		stage++;
		goto test_angle;
	}
#endif

	if (print) fprintf_rl(stdout, "Min RMS: %.3f   width %.2f  offset %.1f  angle %.0f/16\n\n", min_rms, c[0], c[1], c[4]*16.);
}

int cmp_frgb_alpha(const frgb_t *a, const frgb_t *b)
{
	if (isnan(a->a))	return -1;
	if (isnan(b->a))	return 1;
	if (a->a > b->a)	return 1;
	if (a->a == b->a)	return 0;
	return -1;
}

raster_t frgb_to_compressed_texture(raster_t r0, compression_param1_t *cp_in)
{
	double wr=0.124, wg=0.686, wb=0.19, c[5];
	double sum, sumw, mean_lum, sd, s0, s1, s2, bary0w, bary1w, erfw, sum0, sum0w, sum1, sum1w, mean_lum0, mean_lum1, erf_th, erf_off, erf_width;
	xy_t bary0, bary1;
	raster_t r1={0};
	xyi_t ip, ib;
	int i, yb;
	compression_param1_t cp={0};
	frgb_t *b0, *bs, col0, col1, colmid, mean_col;
	float *block_coef;
	xyz_t hsl0, hsl1, hslm;
	buffer_t buf={0};
	size_t di=0;

	r1.dim = r0.dim;
	r1.buf_fmt = 20;

	// Compression parameters initialisation
	if (cp_in)
		cp = *cp_in;
	else
	{
		cp.block_size = 8;
		cp.window_base = calloc(cp.window_base_count = 0, sizeof(float));
		//cp.window_base[0] = 6.f/12.f;
		/*cp.window_base[0] = 2.f/12.f;
		cp.window_base[1] = 6.f/12.f;
		cp.window_base[2] = 10.f/12.f;*/

		cp.quincunx = 1;
		cp.bits_ch = 7;
		cp.bits_cs = 4;
		cp.bits_cl = 8;

		if (0)
		{
			cp.bits_erf_th = 6;
			cp.bits_erf_off = 8;
			cp.bits_erf_width = 6;
		}
		else
			cp.bits_per_pixel = 3;
	}

	calc_compression_param1(&cp);
	block_coef = make_windowing_block(cp);
	
	// Write parameters necessary for decoding to buffer
	bufwrite_LE16(&buf, cp.block_size);
	bufwrite_LE16(&buf, cp.bits_per_block);
	bufwrite_byte8(&buf, cp.window_base_count);
	for (i=0; i < cp.window_base_count; i++)
		bufwrite_LE32(&buf, float_as_u32(cp.window_base[i]));
	bufwrite_byte8(&buf, cp.quincunx);
	bufwrite_byte8(&buf, cp.bits_ch);
	bufwrite_byte8(&buf, cp.bits_cs);
	bufwrite_byte8(&buf, cp.bits_cl);
	bufwrite_byte8(&buf, cp.bits_per_pixel);
	if (cp.bits_per_pixel==0)
	{
		bufwrite_byte8(&buf, cp.bits_erf_th);
		bufwrite_byte8(&buf, cp.bits_erf_off);
		bufwrite_byte8(&buf, cp.bits_erf_width);
	}
	di = buf.len * 8;
	if (cp_in==NULL)
		free(cp.window_base);

	b0 = calloc(cp.block_size*cp.block_size, sizeof(frgb_t));
	bs = calloc(cp.block_size*cp.block_size, sizeof(frgb_t));

	// Go through blocks in quincunx formation
	for (yb=0, ip.y = -cp.window_base_count; ip.y < r1.dim.y; ip.y+=cp.block_step, yb++)
	{
		for (ip.x = -cp.window_base_count + cp.odd_offset*(yb & 1); ip.x < r1.dim.x; ip.x+=cp.block_step)
		{
			sum = 0.;
			sumw = 0.;
			memset(&mean_col, 0, sizeof(frgb_t));

			// Make input block
			for (ib.y=0; ib.y < cp.block_size; ib.y++)
				for (ib.x=0; ib.x < cp.block_size; ib.x++)
				{
					i = ib.y*cp.block_size + ib.x;
					// Copy from r0
					if (ip.y+ib.y >=0 && ip.y+ib.y < r0.dim.y && ip.x+ib.x >=0 && ip.x+ib.x < r0.dim.x)
					{
						//b0[i] = r0.f[(ip.y+ib.y)*r0.dim.x + (ip.x+ib.x)];
						b0[i] = get_raster_pixel_in_frgb(r0, (ip.y+ib.y)*r0.dim.x + (ip.x+ib.x));
						
						// "Alpha" is the perceptual luminosity
						b0[i].a = linear_to_Lab_L(b0[i].r*wr + b0[i].g*wg + b0[i].b*wb);
						b0[i] = linear_to_Lab_L_f(b0[i]);
						sum += b0[i].a * block_coef[i];
						sumw += block_coef[i];
						mean_col = add_frgb(mean_col, mul_scalar_frgb(b0[i], block_coef[i]));
					}
					else
					{
						b0[i] = make_grey_f(0.);
						b0[i].a = NAN;
					}
					bs[i] = b0[i];
				}

			qsort(bs, cp.block_size*cp.block_size, sizeof(frgb_t), cmp_frgb_alpha);

			mean_lum = sum / sumw;
			mean_col = mul_scalar_frgb(mean_col, 1./sumw);
			sd = s0 = s1 = s2 = 0.;

			if (cp.bits_per_pixel==0)
			{
			// Calculate standard deviation from mean lum
			for (ib.y=0; ib.y < cp.block_size; ib.y++)
				for (ib.x=0; ib.x < cp.block_size; ib.x++)
				{
					i = ib.y*cp.block_size + ib.x;
					if (isnan(b0[i].a)==0)
					{
						s0 += block_coef[i];
						s1 += b0[i].a * block_coef[i];
						s2 += sq(b0[i].a) * block_coef[i];
					}
				}

			sd = sqrt(MAXN(0, s0*s2 - sq(s1))) / s0;		// the fitting will be based on erf(t/sd)

			// Calculate the weighted barycentres and mean lum for each side
			bary0 = bary1 = XY0;
			bary0w = bary1w = 0.;
			sum0 = sum0w = sum1 = sum1w = 0.;
			col0 = col1 = make_grey_f(0.);

			if (sd > 1e-3)
			{
				for (ib.y=0; ib.y < cp.block_size; ib.y++)
					for (ib.x=0; ib.x < cp.block_size; ib.x++)
					{
						i = ib.y*cp.block_size + ib.x;

						if (isnan(b0[i].a)==0)
						{
							if (b0[i].a < mean_lum)
							{
								erfw = sq(erf((mean_lum-b0[i].a)/sd));
								bary0 = add_xy(bary0, mul_xy(xyi_to_xy(ib), set_xy(block_coef[i] * erfw)));
								bary0w += block_coef[i] * erfw;
								sum0 += b0[i].a * block_coef[i] * sq(erfw);
								sum0w += block_coef[i] * sq(erfw);
								col0 = add_frgb(col0, mul_scalar_frgb(b0[i], block_coef[i] * sq(erfw)));
							}
							else
							{
								erfw = sq(erf((b0[i].a-mean_lum)/sd));
								bary1 = add_xy(bary1, mul_xy(xyi_to_xy(ib), set_xy(block_coef[i] * erfw)));
								bary1w += block_coef[i] * erfw;
								sum1 += b0[i].a * block_coef[i] * sq(erfw);
								sum1w += block_coef[i] * sq(erfw);
								col1 = add_frgb(col1, mul_scalar_frgb(b0[i], block_coef[i] * sq(erfw)));
							}
						}
					}
				bary0 = div_xy(bary0, set_xy(bary0w));
				bary1 = div_xy(bary1, set_xy(bary1w));
				mean_lum0 = sum0 / sum0w;
				mean_lum1 = sum1 / sum1w;
				col0 = mul_scalar_frgb(col0, 1./sum0w);
				col1 = mul_scalar_frgb(col1, 1./sum1w);

				c[4] = atan2(bary1.x-bary0.x, bary1.y-bary0.y) / (2.*pi);
				//fit_erf_block_bruteforce(cp, block_coef, b0, mean_lum0, mean_lum1, c);
				c[0] = 1.25;
				c[1] = 0.;
			}
			else	// if all pixels are the same
			{
				mean_lum0 = mean_lum1 = mean_lum;
				col0 = col1 = mean_col;
				bary0 = bary1 = cp.block_centre;
			}
			}
			else
				bary0 = bary1 = XY0;

			// Prepare the arguments
			if (equal_xy(bary0, bary1))
			{
				erf_th = 0.;
				erf_off = 0.;
				erf_width = 1.25;
			}
			else
			{
				erf_th = rangewrap(c[4], 0., 1.);	// angle in turns
				erf_off = c[1];
				erf_width = c[0];
			}

			// Find col0 and col1 in a different way
			if (cp.bits_per_pixel)
			{
				int nan_count;
				// Watch out for NANs at the beginning of bs
				for (nan_count=0; nan_count < cp.block_size*cp.block_size; nan_count++)
					if (isnan(bs[nan_count].a)==0)
						break;

				memset(&col0, 0, sizeof(frgb_t));
				memset(&col1, 0, sizeof(frgb_t));
				memset(&colmid, 0, sizeof(frgb_t));
				sumw = 0.;

				int sec_count = cp.block_size*cp.block_size >> (cp.bits_per_pixel+1);
				for (i=0; i < sec_count; i++)
				{
					if (nan_count+i < cp.block_size*cp.block_size)
						col0 = add_frgba(col0, bs[nan_count+i]);
					if (cp.block_size*cp.block_size-1-i >= nan_count)
						col1 = add_frgba(col1, bs[cp.block_size*cp.block_size-1-i]);
				}
				col0 = mul_scalar_frgba(col0, 1./(double) sec_count);
				col1 = mul_scalar_frgba(col1, 1./(double) sec_count);

				// Find middle colour (between the luminosities of col0 and col1)
				double mid, width, weight;
				mid = 0.5*(col0.a + col1.a);
				if (col1.a > col0.a)
					width = 8. / (col1.a-col0.a);
				else
					width = 0.;

				for (i=nan_count; i < cp.block_size*cp.block_size; i++)
				{
					weight = (bs[i].a - mid) * width;
					if (fabs(weight) <= 2.)
					{
						weight = fastgaussianf_d0(weight);
						colmid = add_frgba(colmid, mul_scalar_frgba(bs[i], weight));
						sumw += weight;
					}
				}
				colmid = mul_scalar_frgba(colmid, 1./sumw);
			}

			hsl0 = compr_frgb_to_hsl(Lab_L_to_linear_f(col0));
			hsl1 = compr_frgb_to_hsl(Lab_L_to_linear_f(col1));
			hslm = compr_frgb_to_hsl(Lab_L_to_linear_f(colmid));

			// Add compressed block to buffer
			int h0, s0, l0, h1, s1, l1, hm, sm, lm, i_erf_th, i_erf_off, i_erf_width, pix;

			make_compr_hsl(cp, hsl0, &h0, &s0, &l0);
			make_compr_hsl(cp, hsl1, &h1, &s1, &l1);
			make_compr_hsl(cp, hslm, &hm, &sm, &lm);
			i_erf_th = erf_th * bits_to_mul(cp.bits_erf_th) + 0.5;
			i_erf_off = nearbyint(erf_off * 8.);
			i_erf_width = erf_width/1.25 * bits_to_mul(cp.bits_erf_width) + 0.5;

			//buf_alloc_enough(&buf, ceil_rshift(di + cp.bits_per_block, 3));
			alloc_enough(&buf.buf, ceil_rshift(di + cp.bits_per_block, 3), &buf.as, 1, 1.5);

			set_bits_in_stream_inc(buf.buf, &di, cp.bits_ch, h0);
			set_bits_in_stream_inc(buf.buf, &di, cp.bits_cs, s0);
			set_bits_in_stream_inc(buf.buf, &di, cp.bits_cl, l0);
			set_bits_in_stream_inc(buf.buf, &di, cp.bits_ch, h1);
			set_bits_in_stream_inc(buf.buf, &di, cp.bits_cs, s1);
			set_bits_in_stream_inc(buf.buf, &di, cp.bits_cl, l1);
			set_bits_in_stream_inc(buf.buf, &di, cp.bits_ch, hm);
			set_bits_in_stream_inc(buf.buf, &di, cp.bits_cs, sm);

			set_bits_in_stream_inc(buf.buf, &di, cp.bits_erf_th, i_erf_th);
			set_bits_in_stream_inc(buf.buf, &di, cp.bits_erf_off, i_erf_off);
			set_bits_in_stream_inc(buf.buf, &di, cp.bits_erf_width, i_erf_width);

			if (cp.bits_per_pixel)		// bitmap
			{
				double pix_mul = bits_to_mul(cp.bits_per_pixel);
				double dmul = 1. / (col1.a-col0.a);

				for (ib.y=0; ib.y < cp.block_size; ib.y++)
					for (ib.x=0; ib.x < cp.block_size; ib.x++)
					{
						if (ip.y+ib.y >=0 && ip.y+ib.y < r1.dim.y && ip.x+ib.x >=0 && ip.x+ib.x < r1.dim.x)
						{
							i = ib.y*cp.block_size + ib.x;
							pix = rangelimit(nearbyint((b0[i].a - col0.a) * dmul * pix_mul), 0., pix_mul);
						}
						else
							pix = 0;

						set_bits_in_stream_inc(buf.buf, &di, cp.bits_per_pixel, pix);
					}
			}
		}
	}
	buf.len = ceil_rshift(di, 3);
	//fprintf_rl(stdout, "Compressed buffer: %.1f kB (%.1f bits/texel)\n", (double) buf.len/1024., (double) buf.len*8. / mul_x_by_y_xyi(r1.dim));
	/*buffer_t gz_buf = gz_compress_to_buffer(buf.buf, buf.len, 10);
	fprintf_rl(stdout, "GZip compression: %.1f kB -> %.1f kB (%.1f%% compression)\n", (double) buf.len/1024., (double) gz_buf.len/1024., 100. * (double) gz_buf.len / (double) buf.len);
	free_buf(&gz_buf);*/

	free(b0);
	free(bs);
	free(block_coef);

	buf.buf = realloc(buf.buf, buf.len);
	r1.buf = buf.buf;
	r1.as = buf.len;
	r1.buf_size = buf.len;
	return r1;
}

raster_t compressed_texture_to_frgb(raster_t r0)
{
	double erf_th, erf_off, erf_width;
	raster_t r1={0};
	xyi_t ip, ib;
	int i, yb;
	compression_param1_t cp={0};
	frgb_t pv, col0, col1, colm;
	float *block_coef;
	xyz_t hsl0, hsl1, hslm;
	uint8_t *bd;
	size_t di=0;

	bd = r0.buf;
	r1 = make_raster(NULL, r0.dim, XYI0, IMAGE_USE_FRGB);

	// Load decoding parameters
	cp.block_size = read_LE16(&bd[di], &di);
	read_LE16(&bd[di], &di);	// bits_per_block
	cp.window_base_count = read_byte8(&bd[di], &di);
	cp.window_base = calloc(cp.window_base_count, sizeof(float));
	for (i=0; i < cp.window_base_count; i++)
		cp.window_base[i] = u32_as_float(read_LE32(&bd[di], &di));
	cp.quincunx = read_byte8(&bd[di], &di);
	cp.bits_ch = read_byte8(&bd[di], &di);
	cp.bits_cs = read_byte8(&bd[di], &di);
	cp.bits_cl = read_byte8(&bd[di], &di);
	cp.bits_per_pixel = read_byte8(&bd[di], &di);
	if (cp.bits_per_pixel==0)
	{
		cp.bits_erf_th = read_byte8(&bd[di], &di);
		cp.bits_erf_off = read_byte8(&bd[di], &di);
		cp.bits_erf_width = read_byte8(&bd[di], &di);
	}
	di *= 8;
	
	calc_compression_param1(&cp);
	block_coef = make_windowing_block(cp);
	free(cp.window_base);
	int *hs_histo = calloc(1<<cp.bits_ch+cp.bits_cs, sizeof(int));

	float pix_mul = 2.f / bits_to_mul(cp.bits_per_pixel);

	// Go through blocks in quincunx formation
	for (yb=0, ip.y = -cp.window_base_count; ip.y < r1.dim.y; ip.y+=cp.block_step, yb++)
	{
		for (ip.x = -cp.window_base_count + cp.odd_offset*(yb & 1); ip.x < r1.dim.x; ip.x+=cp.block_step)
		{
			// Read block data
			int h0, s0, l0, h1, s1, l1, hm, sm, i_erf_th, i_erf_off, i_erf_width, pix;

			h0 = get_bits_in_stream_inc(bd, &di, cp.bits_ch);
			s0 = get_bits_in_stream_inc(bd, &di, cp.bits_cs);
			l0 = get_bits_in_stream_inc(bd, &di, cp.bits_cl);
			h1 = get_bits_in_stream_inc(bd, &di, cp.bits_ch);
			s1 = get_bits_in_stream_inc(bd, &di, cp.bits_cs);
			l1 = get_bits_in_stream_inc(bd, &di, cp.bits_cl);
			hm = get_bits_in_stream_inc(bd, &di, cp.bits_ch);
			sm = get_bits_in_stream_inc(bd, &di, cp.bits_cs);

			i_erf_th = get_bits_in_stream_inc(bd, &di, cp.bits_erf_th);
			i_erf_off = get_bits_in_stream_inc(bd, &di, cp.bits_erf_off);
			i_erf_width = get_bits_in_stream_inc(bd, &di, cp.bits_erf_width);

			hsl0 = compr_hsl_to_xyz(cp, h0, s0, l0);
			hsl1 = compr_hsl_to_xyz(cp, h1, s1, l1);
			hslm = compr_hsl_to_xyz(cp, hm, sm, l0+l1);
			hslm.z *= 0.5;
			col0 = compr_hsl_to_frgb(hsl0);
			col1 = compr_hsl_to_frgb(hsl1);
			colm = compr_hsl_to_frgb(hslm);

			if (cp.bits_per_pixel)		// if we're using bitmaps
			{
				for (ib.y=0; ib.y < cp.block_size; ib.y++)
					for (ib.x=0; ib.x < cp.block_size; ib.x++)
					{
						pix = get_bits_in_stream_inc(bd, &di, cp.bits_per_pixel);

						if (ip.y+ib.y >=0 && ip.y+ib.y < r1.dim.y && ip.x+ib.x >=0 && ip.x+ib.x < r1.dim.x)
						{
							i = ib.y*cp.block_size + ib.x;

							// Interpolate between colours
							float t = (float) pix * pix_mul;
							if (t < 1.f)
								pv = add_frgb(col0, mul_scalar_frgb(sub_frgb(colm, col0), t));
							else
								pv = add_frgb(colm, mul_scalar_frgb(sub_frgb(col1, colm), t-1.f));

							pv = mul_scalar_frgb(Lab_L_to_linear_f(pv), block_coef[i]);

							// Write to frgb buffer
							r1.f[(ip.y+ib.y)*r1.dim.x + (ip.x+ib.x)] = add_frgb(r1.f[(ip.y+ib.y)*r1.dim.x + (ip.x+ib.x)], pv);
						}
					}
			}
			else				// if we're using erf
			{
				erf_th = (double) i_erf_th / bits_to_mul(cp.bits_erf_th);
				erf_off = (double) i_erf_off / 8.;
				erf_width = (double) i_erf_width * 1.25 / bits_to_mul(cp.bits_erf_width);

				hs_histo[(h0<<cp.bits_cs) + s0]++;
				hs_histo[(h1<<cp.bits_cs) + s1]++;

				double costh = fastcos_tr_d2(erf_th);
				double sinth = fastsin_tr_d2(erf_th);
				xy_t rp;

				// Make block
				for (ib.y=0; ib.y < cp.block_size; ib.y++)
				{
					rp.y = ((double) ib.y-cp.block_centre.y) * costh;
					for (ib.x=0; ib.x < cp.block_size; ib.x++)
					{
						rp.x = ((double) ib.x-cp.block_centre.x) * sinth;
						i = ib.y*cp.block_size + ib.x;
						//t = rotate_xy2(sub_xy(xyi_to_xy(ib), cp.block_centre), erf_th).y;
						//pv = add_frgb(col0, mul_scalar_frgb(sub_frgb(col1, col0), fasterfrf_d1((t-erf_off*gv[0])*1.25)));
						pv = add_frgb(col0, mul_scalar_frgb(sub_frgb(col1, col0), fasterfrf_d1((rp.x + rp.y + erf_off)*erf_width)));
						pv = mul_scalar_frgb(Lab_L_to_linear_f(pv), block_coef[i]);

						// Write to frgb buffer
						if (ip.y+ib.y >=0 && ip.y+ib.y < r1.dim.y && ip.x+ib.x >=0 && ip.x+ib.x < r1.dim.x)
							r1.f[(ip.y+ib.y)*r1.dim.x + (ip.x+ib.x)] = add_frgb(r1.f[(ip.y+ib.y)*r1.dim.x + (ip.x+ib.x)], pv);
					}
				}
			}
		}
	}

	#if 0
	// Histogram saving
	char *path = save_file_dialog("Text file\1*.txt\1");
	if (path)
	{
		int set_count = 0;
		FILE *file = fopen_utf8(path, "wb");

		for (ip.y=0; ip.y < 1<<cp.bits_ch; ip.y++)
			for (ip.x=0; ip.x < 1<<cp.bits_cs; ip.x++)
			{
				if (hs_histo[(ip.y<<cp.bits_cs) + ip.x]==0)
					fprintf(file, "    -");
				else
				{
					fprintf_rl(file, "%5d", hs_histo[(ip.y<<cp.bits_cs) + ip.x]);
					set_count++;
				}

				if (ip.x < (1<<cp.bits_cs)-1)
					fprintf(file, "  ");
				else
					fprintf(file, "\n");
			}

		fprintf(file, "HS gamut used: %.2f%% (%d HS values)\n", 100. * (double) set_count / (double) (1<<cp.bits_ch+cp.bits_cs), set_count);
		fclose(file);
		system_open(path);
	}
	#endif
	free(hs_histo);

	free(block_coef);

	return r1;
}

mipmap_t raster_to_mipmap_compr_then_free(raster_t *r, const int mode, xyi_t thresh_dim)
{
	int il;
	mipmap_t m0, m1={0};
	mipmap_level_t *ml0, *ml1;
	xyi_t it;

	// Mipmap the uncompressed raster
	m0 = raster_to_tiled_mipmaps_fast_defaults(*r, mode);
	free_raster(r);

	// Allocate compressed mipmap
	m1 = m0;
	m1.lvl = calloc(m1.lvl_count, sizeof(mipmap_level_t));
	m1.total_bytes = m1.lvl_count * sizeof(mipmap_level_t);

	if (equal_xyi(thresh_dim, XYI0))
		thresh_dim = set_xyi(0x7FFFFFFFL);

	// Convert each level up to the threshold dimension
	for (il=m1.lvl_count-1; il >= 0; il--)
	{
		ml0 = &m0.lvl[il];
		ml1 = &m1.lvl[il];

		if (ml0->fulldim.x > thresh_dim.x || ml0->fulldim.y > thresh_dim.y)
		{
			free_mipmap_level(ml0);
		}
		else
		{
			*ml1 = *ml0;
			ml1->r = calloc(mul_x_by_y_xyi(ml1->tilecount), sizeof(raster_t));
			ml1->total_bytes = mul_x_by_y_xyi(ml1->tilecount) * sizeof(raster_t);

			// Convert each tile
			for (it.y=0; it.y < ml1->tilecount.y; it.y++)
				for (it.x=0; it.x < ml1->tilecount.x; it.x++)
				{
					ml1->r[it.y*ml1->tilecount.x + it.x] = frgb_to_compressed_texture(ml0->r[it.y*ml0->tilecount.x + it.x], NULL);
					ml1->total_bytes += ml1->r[it.y*ml1->tilecount.x + it.x].buf_size;
					free_raster(&ml0->r[it.y*ml0->tilecount.x + it.x]);
				}

			m1.total_bytes += ml1->total_bytes;
		}
		free_null(&ml0->r);
	}
	free_null(&m0.lvl);

	return m1;
}
