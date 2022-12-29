_Thread_local uint16_t (*read16)(const void *, size_t *);
_Thread_local uint32_t (*read32)(const void *, size_t *);

// format documentation: https://www.itu.int/itudoc/itu-t/com16/tiff-fx/docs/tiff6.pdf
// also https://gitlab.com/libtiff/libtiff/-/blob/master/libtiff/tiff.h

int tiff_tag_type_size(const int type)
{
	switch (type)
	{
		case 1:
		case 2:
		case 6:
		case 7:
			return 1;

		case 3:
		case 8:
			return 2;

		case 4:
		case 9:
		case 11:
		case 13:
			return 4;

		case 5:
		case 10:
		case 12:
		case 16:
		case 17:
		case 18:
			return 8;
	}

	return 0;
}

#include "tiff_tags.h"	// contains the tifftag array with the members .tag and .name
const char *find_tifftag_name(const int tag)
{
	int i;

	for (i=0; tifftag[i].tag > 0; i++)
		if (tifftag[i].tag == tag)
			return tifftag[i].name;

	return NULL;
}

void print_tiff_tag(uint8_t *data, uint8_t *p, uint32_t value32, int tag, int type, int count)
{
	int i, disp_count;
	uint32_t num, den;
	const char *tag_name=NULL;

	tag_name = find_tifftag_name(tag);
	fprintf_rl(stdout, "Tag %s (%d, type %d, count %d", tag_name ? tag_name : "of unknown type", tag, type, count);
	if (count * tiff_tag_type_size(type) > 4)
	{
		p = &data[value32];
		fprintf_rl(stdout, ", offset 0x%X", value32);
	}
	fprintf_rl(stdout, "):");

	disp_count = count;
	if (count > 16)
		disp_count = MINN(count, 10);

	switch (type)
	{
		case 2:		// string
			fprintf_rl(stdout, " \"%s\"", p);
			break;

		case 1:		// u8
		case 7:		// ?8
			for (i=0; i < disp_count; i++)
				fprintf_rl(stdout, " %d", read_byte8(p, (size_t *) &p));
			break;

		case 6:		// s8
			for (i=0; i < disp_count; i++)
				fprintf_rl(stdout, " %d", read_byte8s(p, (size_t *) &p));
			break;

		case 3:		// u16
			for (i=0; i < disp_count; i++)
				fprintf_rl(stdout, " %d", read16(p, (size_t *) &p));
			break;

		case 8:		// s16
			for (i=0; i < disp_count; i++)
				fprintf_rl(stdout, " %d", (int16_t) read16(p, (size_t *) &p));
			break;

		case 4:		// u32
		case 13:	// u32
			for (i=0; i < disp_count; i++)
				fprintf_rl(stdout, " %d", read32(p, (size_t *) &p));
			break;

		case 9:		// s32
			for (i=0; i < disp_count; i++)
				fprintf_rl(stdout, " %d", (int32_t) read32(p, (size_t *) &p));
			break;

		case 5:		// u32 / u32 rational
			for (i=0; i < disp_count; i++)
			{
				num = read32(p, (size_t *) &p);
				den = read32(p, (size_t *) &p);
				fprintf_rl(stdout, "  %g (%d/%d)", (double) num / (double) den, num, den);
			}
			break;

		case 10:	// s32 / s32 rational
			for (i=0; i < disp_count; i++)
			{
				num = read32(p, (size_t *) &p);
				den = read32(p, (size_t *) &p);
				fprintf_rl(stdout, "  %g (%d/%d)", (double) (int32_t) num / (double) (int32_t) den, (int32_t) num, (int32_t) den);
			}
			break;

		case 11:	// float
			for (i=0; i < disp_count; i++)
				fprintf_rl(stdout, " %g", u32_as_float(read32(p, (size_t *) &p)));
			break;

		case 12:	// double
			for (i=0; i < disp_count; i++)
				fprintf_rl(stdout, " %g", u64_as_double(read_LE64(p, (size_t *) &p)));
			break;
	}
	if (count > disp_count && type != 2)
		fprintf_rl(stdout, "...");
	fprintf_rl(stdout, "\n");
}

