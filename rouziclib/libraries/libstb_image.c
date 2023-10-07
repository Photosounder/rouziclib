#ifdef RL_IMAGE_FILE

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#undef pi
#include "orig/stb_image.h"
#define pi RL_PI

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define stbiw__fopen	fopen_utf8
#include "orig/stb_image_write.h"

#endif

raster_t load_image_mem_libstb_image(uint8_t *raw_data, size_t size, const int mode)
{
	raster_t im={0};
#ifdef RL_IMAGE_FILE
	int comp=0;
	uint8_t *b8=NULL;
	uint16_t *b16=NULL;
	float *bf=NULL;

	/*if (stbi_is_hdr_from_memory(raw_data, size))	// only RGBE is supported so who cares
	{
		bf = stbi_loadf_from_memory(raw_data, size, &im.dim.x, &im.dim.y, &comp, 4);
	}
	else*/ if (stbi_is_hdr_from_memory(raw_data, size) || stbi_is_16_bit_from_memory(raw_data, size))
	{
		b16 = stbi_load_16_from_memory(raw_data, size, &im.dim.x, &im.dim.y, &comp, 4);

		convert_image_srgb16(&im, b16, mode);
		free(b16);
	}
	else
	{
		b8 = stbi_load_from_memory(raw_data, size, &im.dim.x, &im.dim.y, &comp, 4);

		if (mode & IMAGE_USE_SRGB)
			im.srgb = (srgb_t *) b8;

		convert_image_srgb8(&im, b8, mode);
		if (im.srgb != (srgb_t *) b8)
			free(b8);
	}
#endif

	return im;
}

int save_image_srgb_libstb(const char *path, raster_t r, int jpg_quality)
{
#ifdef RL_IMAGE_FILE
	char ext[32];

	extract_file_extension(path, ext);

	if (strcmp(ext, "png")==0)
		return stbi_write_png(path, r.dim.x, r.dim.y, 4, r.srgb, r.dim.x * sizeof(srgb_t));

	if (strcmp(ext, "bmp")==0)
		return stbi_write_bmp(path, r.dim.x, r.dim.y, 4, r.srgb);

	if (strcmp(ext, "jpg")==0 || strcmp(ext, "jpeg")==0)
		return stbi_write_jpg(path, r.dim.x, r.dim.y, 4, r.srgb, jpg_quality);
#else
	fprintf_rl(stderr, "Define RL_IMAGE_FILE in order to be able to use save_image_srgb_libstb()\n");
#endif

	return 0;
}
