int utf8_char_size(uint8_t *c)
{
	const uint8_t	m0x	= 0x80, c0x	= 0x00,
	      		m10x	= 0xC0, c10x	= 0x80,
	      		m110x	= 0xE0, c110x	= 0xC0,
	      		m1110x	= 0xF0, c1110x	= 0xE0,
	      		m11110x	= 0xF8, c11110x	= 0xF0;

	if ((c[0] & m0x) == c0x)
		return 1;

	if ((c[0] & m110x) == c110x)
	if ((c[1] & m10x) == c10x)
		return 2;

	if ((c[0] & m1110x) == c1110x)
	if ((c[1] & m10x) == c10x)
	if ((c[2] & m10x) == c10x)
		return 3;

	if ((c[0] & m11110x) == c11110x)
	if ((c[1] & m10x) == c10x)
	if ((c[2] & m10x) == c10x)
	if ((c[3] & m10x) == c10x)
		return 4;

	if ((c[0] & m10x) == c10x)	// not a first UTF-8 byte
		return 0;

	return -1;			// if c[0] is a first byte but the other bytes don't match
}

uint32_t utf8_to_unicode32(uint8_t *c, int32_t *index)
{
	uint32_t v;
	int size;
	const uint8_t m6 = 63, m5 = 31, m4 = 15, m3 = 7;

	size = utf8_char_size(c);

	if (size > 0 && index)
		*index += size-1;

	switch (size)
	{
		case 1:
			v = c[0];
			break;
		case 2:
			v = c[0] & m5;
			v = v << 6 | c[1] & m6;
			break;
		case 3:
			v = c[0] & m5;
			v = v << 6 | c[1] & m6;
			v = v << 6 | c[2] & m6;
			break;
		case 4:
			v = c[0] & m5;
			v = v << 6 | c[1] & m6;
			v = v << 6 | c[2] & m6;
			v = v << 6 | c[3] & m6;
			break;
		case 0:				// not a first UTF-8 byte
		case -1:			// corrupt UTF-8 letter
		default:
			v = -1;
			break;
	}

	return v;
}

uint8_t *sprint_unicode(uint8_t *str, uint32_t c)	// str must be able to hold new 5 bytes and will be null-terminated by this function
{
	const uint8_t m6 = 63, m5 = 31, m4 = 15, m3 = 7;
	const uint8_t	c10x	= 0x80,
	      		c110x	= 0xC0,
	      		c1110x	= 0xE0,
	      		c11110x	= 0xF0;

	if (c < 0x0080)
	{
		str[0] = c;
		str[1] = '\0';
	}
	else if (c < 0x0800)
	{
		str[1] = (c & m6) | c10x;
		c >>= 6;
		str[0] = c | c110x;
		str[2] = '\0';
	}
	else if (c < 0x10000)
	{
		str[2] = (c & m6) | c10x;
		c >>= 6;
		str[1] = (c & m6) | c10x;
		c >>= 6;
		str[0] = c | c1110x;
		str[3] = '\0';
	}
	else if (c < 0x200000)
	{
		str[3] = (c & m6) | c10x;
		c >>= 6;
		str[2] = (c & m6) | c10x;
		c >>= 6;
		str[1] = (c & m6) | c10x;
		c >>= 6;
		str[0] = c | c11110x;
		str[4] = '\0';
	}
	else
		str[0] = '\0';		// Unicode character doesn't map to UTF-8

	return str;
}