void load_tiff_ifd_entry(uint8_t *data, uint8_t *p, tiff_info_t *info)
{
	uint32_t tag, type, count, value16, value32, value, *ptr_value;

	tag = read16(p, (size_t *) &p);
	type = read16(p, (size_t *) &p);	// 3 means u16, 4 means u32
	count = read32(p, (size_t *) &p);
	ptr_value = (uint32_t *) p;
	value16 = read16(p, NULL);
	value32 = read32(p, NULL);
	value = tiff_tag_type_size(type)==2 ? value16 : value32;

	//print_tiff_tag(data, p, value32, tag, type, count);

	switch (tag)
	{
		case 256:	// image width
			info->dim.x = value;
			break;

		case 257:	// image height
			info->dim.y = value;
			break;

		case 258:	// bits per channel, each channel has its own bit depth
			if (info->chan==0)
				info->chan = count;

			if (count > 2)
				info->bpc = read16(&data[value32], NULL);
			else
				info->bpc = value;
			break;

		case 259:	// compression
			info->compression = value;
			break;

		case 262:	// photometric (what the channels represent)
			info->photometric = value;
			break;

		case 273:	// offsets to data strips
			info->offset_count = count;
			if (count > 1)
				info->data_offset = (uint32_t *) &data[value32];
			else
				info->data_offset = ptr_value;
			break;

		case 277:	// channel count aka samplesperpixel
			info->chan = value;
			break;

		case 278:	// rows per strip
			info->rowsperstrip = value;
			break;

		case 284:	// planar config (1 for interleaved, 2 for planar)
			info->planarconfig = value;
			break;

		case 317:	// LZW Differencing Predictor
			info->lzw_diff = value;
			break;

		case 339:	// sample format
			if (count > 2)
				info->sample_format = read16(&data[value32], NULL);
			else
				info->sample_format = value;
			break;
	}
}

size_t load_tiff_ifd(uint8_t *data, size_t ifd_index, tiff_info_t *info)
{
	uint8_t *p = &data[ifd_index];
	int i, entry_count;

	entry_count = read16(p, (size_t *) &p);

	for (i=0; i < entry_count; i++)
	{
		load_tiff_ifd_entry(data, p, info);
		p = &p[12];
	}

	// Calculate the size of decoded strips
	info->bytesperstrip = info->rowsperstrip * info->dim.x * ceil_rshift(info->bpc, 3);
	if (info->planarconfig == 1)				// if the channels are interleaved
		info->bytesperstrip *= info->chan;

	return read32(p, NULL);
}

float *load_tiff_pix_data_fl32(void *im_data, tiff_info_t info, int out_chan)
{
	float *im, *in_pixel;
	size_t i, count;
	int ic;
	static float *s8lut=NULL;
	frgb_t f;

	if (s8lut==NULL)
		s8lut = get_lut_slrgb().flut;

	count = mul_x_by_y_xyi(info.dim);
	im = calloc(count*out_chan, sizeof(float));
	in_pixel = calloc(info.chan, sizeof(float));

	if ((info.compression==1 || info.compression==5) && info.photometric<=2)
	{
		// 8-bit unsigned int
		if (info.bpc==8 && info.sample_format==1)
			for (i=0; i < count; i++)
			{
				for (ic=0; ic < info.chan; ic++)
					in_pixel[ic] = s8lut[((uint8_t *) im_data)[i*info.chan + ic]];
				image_float_channel_conversion(in_pixel, info.chan, &im[i*out_chan], out_chan);
			}

		// 16-bit unsigned int
		if (info.bpc==16 && info.sample_format==1)
			if (info.be)
				for (i=0; i < count; i++)
				{
					for (ic=0; ic < info.chan; ic++)
						in_pixel[ic] = s16lrgb(read_BE16(&((uint16_t *) im_data)[i*info.chan + ic], NULL));
					image_float_channel_conversion(in_pixel, info.chan, &im[i*out_chan], out_chan);
				}
			else
				for (i=0; i < count; i++)
				{
					for (ic=0; ic < info.chan; ic++)
						in_pixel[ic] = s16lrgb(((uint16_t *) im_data)[i*info.chan + ic]);
					image_float_channel_conversion(in_pixel, info.chan, &im[i*out_chan], out_chan);
				}

		// 32-bit float
		if (info.bpc==32 && info.sample_format==3)
			if (info.be)
				for (i=0; i < count; i++)
				{
					for (ic=0; ic < info.chan; ic++)
						in_pixel[ic] = u32_as_float(read_BE32(&((uint32_t *) im_data)[i*info.chan + ic], NULL));
					image_float_channel_conversion(in_pixel, info.chan, &im[i*out_chan], out_chan);
				}
			else
				for (i=0; i < count; i++)
				{
					for (ic=0; ic < info.chan; ic++)
						in_pixel[ic] = ((float *) im_data)[i*info.chan + ic];
					image_float_channel_conversion(in_pixel, info.chan, &im[i*out_chan], out_chan);
				}
	}

	free(in_pixel);

	return im;
}

