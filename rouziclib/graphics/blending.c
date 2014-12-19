// Blends a whole foreground pixel with a background pixel by a ratio of p (1.15 fixed-point format)
lrgb_t blend_pixels(lrgb_t bg, lrgb_t fg, int32_t p, const int mode)
{
	int32_t pinv, r, g, b, a, fba;

	switch (mode)
	{
		case SOLID:
			bg = fg;
			break;

		case ADD:	// add
			r = (fg.r * p >> 15) + bg.r;	if (r>ONE) r = ONE;
			g = (fg.g * p >> 15) + bg.g;	if (g>ONE) g = ONE;
			b = (fg.b * p >> 15) + bg.b;	if (b>ONE) b = ONE;
			
			bg.r = r;
			bg.g = g;
			bg.b = b;
			break;

		case SUB:	// sub	
			r = -(fg.r * p >> 15) + bg.r;	if (r<0) r = 0;
			g = -(fg.g * p >> 15) + bg.g;	if (g<0) g = 0;
			b = -(fg.b * p >> 15) + bg.b;	if (b<0) b = 0;

			bg.r = r;
			bg.g = g;
			bg.b = b;
			break;

		case MUL:	// multiply
			pinv = 32768 - p;
			bg.r = (bg.r*fg.r >> LBD) * p + bg.r * pinv >> 15;
			bg.g = (bg.g*fg.g >> LBD) * p + bg.g * pinv >> 15;
			bg.b = (bg.b*fg.b >> LBD) * p + bg.b * pinv >> 15;
			break;

		case MUL4:	// multiply by a quarter intensity image (1.0 == sRGB value of 137)
			pinv = 32768 - p;
			r = (bg.r*fg.r >> (LBD-2)) * p + bg.r * pinv >> 15;	if (r>ONE) r = ONE;
			g = (bg.g*fg.g >> (LBD-2)) * p + bg.g * pinv >> 15;	if (g>ONE) g = ONE;
			b = (bg.b*fg.b >> (LBD-2)) * p + bg.b * pinv >> 15;	if (b>ONE) b = ONE;
			
			bg.r = r;
			bg.g = g;
			bg.b = b;
			break;

		case BLEND:	// blend
			pinv = 32768 - p;

			bg.r = fg.r * p + bg.r * pinv >> 15;
			bg.g = fg.g * p + bg.g * pinv >> 15;
			bg.b = fg.b * p + bg.b * pinv >> 15;
			break;

		case ALPHABLEND:		// proper alpha blending when both pixels might have any possible alpha value
			pinv = 32768 - p;

			#if LBD == 15
			a = bg.a;
			#else
			a = 32768 * bg.a >> LBD;		// 1.15
			#endif

			bg.a = ONE * p + bg.a * pinv >> 15;	// FIXME fg.a is assumed to be 1.0
			if (bg.a > 0)
			{
				#if LBD == 15
				fba = bg.a;
				#else
				fba = 32768 * bg.a >> LBD;
				#endif

				r = (fg.r*p + (bg.r * a >> 15)*pinv) / fba;
				g = (fg.g*p + (bg.g * a >> 15)*pinv) / fba;
				b = (fg.b*p + (bg.b * a >> 15)*pinv) / fba;

				if (r>ONE) r = ONE;	bg.r = r;
				if (g>ONE) g = ONE;	bg.g = g;
				if (b>ONE) b = ONE;	bg.b = b;
			}
			else
			{
				bg.r = 0;
				bg.g = 0;
				bg.b = 0;
			}
			break;

		case ALPHABLENDFG:		// alpha blending (doesn't take framebuffer's alpha into account though, assumed to be 1.0)
			p = p * fg.a >> LBD;
			pinv = 32768 - p;

			bg.r = fg.r * p + bg.r * pinv >> 15;
			bg.g = fg.g * p + bg.g * pinv >> 15;
			bg.b = fg.b * p + bg.b * pinv >> 15;
			break;

		case BLENDALPHAONLY:
			pinv = 32768 - p;
			bg.a = fg.a * p + bg.a * pinv >> 15;
			break;

		default:
			bg.g = rand() % (ONE-1);
	}

	return bg;
}
