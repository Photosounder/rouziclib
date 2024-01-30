#define ICC_TAG_COUNT_SRGB_LIN 11
#define ICC_TAG_COUNT_GREY_LIN 4
#define ICC_DESC_SRGB_LIN "sRGB-linear"
#define ICC_DESC_GREY_LIN "greyscale-linear"
#define ICC_CPRT "."

size_t icc_mluc_size(const char *string, size_t *padding)
{
	size_t raw_size = 28 + 2 * strlen_utf8_to_utf16(string);
	size_t padded_size = ceil_rshift(raw_size, 2) << 2;

	if (padding)
		*padding = padded_size - raw_size;

	return padded_size;
}

size_t get_icc_profile_size(const int channel_count)
{
	int size = 128;

	if (channel_count == 1)
	{
		size += 4 + ICC_TAG_COUNT_GREY_LIN * 12;
		size += icc_mluc_size(ICC_DESC_GREY_LIN, NULL);
		size += icc_mluc_size(ICC_CPRT, NULL);
		size += 20 + 16;
	}
	else
	{
		size += 4 + ICC_TAG_COUNT_SRGB_LIN * 12;
		size += icc_mluc_size(ICC_DESC_SRGB_LIN, NULL);
		size += icc_mluc_size(ICC_CPRT, NULL);
		size += 20 + 44 + 3*20 + 16 + 36;
	}

	return size;
}

void write_icc_profile_mluc(FILE *file, const char *string)
{
	size_t i, ic, tag_size, padding, utf16_size;
	uint16_t utf16[3];
	uint32_t c;

	icc_mluc_size(string, &padding);

	// Write tags
	fprintf_override(file, "mluc");				// Type signature
	fwrite_BE32(file, 0);					// Reserved
	fwrite_BE32(file, 1);					// Number of records
	fwrite_BE32(file, 12);					// Record size
	fprintf_override(file, "enUS");				// Language and country code
	fwrite_BE32(file, 2 * strlen_utf8_to_utf16(string));	// Size of the string
	fwrite_BE32(file, 28);					// Offset of record

	// Write UTF-16 string
	for (i=0; string[i]; i++)
	{
		c = utf8_to_unicode32(&string[i], &i);
		sprint_utf16(utf16, c);
		utf16_size = codepoint_utf16_size(c);

		for (ic=0; ic < utf16_size; ic++)
			fwrite_BE16(file, utf16[ic]);
	}

	// Write padding bytes
	for (i=0; i < padding; i++)
		fwrite_byte8(file, 0);
}

void write_icc_profile_tag(FILE *file, const char *signature, uint32_t *pos, uint32_t size)
{
	// Write tag
	fprintf_override(file, "%4s", signature);
	fwrite_BE32(file, *pos);
	fwrite_BE32(file, size);

	// Update the data position for the next tag
	*pos += size;
}