void *load_tiff_pix_data_raw(void *im_data, tiff_info_t info)
{
	void *im;
	size_t i, count, pix_size;
	int ic;

	pix_size = info.bpc / 8;
	count = mul_x_by_y_xyi(info.dim);
	im = calloc(count*info.chan, pix_size);

	if (info.compression==1 && info.photometric<=2)
		memcpy(im, im_data, count*info.chan*pix_size);

	return im;
}

int is_file_tiff_mem(uint8_t *data)
{
	// Check that the file starts with the endianness TIFF tag
	if (strcmp(data, "II*")==0 || strcmp(data, "MM")==0)
		return 1;
	return 0;
}

tiff_info_t load_tiff_info(uint8_t *data)
{
	tiff_info_t info={0};
	size_t ifd_index;

	// Check that the file starts with the endianness TIFF tag and set the reading functions
	if (strcmp(data, "II*")==0)
	{
		read16 = read_LE16;
		read32 = read_LE32;
	}
	else if (strcmp(data, "MM")==0)
	{
		read16 = read_BE16;
		read32 = read_BE32;
		info.be = 1;
	}
	else
	{
		fprintf_rl(stderr, "In load_tiff_info(): not a little or big endian TIFF file\n");
		return info;
	}

	// set default info values in case of missing tags
	info.sample_format = 1;

	// Load tags
	ifd_index = read32(&data[4], NULL);
	while (info.compression != 1 && info.compression != 5 && ifd_index)		// find an uncompressed or LZW-compressed IFD
		ifd_index = load_tiff_ifd(data, ifd_index, &info);

	return info;
}

float *load_tiff_mem(uint8_t *data, xyi_t *dim, int *out_chan)
{
	int i;
	float *im;
	tiff_info_t info;

	info = load_tiff_info(data);		// load info and find pixel data
	*dim = info.dim;
	if (*out_chan <= 0)			// 0 loads whatever number of channels there is
		*out_chan = info.chan;

	if (info.compression==5)	// LZW decompression
	{
		buffer_t dec={0};	// the decoded output

		buf_alloc_enough(&dec, (size_t) mul_x_by_y_xyi(info.dim) * info.chan * info.bpc / 8);	// alloc the expected output size

		for (i=0; i < info.offset_count; i++)				// go through every LZW strip
			tiff_lzw_decode(&data[read32(&info.data_offset[i], NULL)], &dec, info.bytesperstrip);

		if (info.lzw_diff==2)						// horizontal difference decoding
			tiff_lzw_diff_decode(dec.buf, info);

		im = load_tiff_pix_data_fl32(dec.buf, info, *out_chan);		// convert decoded data
		free_buf(&dec);
	}
	else
		im = load_tiff_pix_data_fl32(&data[read32(&info.data_offset[0], NULL)], info, *out_chan);	// convert data

	return im;
}

void *load_tiff_mem_raw(uint8_t *data, tiff_info_t *info)
{
	void *im;

	*info = load_tiff_info(data);		// load info and find pixel data

	im = load_tiff_pix_data_raw(&data[read32(&info->data_offset[0], NULL)], *info);	// copy data

	return im;
}

raster_t load_tiff_mem_raster(uint8_t *data)
{
	float *im;
	xyi_t dim={0};
	int out_chan=4;

	im = load_tiff_mem(data, &dim, &out_chan);

	return make_raster(im, dim, XYI0, IMAGE_USE_FRGB);
}

