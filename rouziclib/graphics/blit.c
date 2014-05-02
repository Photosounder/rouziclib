void blit_sprite(lrgb_t *fb, int32_t fbw, int32_t fbh, lrgb_t *sprite, int32_t spw, int32_t sph, int32_t pos_x, int32_t pos_y, int blendingmode, int hmode, int vmode)
{
	int32_t iy_fbw, iy_spw;
	int32_t ix, iy, offset_x, offset_y, start_x, start_y, stop_x, stop_y;

	if (hmode==A_CEN)	pos_x -= (spw>>1);
	if (hmode==A_RIG)	pos_x -= (spw-1);
	if (vmode==A_CEN)	pos_y -= (sph>>1);
	if (vmode==A_BOT)	pos_y -= (sph-1);

	offset_x = pos_x;
	offset_y = pos_y;
	if (pos_x < 0)		start_x = -pos_x;	else	start_x = 0;
	if (pos_y < 0)		start_y = -pos_y;	else	start_y = 0;
	if (pos_x+spw >= fbw)	stop_x = fbw - pos_x;	else	stop_x = spw;
	if (pos_y+sph >= fbh)	stop_y = fbh - pos_y;	else	stop_y = sph;

	if (stop_x <= start_x || stop_y <= start_y)
		return ;

	if (blendingmode==SOLID)	// if the sprite is opaque then it can be blitted whole lines at a time
	{
		for (iy=start_y; iy<stop_y; iy++)
		{
			iy_fbw = (iy + offset_y) * fbw + offset_x;
			iy_spw = iy * spw;
			memcpy(&fb[iy_fbw], &sprite[iy_spw], (stop_x-start_x) * sizeof(lrgb_t));
		}
		return;
	}

	for (iy=start_y; iy<stop_y; iy++)
	{
		iy_fbw = (iy + offset_y) * fbw + offset_x;
		iy_spw = iy * spw;

		for (ix=start_x; ix<stop_x; ix++)
		{
			fb[iy_fbw+ix] = blend_pixels(fb[iy_fbw+ix], sprite[iy_spw+ix], 32768, blendingmode);
		}
	}

	/*if ((mode&1)==NOALPHA && (mode&(ADD|MULTIPLY|MULT128))==0 && stop_x>start_x)
		for (iy=start_y; iy<stop_y; iy++)
		{
			iy_fbw = (iy + offset_y) * fbw;
			iy_spw = iy * spw;
			memcpy(&fb[iy_fbw + offset_x], &sprite[iy_spw], (stop_x-start_x) * sizeof(uint32_t));
		}

	if ((mode&ALPHA)==ALPHA && (mode&(ADD|MULTIPLY|MULT128))==0 && stop_x>start_x)
		for (iy=start_y; iy<stop_y; iy++)
		{
			iy_fbw = (iy + offset_y) * fbw;
			iy_spw = iy * spw;
			
			for (ix=start_x; ix<stop_x; ix++)
			{
				switch (sprite[iy_spw+ix].a)
				{
					case ONE:
						fb[iy_fbw + ix + offset_x] = sprite[iy_spw+ix];
						break;
					case 0:
						break;
					default:
						a = sprite[iy_spw+ix].a;
						ainv = ONE-a;
			fb[iy_fbw + ix + offset_x].r = (fb[iy_fbw + ix + offset_x].r*ainv + sprite[iy_spw+ix].r*a) >> LBD;
			fb[iy_fbw + ix + offset_x].g = (fb[iy_fbw + ix + offset_x].g*ainv + sprite[iy_spw+ix].g*a) >> LBD;
			fb[iy_fbw + ix + offset_x].b = (fb[iy_fbw + ix + offset_x].b*ainv + sprite[iy_spw+ix].b*a) >> LBD;
						break;
				}
			}
		}

	if ((mode&ADD)==ADD && stop_x>start_x)
		for (iy=start_y; iy<stop_y; iy++)
		{
			iy_fbw = (iy + offset_y) * fbw;
			iy_spw = iy * spw;
			for (ix=start_x; ix<stop_x; ix++)
			{
				r = fb[iy_fbw + ix + offset_x].r + sprite[iy_spw+ix].r;
				g = fb[iy_fbw + ix + offset_x].g + sprite[iy_spw+ix].g;
				b = fb[iy_fbw + ix + offset_x].b + sprite[iy_spw+ix].b;
				fb[iy_fbw + ix + offset_x].r = r>ONE ? ONE : r;
				fb[iy_fbw + ix + offset_x].g = g>ONE ? ONE : g;
				fb[iy_fbw + ix + offset_x].b = b>ONE ? ONE : b;
			}
		}

	if ((mode&MULTIPLY)==MULTIPLY && stop_x>start_x)
		for (iy=start_y; iy<stop_y; iy++)
		{
			iy_fbw = (iy + offset_y) * fbw;
			iy_spw = iy * spw;
			for (ix=start_x; ix<stop_x; ix++)
			{
				fb[iy_fbw + ix + offset_x].r = (fb[iy_fbw + ix + offset_x].r * sprite[iy_spw+ix].r) >> LBD;
				fb[iy_fbw + ix + offset_x].g = (fb[iy_fbw + ix + offset_x].g * sprite[iy_spw+ix].g) >> LBD;
				fb[iy_fbw + ix + offset_x].b = (fb[iy_fbw + ix + offset_x].b * sprite[iy_spw+ix].b) >> LBD;
			}
		}

	if ((mode&MULT128)==MULT128 && stop_x>start_x)
		for (iy=start_y; iy<stop_y; iy++)
		{
			iy_fbw = (iy + offset_y) * fbw;
			iy_spw = iy * spw;
			for (ix=start_x; ix<stop_x; ix++)
			{
				r = (fb[iy_fbw + ix + offset_x].r * sprite[iy_spw+ix].r) >> (LBD-1);
				g = (fb[iy_fbw + ix + offset_x].g * sprite[iy_spw+ix].g) >> (LBD-1);
				b = (fb[iy_fbw + ix + offset_x].b * sprite[iy_spw+ix].b) >> (LBD-1);
				fb[iy_fbw + ix + offset_x].r = r>ONE ? ONE : r;
				fb[iy_fbw + ix + offset_x].g = g>ONE ? ONE : g;
				fb[iy_fbw + ix + offset_x].b = b>ONE ? ONE : b;
			}
		}*/
}
