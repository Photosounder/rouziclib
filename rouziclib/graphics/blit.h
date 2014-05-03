enum
{	// alignments
	A_CEN,
	A_LEF,
	A_RIG,
	A_TOP,
	A_BOT,
};

extern void blit_sprite(lrgb_t *fb, int32_t fbw, int32_t fbh, lrgb_t *sprite, int32_t spw, int32_t sph, int32_t pos_x, int32_t pos_y, int blendingmode, int hmode, int vmode);
extern void blit_layout(lrgb_t *fb, lrgb_t *sprite, int32_t w, int32_t h);