float *load_tiff_file(const char *path, xyi_t *dim, int *out_chan)	// *out_chan of 0 loads the actual number of channels, > 0 forces the number
{
	uint8_t *data;
	size_t data_len;
	float *im=NULL;

	data = load_raw_file(path, &data_len);
	if (data==NULL)
		return im;

	im = load_tiff_mem(data, dim, out_chan);
	free(data);

	return im;
}

void *load_tiff_file_raw(const char *path, tiff_info_t *info)
{
	uint8_t *data;
	size_t data_len;
	void *im=NULL;

	data = load_raw_file(path, &data_len);
	if (data==NULL)
		return im;

	im = load_tiff_mem_raw(data, info);
	free(data);

	return im;
}

raster_t load_tiff_file_raster(const char *path)
{
	uint8_t *data;
	size_t data_len;
	raster_t r={0};

	data = load_raw_file(path, &data_len);
	if (data==NULL)
		return r;

	r = load_tiff_mem_raster(data);
	free(data);

	return r;
}


// Writing

void write_tiff_ifd_entry(FILE *file, size_t *misc_start, int tag, int type, uint32_t count, int64_t value)
{
	int i, val_size, write_count;

	// Write tag, type and count
	fwrite_LE16(file, tag);
	fwrite_LE16(file, type);
	fwrite_LE32(file, count);

	// If the value data takes more than 4 bytes we write the offset to the data
	val_size = count * tiff_tag_type_size(type);
	if (val_size > 4)
	{
		write_count = 1;
		value = *misc_start;
		*misc_start += ceil_rshift(val_size, 1) << 1;
	}
	else
		write_count = count;

	// Write the value or offset to the value data
	for (i=0; i < write_count; i++)
	{
		if (val_size > 4)
			fwrite_LE32(file, value);
		else if (tiff_tag_type_size(type) == 4)
			fwrite_LE32(file, value);
		else if (tiff_tag_type_size(type) == 2)
			fwrite_LE16(file, value);
		else
			fwrite_byte8(file, value);
	}

	// Zero padding if the value took less than 4 bytes
	for (i=val_size; i < 4; i++)
		fwrite_byte8(file, 0);
}

int tiff_store_pixels_last = 1;

uint32_t tiff_calc_strip_offset(uint32_t data_start, int out_chan, int strip_count)
{
	int offset;

	if (tiff_store_pixels_last)
	{
		offset = data_start;
		offset += 4*4;					// x/y resolution

		if (out_chan*tiff_tag_type_size(3) > 4)
		{
			offset += 2*out_chan;			// bpc
			offset += 2*out_chan;			// sample format
		}

		if (strip_count > 1)
		{
			offset += 4*strip_count;		// strip offsets
			offset += 4*strip_count;		// strip byte count
		}

		offset += get_icc_profile_size(out_chan);	// ICC profile
	}
	else
		offset = 8;

	return offset;
}

