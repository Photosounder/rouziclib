void gaussian_blur(float *a, float *b, int w, int h, double radius)
{
	int i, j, ic, ix, iy, jx, jy, d, in_place=0;
	double *gk;
	int gks, start, stop;
	float *column;

	if (a==b)
	{
		in_place = 1;
		b = calloc(w*h*4, sizeof(float));
	}
	else
		memset(b, 0, w*h*4*sizeof(float));

	gks = floor(GAUSSRAD_HQ * radius) + 1.;
	gk = calloc (gks, sizeof(double));
	
	for (i=0; i<gks; i++)
	{
		gk[i] = gaussian((double) i / radius) / radius / sqrt(pi);
		//gk[i] = sinc((double) i, 0.6/4.) * blackman((double) i, gks);
		fprintf_rl(stdout, "gk[%d] = %g\n", i, gk[i]);
	}
	
	for (iy=0; iy<h; iy++)		// horizontal blurring
	{
		for (ix=0; ix<w; ix++)
		{
			start = MAXN (0, ix-(gks-1));
			stop = MINN (w - 1, ix+(gks-1));

			for (jx=start; jx<=stop; jx++)
			{
				i = (iy*w + ix) << 2;
				j = (iy*w + jx) << 2;
				d = abs(jx-ix);

				for (ic=0; ic<4; ic++)
					b[i + ic] += gk[d] * a[j + ic];
			}
		}
	}

	column = calloc (h*4, sizeof(float));

	for (ix=0; ix<w; ix++)
	{
		for (iy=0; iy<h; iy++)
		{
			((frgb_t *)column)[iy] = ((frgb_t *)b)[iy*w + ix];
			memset(&((frgb_t *)b)[iy*w + ix], 0, sizeof(frgb_t));
		}

		for (iy=0; iy<h; iy++)		// vertical blurring
		{
			start = MAXN (0, iy-(gks-1));
			stop = MINN (h - 1, iy+(gks-1));
			i = (iy*w + ix) << 2;

			for (jy=start; jy<=stop; jy++)
				for (ic=0; ic<4; ic++)
					b[i + ic] += gk[abs(jy-iy)] * column[(jy<<2)+ic];
		}
	}

	free (column);
	free (gk);

	if (in_place)
	{
		memcpy(a, b, w*h*4*sizeof(float));
		free (b);
	}
}
