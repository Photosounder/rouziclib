enum
{
	NODITHER,
	DITHER,
};

extern lut_t get_lut_lsrgb();
extern lut_t get_lut_slrgb();
extern lut_t dither_lut_init();
extern lut_t bytecheck_lut_init(int border);
extern void convert_lrgb_to_srgb(srgb_t *sfb, lrgb_t *fb, int32_t pixc, int mode);
