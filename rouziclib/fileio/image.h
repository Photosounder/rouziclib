#define IMAGE_USE_SRGB	1
#define IMAGE_USE_LRGB	2
#define IMAGE_USE_FRGB	4
#define IMAGE_USE_SQRGB	8

typedef raster_t (*image_load_mem_func_t)(uint8_t *, size_t, const int);

extern void convert_image_srgb8(raster_t *im, const uint8_t *data, const int mode);
extern raster_t load_image_mem_lib(image_load_mem_func_t load_func, uint8_t *raw_data, size_t size, const int mode);
extern raster_t load_image_lib(image_load_mem_func_t load_func, const char *path, const int mode);
extern raster_t load_image_from_http_lib(image_load_mem_func_t load_func, char *url, const int mode);
extern mipmap_t load_mipmap_from_http_lib(image_load_mem_func_t load_func, char *url, const int mode);
extern mipmap_t load_mipmap_lib(image_load_mem_func_t load_func, const char *path, const int mode);

#define load_image(path, mode)			load_image_lib(NULL, path, mode)
#define load_image_from_http(url, mode)		load_image_from_http_lib(NULL, url, mode);
#define load_mipmap_from_http(url, mode)	load_mipmap_from_http_lib(NULL, url, mode);
#define load_mipmap(path, mode)			load_mipmap_lib(NULL, path, mode)
