#include <string.h>	// for memcpy
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
}

void blit_layout(lrgb_t *fb, lrgb_t *sprite, int32_t w, int32_t h)
{
	int32_t i, wh;
	lrgb_t *p;

	// Current layout: ONES: 349998   MIDS: 20470   ZEROS: 169436
	// Cycles:	ONES: 7		MIDS: 11	ZEROS: ~0

	wh = w*h;
	for (i=0; i<wh; i++)
	{
		p = &sprite[i];

		if (p->a)
		if (p->a==ONE)
			fb[i] = *p;
		else
		{
			fb[i].r = (((int32_t) p->r - (int32_t) fb[i].r) * p->a >> LBD) + fb[i].r;
			fb[i].g = (((int32_t) p->g - (int32_t) fb[i].g) * p->a >> LBD) + fb[i].g;
			fb[i].b = (((int32_t) p->b - (int32_t) fb[i].b) * p->a >> LBD) + fb[i].b;
		}
	}
}
