extern raster_t load_tiff_mem_raster(uint8_t *data);
extern float *load_tiff_file(const char *path, xyi_t *dim, int *out_chan);
extern raster_t load_tiff_file_raster(const char *path);
extern void save_image_tiff(const char *path, float *im, xyi_t dim, int in_chan, int out_chan, int bpc);
