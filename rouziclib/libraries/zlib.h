#ifdef RL_ZLIB

#pragma comment(lib,"zlib.lib")

#include <zlib.h>

extern int gz_decompress(const uint8_t *src, int src_len, uint8_t **dst, int *dst_alloc);

#endif
