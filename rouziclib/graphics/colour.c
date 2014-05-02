lrgb_t make_colour_lin(double r, double g, double b, double a)
{
	lrgb_t c;

	c.r = r * ONEF + 0.5;
	c.g = g * ONEF + 0.5;
	c.b = b * ONEF + 0.5;
	c.a = a * ONEF + 0.5;

	return c;
}

lrgb_t make_colour_srgb(int r, int g, int b, int a)
{
	lrgb_t c;
	static int init=1;
	static lut_t slrgb_l;

	if (init)
	{
		init = 0;

		slrgb_l = get_lut_slrgb();
	}

	c.r = slrgb_l.lutint[r];
	c.g = slrgb_l.lutint[g];
	c.b = slrgb_l.lutint[b];
	c.a = slrgb_l.lutint[a];

	return c;
}

double Lab_L_to_linear(double t)
{
	const double stn=6./29;

	t = (t+0.16) / 1.16;

	if (t > stn)
		return t*t*t;
	else
		return 3.*stn*stn*(t - 4./29.);
}

void hsl_to_rgb(double H, double S, double L, double *r, double *g, double *b, int huemode, int secboost)
{
	const double Wr=WEIGHT_R, Wg=WEIGHT_G, Wb=WEIGHT_B;        // these are the weights for each colour
	double red, grn, blu, t, Y;

	// hue
	if (huemode==HUERAD)
		H = 3.*H / (2.*pi);	// full circle is 3.0
	if (huemode==HUEDEG)
		H = 3.*H / 360.;	// full circle is 3.0
	t = fabs(rangelimit(rangewrap(H-0., -1., 2.), -1., 1.));	red = t <= 0.5 ? 1.0 : Lab_L_to_linear(2.*(1.-t));
	t = fabs(rangelimit(rangewrap(H-1., -1., 2.), -1., 1.));	grn = t <= 0.5 ? 1.0 : Lab_L_to_linear(2.*(1.-t));
	t = fabs(rangelimit(rangewrap(H-2., -1., 2.), -1., 1.));	blu = t <= 0.5 ? 1.0 : Lab_L_to_linear(2.*(1.-t));

	// luminosity
	Y = Wr*red + Wg*grn + Wb*blu;
	Y = L/Y;	// divide by temporary lightness, multiply by desired lightness
	if (secboost)
	{
		t = 2.*fabs(rangewrap(H, 0., 1.) - 0.5);	// tc is 0 on a secondary, 1 on a primary
		Y *= (0.5*cos(pi*t)+0.5) + 1.;			// doubles lightness for secondaries
	}
	red *= Y;
	grn *= Y;
	blu *= Y;

	// saturation
	red = red*S + L*(1.-S);
	grn = grn*S + L*(1.-S);
	blu = blu*S + L*(1.-S);

	*r = red;
	*g = grn;
	*b = blu;
}

void colour_blowout_double(double *pred, double *pgrn, double *pblu)
{
	const double Wr=WEIGHT_R, Wg=WEIGHT_G, Wb=WEIGHT_B;         // these are the weights for each colour
	double max, red, grn, blu, t, L;
	
	max = MAXN(MAXN(*pred, *pgrn), *pblu);    // max is the maximum value of the 3 colours
	
	if (max > 1.)       // if the colour is out of gamut
	{
		red = *pred;	grn = *pgrn;	blu = *pblu; 

		L = Wr*red + Wg*grn + Wb*blu;   // Luminosity of the colour's grey point
		
		if (L < 1.) // if the grey point is no brighter than white
		{
			// t represents the ratio on the line between the input colour
			// and its corresponding grey point. t is between 0 and 1,
			// a lower t meaning closer to the grey point and a
			// higher t meaning closer to the input colour
			t = (1.-L) / (max-L);
		
			// a simple linear interpolation between the
			// input colour and its grey point
			*pred = red*t + L*(1.-t);
			*pgrn = grn*t + L*(1.-t);
			*pblu = blu*t + L*(1.-t);
		}
		else    // if it's too bright regardless of saturation
		{
			*pred = *pgrn = *pblu = 1.;
		}
	}	
}

void colour_blowout_int(uint32_t *pred, uint32_t *pgrn, uint32_t *pblu)
{
	const uint32_t Wr=WEIGHT_R*32768.+0.5, Wg=WEIGHT_G*32768.+0.5, Wb=WEIGHT_B*32768.+0.5;        // these are the weights for each colour
	uint32_t max, red, grn, blu, t, L;

	max = MAXN(MAXN(*pred, *pgrn), *pblu);    // max is the maximum value of the 3 colours
	
	if (max > ONE)       // if the colour is out of gamut
	{
		red = *pred;	grn = *pgrn;	blu = *pblu;

		// turn max from 1.LBD format to 1.15 format
		#if (LBD < 15)
			max <<= 15-LBD;
		#endif
		#if (LBD > 15)
			max >>= LBD-15;
		#endif

		L = (Wr*red + Wg*grn + Wb*blu) >> LBD;   // Luminosity of the colour's grey point, 0.15

		if (L < 32768) // if the grey point is no brighter than white
		{
			// t represents the ratio on the line between the input colour
			// and its corresponding grey point. t is between 0 and 1,
			// a lower t meaning closer to the grey point and a
			// higher t meaning closer to the input colour
			t = ((32768-L)<<15) / (max-L);

			L = L*(32768-t) >> 15;
			// turn L from 0.15 format to 0.LBD format
			#if (LBD < 15)
				L >>= 15-LBD;
			#endif
			#if (LBD > 15)
				L <<= LBD-15;
			#endif

			// a simple linear interpolation between the
			// input colour and its grey point
			*pred = (red*t>>15) + L;
			*pgrn = (grn*t>>15) + L;
			*pblu = (blu*t>>15) + L;

			*pred = MINN(*pred, ONE);	// due to all the bit shifting, 1.0 becomes ONE+1 (e.g. 4096 instead of 4095)
			*pgrn = MINN(*pgrn, ONE);
			*pblu = MINN(*pblu, ONE);
		}
		else    // if it's too bright regardless of saturation
		{
			*pred = *pgrn = *pblu = ONE;
		}
	}
}
