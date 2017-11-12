#ifdef RL_LIBJPEG

#ifdef _MSC_VER
#pragma comment (lib, "jpeg.lib")
#endif

#include <jpeglib.h>
#include <setjmp.h>

typedef struct
{
	xyi_t image_dim, block_dim;	// number of end pixels and blocks
	uint16_t quant[DCTSIZE2];	// 8x8 quantisation table
	int16_t **dct_block;	// array of DCT blocks, each block an 8x8 DCT coef table
} jpeg_comp_dct_t;

extern jpeg_comp_dct_t *libjpeg_get_dct_data(const char *filepath);
extern void free_jpeg_comp_dct(jpeg_comp_dct_t *data);
extern void copy_convert_8x8_block(float *im, xyi_t dim, xyi_t ib, double *block);
extern void paste_convert_8x8_block(float *im, xyi_t dim, xyi_t ib, double *block);

#endif
