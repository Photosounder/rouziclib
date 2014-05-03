lut_t get_lut_lsrgb()
{
	int32_t i;
	double linear, s;
	static int init=1;
	static lut_t lsrgb_l;

	if (init)
	{
		init = 0;

		lsrgb_l.lut_size = ONE+1;
	
		lsrgb_l.lutint = calloc (lsrgb_l.lut_size, sizeof(int32_t));
	
		for (i=0; i<lsrgb_l.lut_size; i++)
		{
			linear = (double) i / ONEF;
	
			if (linear <= 0.0031308)
				s = linear * 12.92;
			else
				s = 1.055 * pow(linear, 1.0/2.4) - 0.055;
			
			lsrgb_l.lutint[i] = s*8160. + 0.5;	// 8160 = 255 * 32 (8.5 fixed point format)
		}
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
	
		for (i=0; i<slrgb_l.lut_size; i++)
		{
			if (i <= 10)
				slrgb_l.lutint[i] = ONEF * ((double) i / 255.) / 12.92 + 0.5;
			else
				slrgb_l.lutint[i] = ONEF * pow((((double) i / 255.) + 0.055) / 1.055, 2.4) + 0.5;
		}
	}

	return slrgb_l;
}

lut_t dither_lut_init()
{
	int32_t i;
	lut_t dither_l;

	dither_l.lut_size = 16384;

	dither_l.lutint = calloc (dither_l.lut_size, sizeof(int32_t));

	for (i=0; i<dither_l.lut_size; i++)
	{		
		// The ratio is 16. instead of 32. because only 0.5x the noise intensity is needed (even less, around 0.45x or even as low as 0.4x, might be tolerable)
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

void convert_lrgb_to_srgb(srgb_t *sfb, lrgb_t *fb, int32_t pixc, int mode)
{
	int32_t i, id, stop, dither;
	static int init=1;
	static lut_t lsrgb_l, dither_l, bytecheck_l;

	if (init)
	{
		init = 0;

		lsrgb_l = get_lut_lsrgb();
		dither_l = dither_lut_init();
		bytecheck_l = bytecheck_lut_init(4);
	}

	id = rand() & 0x3FFF;
	stop = (id + 100 + (rand() & 0x03FF)) & 0x3FFF;

	if (mode==DITHER)
	{
		for (i=0; i<pixc; i++)
		{
			dither = dither_l.lutint[id];
			sfb[i].r = bytecheck_l.lutb[lsrgb_l.lutint[fb[i].r] + dither >> 5];		// 8.5 + 2.5 >> 5 = 8.0 sRGB
			sfb[i].g = bytecheck_l.lutb[lsrgb_l.lutint[fb[i].g] + dither >> 5];
			sfb[i].b = bytecheck_l.lutb[lsrgb_l.lutint[fb[i].b] + dither >> 5];
	
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
			sfb[i].r = lsrgb_l.lutint[fb[i].r] >> 5;
			sfb[i].g = lsrgb_l.lutint[fb[i].g] >> 5;
			sfb[i].b = lsrgb_l.lutint[fb[i].b] >> 5;
		}
	}
}

void convert_srgb_to_lrgb(srgb_t *sfb, lrgb_t *fb, int32_t pixc)
{
	int32_t i;
	static int init=1;
	static lut_t slrgb_l;

	if (init)
	{
		init = 0;

		slrgb_l = get_lut_slrgb();
	}


	for (i=0; i<pixc; i++)
	{
		fb[i].r = slrgb_l.lutint[sfb[i].r];
		fb[i].g = slrgb_l.lutint[sfb[i].g];
		fb[i].b = slrgb_l.lutint[sfb[i].b];
	}
}
