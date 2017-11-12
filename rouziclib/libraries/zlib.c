#ifdef RL_ZLIB

int gz_decompress(const uint8_t *src, int src_len, uint8_t **dst, int *dst_alloc)
{
	int ret = -1;
	z_stream strm  = {0};

	strm.avail_in  = src_len;
	strm.avail_out = *dst_alloc;
	strm.next_in   = (Bytef *) src;
	strm.next_out  = (Bytef *) *dst;

	strm.zalloc = Z_NULL;
	strm.zfree  = Z_NULL;
	strm.opaque = Z_NULL;

	ret = inflateInit2(&strm, (15 + 32));	// 15 window bits, and the +32 tells zlib to to detect if using gzip or zlib
	if (ret != Z_OK)
		return ret;

	do
	{
		ret = inflate(&strm, Z_NO_FLUSH);

		if (ret==Z_OK && strm.avail_out==0)	// if we need more output space to go on
		{
			alloc_enough(dst, strm.total_out + 1024, dst_alloc, sizeof(uint8_t), 2.);
			strm.next_out = *dst + strm.total_out;			// updated output address
			strm.avail_out = *dst_alloc - strm.total_out;		// updated available free output space
		}
	}
	while (ret==Z_OK && strm.avail_out==0);
	
	if (ret == Z_STREAM_END)
	{
		ret = strm.total_out;
		*dst = realloc(*dst, strm.total_out+1);	// shrink the output buffer to the right size
		*dst[strm.total_out] = 0;
	}

	inflateEnd(&strm);
	return ret;
}

#endif
