// Uses either zlib, miniz or a cutdown Inflate-only miniz depending on RL_ZLIB/RL_MINIZ

#ifndef RL_ZLIB
  #ifdef RL_MINIZ
    #include "orig/miniz.c"
  #else
    #include "miniz_cutdown.c"
  #endif
#endif

int gz_decompress(const uint8_t *src, const size_t src_len, uint8_t **dst, size_t *dst_alloc)
{
	int ret = -1;
	z_stream strm={0};

	strm.avail_in  = src_len;
	strm.avail_out = *dst_alloc;
	strm.next_in   = (Bytef *) src;
	strm.next_out  = (Bytef *) *dst;

	#ifdef RL_ZLIB
	ret = inflateInit2(&strm, 15+32);	// 15 window bits, and the +32 tells zlib to to detect if using gzip or zlib
	#else
	ret = inflateInit2(&strm, MZ_DEFAULT_WINDOW_BITS);
	#endif
	if (ret != Z_OK)
		return ret;

	do
	{
		strm.avail_out = *dst_alloc - strm.total_out;		// updated available free output space
		ret = inflate(&strm, Z_NO_FLUSH);

		if (ret==Z_OK && strm.avail_out==0)			// if we need more output space to go on
		{
			alloc_enough(dst, strm.total_out + 1024, dst_alloc, sizeof(uint8_t), 2.);
			strm.next_out = *dst + strm.total_out;		// updated output address
		}
	}
	while (ret==Z_OK);

	if (ret == Z_STREAM_END)
	{
		ret = strm.total_out;
		*dst = realloc(*dst, strm.total_out+1);	// shrink the output buffer to the right size
		(*dst)[strm.total_out] = 0;
		*dst_alloc = strm.total_out;
	}

	inflateEnd(&strm);
	return ret;
}

buffer_t gz_decompress_to_buffer(const uint8_t *src, const size_t src_len)
{
	buffer_t b={0};

	gz_decompress(src, src_len, &b.buf, &b.len);

	if (b.len > 0)
		b.as = b.len + 1;

	return b;
}

buffer_t gz_compress_to_buffer(const uint8_t *data, const size_t data_size, const int comp_level)
{
	buffer_t bz={0};
#if defined(RL_ZLIB) || defined(RL_MINIZ)
	mz_ulong z_size;

	z_size = compressBound(data_size);
	bz.buf = calloc(z_size, sizeof(uint8_t));

	compress2(bz.buf, &z_size, data, data_size, comp_level);

	// TODO realloc
	bz.as = bz.len = z_size;
#else
	fprintf_rl(stderr, "Define RL_MINIZ in order to be able to use gz_compress_to_buffer()\n");
#endif
	return bz;
}

uint8_t *gz_decompress_file(const char *path, size_t *data_size)
{
	uint8_t *src, *data=NULL;
	size_t src_len;

	src = load_raw_file(path, &src_len);
	gz_decompress(src, src_len, &data, data_size);

	free(src);

	return data;
}