int save_image_tiff(const char *path, float *im, xyi_t dim, int in_chan, int out_chan, int bpc)
{
	FILE *file;
	size_t i, pix_count, ifd_index, misc_start;
	uint64_t pix_data_size;
	int ic, byte_depth, sample_format;
	
	uint32_t strip_size, strip_size2, row_size;
	int32_t row_count, row_count2;

	if (im==NULL || is0_xyi(dim))
		return 0;

	file = fopen_utf8(path, "wb");
	if (file==NULL)
	{
		fprintf_rl(stderr, "save_image_tiff() couldn't open file '%s' for writing.\n", path);
		return 0;
	}

	fprintf_override(file, "II*%c", 0);	// little endian TIFF tag

	// IFD index
	sample_format = bpc < 32 ? 1 : 3;			// 1 for uint, 3 for float
	pix_count = dim.x * dim.y;
	byte_depth = ceil_rshift(bpc, 3);			// bytes per channel
	pix_data_size = pix_count * out_chan * byte_depth;	// size in bytes of the pixel data
	if (tiff_store_pixels_last)
		ifd_index = 8;
	else
		ifd_index = 8 + pix_data_size;
	fwrite_LE32(file, ifd_index);

	// Write pixel data
	if (tiff_store_pixels_last == 0)
		if (bpc == 32)
			for (i=0; i < pix_count; i++)
				fwrite(&im[i*in_chan], out_chan, sizeof(float), file);

	// IFD
	int strip_count = (pix_data_size >> 32) == 1 ? 2 : 1;				// 2 strips if between 4 GB and 8 GB
	const int entry_count = 14;							// increment this number if adding an entry
	misc_start = ifd_index+2 + entry_count*12 + 4;
	uint32_t strip_offset = tiff_calc_strip_offset(misc_start, out_chan, strip_count);

	if (strip_count == 2)
	{
		row_size = dim.x * out_chan * byte_depth;

		// Choose row count for the strips
		row_count = (0xFFFFFFFFUL - strip_offset) / row_size;
		if (row_count >= dim.y)
			row_count = dim.y / 2;
		row_count2 = dim.y - row_count;

		// Size in bytes of each strip
		strip_size = row_count * row_size;
		strip_size2 = row_count2 * row_size;
	}
	else
		row_count = dim.y;

	fwrite_LE16(file, entry_count);		// entry count
	write_tiff_ifd_entry(file, &misc_start, 256, 4, 1, dim.x);
	write_tiff_ifd_entry(file, &misc_start, 257, 4, 1, dim.y);
	write_tiff_ifd_entry(file, &misc_start, 258, 3, out_chan, bpc);			// bpc
	write_tiff_ifd_entry(file, &misc_start, 259, 3, 1, 1);				// compression (uncompressed)
	write_tiff_ifd_entry(file, &misc_start, 262, 3, 1, out_chan==1 ? 1 : 2);	// photometric (1 is grey, 2 is RGB)

	write_tiff_ifd_entry(file, &misc_start, 273, 4, strip_count, strip_offset);	// strip offsets (only one)
	write_tiff_ifd_entry(file, &misc_start, 277, 3, 1, out_chan);			// samples per pixel
	write_tiff_ifd_entry(file, &misc_start, 278, 4, 1, row_count);			// rows per strip
	if (strip_count == 1)
		strip_size = pix_data_size >> 32 ? 0 : pix_data_size;			// set strip size of 0 if > 4 GB
	write_tiff_ifd_entry(file, &misc_start, 279, 4, strip_count, strip_size);	// strip byte count

	write_tiff_ifd_entry(file, &misc_start, 282, 5, 1, 0);				// x resolution
	write_tiff_ifd_entry(file, &misc_start, 283, 5, 1, 0);				// y resolution
	write_tiff_ifd_entry(file, &misc_start, 296, 3, 1, 2);				// resolution units (inches)
	write_tiff_ifd_entry(file, &misc_start, 339, 3, out_chan, sample_format);	// sample format
	write_tiff_ifd_entry(file, &misc_start, 34675, 7, get_icc_profile_size(out_chan), 0);	// ICC profile (linear)

	fwrite_LE32(file, 0);	// index of next IFD

	// Write the bpc (tag 258)
	if (out_chan*tiff_tag_type_size(3) > 4)
		for (i=0; i < out_chan; i++)
			fwrite_LE16(file, bpc);

	if (strip_count == 2)
	{
		// Write strip offsets (tag 273)
		fwrite_LE32(file, strip_offset);
		fwrite_LE32(file, strip_offset + strip_size);

		// Write strip byte count (tag 279)
		fwrite_LE32(file, strip_size);
		fwrite_LE32(file, strip_size2);
	}

	// Write x/y resolution, 72/1 dpi (tags 282 and 283)
	fwrite_LE32(file, 72);
	fwrite_LE32(file, 1);
	fwrite_LE32(file, 72);
	fwrite_LE32(file, 1);

	// Write the sample format (tag 339)
	if (out_chan*tiff_tag_type_size(3) > 4)
		for (i=0; i < out_chan; i++)
			fwrite_LE16(file, sample_format);

	// Write ICC profile (tag 34675)
	write_icc_linear_profile(file, out_chan);

	// Write pixel data
	if (tiff_store_pixels_last)
		if (bpc == 32)
			for (i=0; i < pix_count; i++)
				fwrite(&im[i*in_chan], out_chan, sizeof(float), file);

	fclose(file);

	return 1;
}
