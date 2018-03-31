void fwrite_le_short(uint16_t s, FILE *file)	// write to file a 16-bit integer in little endian
{
	uint8_t byte;

	byte = s;
	fwrite(&byte, 1, 1, file);
	byte = s >> 8;
	fwrite(&byte, 1, 1, file);
}

void fwrite_le_word(uint32_t w, FILE *file)	// write to file a 32-bit integer in little endian
{
	uint8_t byte;

	byte = w;
	fwrite(&byte, 1, 1, file);
	byte = w >> 8;
	fwrite(&byte, 1, 1, file);
	byte = w >> 16;
	fwrite(&byte, 1, 1, file);
	byte = w >> 24;
	fwrite(&byte, 1, 1, file);
}

void save_raster_bmp(char *path, raster_t im)
{
	FILE *bmpfile;
	int32_t	i, iy, ix;			// various iterators
	int32_t	filesize, imagesize;
	uint8_t	zerobytes, val, zero=0;
	srgb_t pv;

	bmpfile = fopen_utf8(path, "wb");
	if (bmpfile==NULL)
	{
		fprintf_rl(stderr, "Couldn't open \"%s\" for writing in save_raster_bmp()\n", path);
		return ;
	}

	zerobytes = (4 - ((im.dim.x*3) & 3)) & 3;		// computation of zero bytes which pads lines so they're 4 byte-aligned

	//********Tags********

	filesize = 56 + ((im.dim.x*3)+zerobytes) * im.dim.y;
	imagesize = 2 + ((im.dim.x*3)+zerobytes) * im.dim.y;

	fwrite_le_short(19778, bmpfile);
	fwrite_le_word(filesize, bmpfile);
	fwrite_le_word(0, bmpfile);
	fwrite_le_word(54, bmpfile);
	fwrite_le_word(40, bmpfile);
	fwrite_le_word(im.dim.x, bmpfile);
	fwrite_le_word(im.dim.y, bmpfile);
	fwrite_le_short(1, bmpfile);
	fwrite_le_word(24, bmpfile);
	fwrite_le_short(0, bmpfile);
	fwrite_le_word(imagesize, bmpfile);
	fwrite_le_word(2834, bmpfile);
	fwrite_le_word(2834, bmpfile);
	fwrite_le_word(0, bmpfile);
	fwrite_le_word(0, bmpfile);
	//--------Tags--------

	for (iy=im.dim.y-1; iy >= 0; iy--)		// backwards writing
	{
		for (ix=0; ix < im.dim.x; ix++)
		{
			pv = get_raster_pixel_in_srgb(im, iy*im.dim.x + ix);

			fwrite(&pv.r, 1, 1, bmpfile);
			fwrite(&pv.g, 1, 1, bmpfile);
			fwrite(&pv.b, 1, 1, bmpfile);
		}

		for (i=0; i < zerobytes; i++)
			fwrite(&zero, 1, 1, bmpfile);	// write padding bytes
	}

	fwrite_le_short(0, bmpfile);

	fclose(bmpfile);
}
