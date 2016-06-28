enum
{
	NODITHER,
	DITHER,
};

extern double lsrgb(double linear);
extern double slrgb(double s);
extern lut_t get_lut_lsrgb();
extern lut_t get_lut_slrgb();
extern lut_t get_lut_lsrgb_fl();
extern int32_t lsrgb_fl(float v, int32_t *lut);
extern lut_t dither_lut_init();
extern lut_t bytecheck_lut_init(int border);
extern void convert_lrgb_to_srgb(raster_t fb, int mode);
extern void convert_frgb_to_srgb(raster_t fb, int mode);
extern void convert_linear_rgb_to_srgb(raster_t fb, int mode);
extern void convert_srgb_to_lrgb(raster_t fb);
extern void convert_frgb_to_lrgb(raster_t *fb);
extern void convert_frgb_to_lrgb_ratio(raster_t *fb, const float ratio);
