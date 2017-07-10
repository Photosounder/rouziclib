#ifdef RL_LIBRAW

#ifdef _MSC_VER
#pragma comment (lib, "libraw.lib")
#endif

#ifdef _WIN32
#define WIN32
#endif
#include "libraw/libraw.h"
#undef WIN32

typedef struct
{
	uint16_t *data;
	int32_t w, h;
	xyz_t wb;
	double bayer_black[4];
	double maximum_value;
	rect_t image_area;
	xy_t image_centre;
	raster_t preview;
} rawphoto_t;

extern rawphoto_t init_rawphoto(int32_t width, int32_t height);
extern void free_rawphoto(rawphoto_t *rp);
extern raster_t load_raw_thumb(libraw_data_t *rd);
extern rawphoto_t load_raw_photo_bayered(char *path, int load_thumb);
extern raster_t raw_photo_to_raster(raster_t fb, rawphoto_t rp);

#endif
