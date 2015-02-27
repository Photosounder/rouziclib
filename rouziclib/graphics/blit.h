enum
{	// alignments
	A_CEN,
	A_LEF,
	A_RIG,
	A_TOP,
	A_BOT,
};

extern int sprite_offsets(int32_t fbw, int32_t fbh, int32_t spw, int32_t sph, int32_t *pos_x, int32_t *pos_y, int32_t *offset_x, int32_t *offset_y, int32_t *start_x, int32_t *start_y, int32_t *stop_x, int32_t *stop_y, int hmode, int vmode);
extern void blit_sprite(lrgb_t *fb, int32_t fbw, int32_t fbh, lrgb_t *sprite, int32_t spw, int32_t sph, int32_t pos_x, int32_t pos_y, int blendingmode, int hmode, int vmode);
extern void blit_layout(lrgb_t *fb, lrgb_t *sprite, int32_t w, int32_t h);