void write_icc_linear_profile(FILE *file, const int chan)
{
	// See https://www.color.org/specification/ICC.1-2022-05.pdf starting from section 7.2 (page 33)
	int i;
	const char *desc = chan==1 ? ICC_DESC_GREY_LIN : ICC_DESC_SRGB_LIN;
	int tag_count = chan==1 ? ICC_TAG_COUNT_GREY_LIN : ICC_TAG_COUNT_SRGB_LIN;
	uint32_t data_pos = 128 + 4 + tag_count*12;

	// Write header
	fwrite_BE32(file, get_icc_profile_size(chan));		// Profile size
	fprintf_override(file, "lcms");				// CMM type signature
	fwrite_BE32(file, 0x04400000);				// Profile version (4.4.0.0)
	if (chan == 1)
		fprintf_override(file, "mntrGRAYXYZ ");		// Device class, colour space, PCS
	else
		fprintf_override(file, "mntrRGB XYZ ");
	fwrite_BE16(file, 2001);				// Date and time of creation
	for (i=0; i < 5; i++)					// 2001-01-01 01:01:01
		fwrite_BE16(file, 1);
	fprintf_override(file, "acspMSFT");			// Signature, platform
	for (i=0; i < 6; i++)					// Flags, device manufacturer, model,
		fwrite_BE32(file, 0);				// attributes, rendering intent
	fwrite_BE32(file, 0xF6D6);				// PCS illuminant
	fwrite_BE32(file, 0x10000);
	fwrite_BE32(file, 0xD32D);
	fprintf_override(file, "lcms");				// Profile creator
	for (i=0; i < 4+7; i++)					// Profile ID, reserved
		fwrite_BE32(file, 0);

	// Tag table
	fwrite_BE32(file, tag_count);	// Tag count
	write_icc_profile_tag(file, "desc", &data_pos, icc_mluc_size(desc, NULL));	// Profile description
	write_icc_profile_tag(file, "cprt", &data_pos, icc_mluc_size(ICC_CPRT, NULL));	// Copyright
	write_icc_profile_tag(file, "wtpt", &data_pos, 20);				// White point
	if (chan == 1)
	{
		write_icc_profile_tag(file, "kTRC", &data_pos, 16);			// Grey tone reproduction curve
	}
	else
	{
		write_icc_profile_tag(file, "chad", &data_pos, 44);			// Chromatic adaptation
		write_icc_profile_tag(file, "rXYZ", &data_pos, 20);			// Red matrix
		write_icc_profile_tag(file, "bXYZ", &data_pos, 20);			// Blue matrix
		write_icc_profile_tag(file, "gXYZ", &data_pos, 20);			// Green matrix
		write_icc_profile_tag(file, "rTRC", &data_pos, 16);			// Red tone reproduction curve
		data_pos -= 16;								// rewind so all channels use one curve
		write_icc_profile_tag(file, "gTRC", &data_pos, 16);			// Green tone reproduction curve
		data_pos -= 16;
		write_icc_profile_tag(file, "bTRC", &data_pos, 16);			// Blue tone reproduction curve
		write_icc_profile_tag(file, "chrm", &data_pos, 36);			// Chromaticity
	}

	// Data that tags point to
	write_icc_profile_mluc(file, desc);		// desc
	write_icc_profile_mluc(file, ICC_CPRT);		// cprt

	fprintf_override(file, "XYZ ");			// wtpt
	fwrite_BE32(file, 0);
	fwrite_BE32(file, 0xF6D6);
	fwrite_BE32(file, 0x10000);
	fwrite_BE32(file, 0xD32D);

	if (chan > 1)
	{
		fprintf_override(file, "sf32");		// chad (3x3 matrix)
		fwrite_BE32(file, 0);
		fwrite_BE32(file, 68674);
		fwrite_BE32(file, 1502);
		fwrite_BE32(file, 0xFFFFF325);
		fwrite_BE32(file, 1939);
		fwrite_BE32(file, 64912);
		fwrite_BE32(file, 0xFFFFFBA1);
		fwrite_BE32(file, 0xFFFFFDA2);
		fwrite_BE32(file, 988);
		fwrite_BE32(file, 49262);

		fprintf_override(file, "XYZ ");		// rXYZ
		fwrite_BE32(file, 0);
		fwrite_BE32(file, 28576);
		fwrite_BE32(file, 14581);
		fwrite_BE32(file, 912);

		fprintf_override(file, "XYZ ");		// bXYZ
		fwrite_BE32(file, 0);
		fwrite_BE32(file, 9375);
		fwrite_BE32(file, 3972);
		fwrite_BE32(file, 46788);

		fprintf_override(file, "XYZ ");		// gXYZ
		fwrite_BE32(file, 0);
		fwrite_BE32(file, 25239);
		fwrite_BE32(file, 46983);
		fwrite_BE32(file, 6361);
	}

	fprintf_override(file, "para");			// _TRC (sets a gamma of 1.0)
	fwrite_BE32(file, 0);
	fwrite_BE16(file, 0);				// function type (0 = simple gamma)
	fwrite_BE16(file, 0);
	fwrite_BE32(file, 0x10000);			// gamma

	if (chan > 1)
	{
		fprintf_override(file, "chrm");		// chrm
		fwrite_BE32(file, 0);
		fwrite_BE16(file, 3);			// number of colour channels
		fwrite_BE16(file, 0);
		fwrite_BE32(file, 41943);
		fwrite_BE32(file, 21628);
		fwrite_BE32(file, 19661);
		fwrite_BE32(file, 39322);
		fwrite_BE32(file, 9831);
		fwrite_BE32(file, 3932);
	}
}
