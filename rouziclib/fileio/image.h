#define IMAGE_USE_SRGB	1
#define IMAGE_USE_LRGB	2
#define IMAGE_USE_FRGB	4
#define IMAGE_USE_CL	8

extern void convert_image_srgb8(raster_t *im, const uint8_t *data, const int mode, const void *clctx);
extern raster_t load_image_from_http(char *url, const int mode);
