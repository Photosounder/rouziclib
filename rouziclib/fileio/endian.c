// File read

uint8_t fread_byte8(FILE *file)
{
	uint8_t	b;
	fread(&b, 1, 1, file);
	return b;
}

uint16_t fread_LE16(FILE *file)
{
#ifdef ASS_LE
	uint16_t v;
	fread(&v, sizeof(v), 1, file);
	return v;
#else
	uint8_t	b0, b1;

	fread(&b0, 1, 1, file);
	fread(&b1, 1, 1, file);

	return (uint16_t) (b1<<8) | b0;
#endif
}

uint16_t fread_BE16(FILE *file)
{
	uint8_t	b0, b1;

	fread(&b1, 1, 1, file);
	fread(&b0, 1, 1, file);

	return (uint16_t) (b1<<8) | b0;
}

uint32_t fread_LE32(FILE *file)
{
#ifdef ASS_LE
	uint32_t v;
	fread(&v, sizeof(v), 1, file);
	return v;
#else
	uint16_t b0, b1;

	b0 = fread_LE16(file);
	b1 = fread_LE16(file);

	return (uint32_t) (b1<<16) | b0;
#endif
}

uint32_t fread_BE32(FILE *file)
{
	uint16_t b0, b1;

	b1 = fread_LE16(file);
	b0 = fread_LE16(file);

	return (uint32_t) (b1<<16) | b0;
}

// File write
void fwrite_byte8(FILE *file, uint8_t s)
{
	fwrite(&s, 1, 1, file);
}

void fwrite_LE16(FILE *file, uint16_t s)
{
#ifdef ASS_LE
	fwrite(&s, sizeof(s), 1, file);
#else
	uint8_t byte;

	byte = s;
	fwrite(&byte, 1, 1, file);
	byte = s >> 8;
	fwrite(&byte, 1, 1, file);
#endif
}

void fwrite_BE16(FILE *file, uint16_t s)
{
	uint8_t byte;

	byte = s >> 8;
	fwrite(&byte, 1, 1, file);
	byte = s;
	fwrite(&byte, 1, 1, file);
}

void fwrite_LE32(FILE *file, uint32_t w)
{
#ifdef ASS_LE
	fwrite(&w, sizeof(w), 1, file);
#else
	fwrite_LE16(file, w);
	fwrite_LE16(file, w >> 16);
#endif
}

void fwrite_BE32(FILE *file, uint32_t w)
{
	fwrite_BE16(file, w >> 16);
	fwrite_BE16(file, w);
}

void fwrite_LE64(FILE *file, uint64_t w)
{
#ifdef ASS_LE
	fwrite(&w, sizeof(w), 1, file);
#else
	fwrite_LE32(file, w);
	fwrite_LE32(file, w >> 32);
#endif
}

void fwrite_BE64(FILE *file, uint64_t w)
{
	fwrite_BE32(file, w >> 32);
	fwrite_BE32(file, w);
}

// Buffer read
uint8_t read_byte8(const void *ptr, size_t *index)	// used when function pointers are needed
{
	const uint8_t *buf = ptr;
	if (index)
		*index += sizeof(uint16_t);

	return buf[0];
}

int32_t read_byte8s(const void *ptr, size_t *index)
{
	const uint8_t *buf = ptr;
	return (int32_t) (int8_t) read_byte8(buf, index);
}

int32_t read_byte8s_offset(const void *ptr, size_t *index)
{
	const uint8_t *buf = ptr;
	return (int32_t) read_byte8(buf, index) - 128;
}

uint16_t read_LE16(const void *ptr, size_t *index)
{
	const uint8_t *buf = ptr;
	if (index)
		*index += sizeof(uint16_t);

#ifdef ASS_LE
	return *((uint16_t *) buf);
#else
	return (uint16_t) (buf[1] << 8) | buf[0];
#endif
}

uint16_t read_BE16(const void *ptr, size_t *index)
{
	const uint8_t *buf = ptr;
	if (index)
		*index += sizeof(uint16_t);

	return (uint16_t) (buf[0] << 8) | buf[1];
}

int32_t read_LE16s(const void *ptr, size_t *index)
{
	const uint8_t *buf = ptr;
	return (int32_t) (int16_t) read_LE16(buf, index);
}

int32_t read_BE16s(const void *ptr, size_t *index)
{
	const uint8_t *buf = ptr;
	return (int32_t) (int16_t) read_BE16(buf, index);
}

uint32_t read_LE24(const void *ptr, size_t *index)
{
	const uint8_t *buf = ptr;
	if (index)
		*index += 3;

	return (uint32_t) (buf[2] << 16) | (buf[1] << 8) | buf[0];
}

uint32_t read_BE24(const void *ptr, size_t *index)
{
	const uint8_t *buf = ptr;
	if (index)
		*index += 3;

	return (uint32_t) (buf[0] << 16) | (buf[1] << 8) | buf[2];
}

int32_t read_LE24s(const void *ptr, size_t *index)
{
	const uint8_t *buf = ptr;
	int32_t u32;

	u32 = read_LE24(buf, index);
	if (u32 >> 23)			// if negative
		u32 |= 0xFF000000LL;	// pad with ones

	return (int32_t) u32;
}

int32_t read_BE24s(const void *ptr, size_t *index)
{
	const uint8_t *buf = ptr;
	int32_t u32;

	u32 = read_BE24(buf, index);
	if (u32 >> 23)			// if negative
		u32 |= 0xFF000000LL;	// pad with ones

	return (int32_t) u32;
}

uint32_t read_LE32(const void *ptr, size_t *index)
{
	const uint8_t *buf = ptr;
	if (index)
		*index += sizeof(uint32_t);

#ifdef ASS_LE
	return *((uint32_t *) buf);
#else
	return (uint32_t) (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
#endif
}

uint32_t read_BE32(const void *ptr, size_t *index)
{
	const uint8_t *buf = ptr;
	if (index)
		*index += sizeof(uint32_t);

	return (uint32_t) (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

uint64_t read_LE64(const void *ptr, size_t *index)
{
	const uint8_t *buf = ptr;
	if (index)
		*index += sizeof(uint64_t);

#ifdef ASS_LE
	return *((uint64_t *) buf);
#else
	return ((uint64_t) read_LE32(&buf[4], NULL) << 32) | read_LE32(buf, NULL);
#endif
}

uint64_t read_BE64(const void *ptr, size_t *index)
{
	const uint8_t *buf = ptr;
	if (index)
		*index += sizeof(uint64_t);

	return ((uint64_t) read_BE32(buf, NULL) << 32) | read_BE32(&buf[4], NULL);
}

// Print to buffer
void print_LE16(uint8_t *buf, uint16_t data)
{
	for (int i=0; i < sizeof(data); i++)
		buf[i] = data >> (i<<3);
}

void print_LE24(uint8_t *buf, uint32_t data)
{
	for (int i=0; i < 3; i++)
		buf[i] = data >> (i<<3);
}

void print_LE32(uint8_t *buf, uint32_t data)
{
	for (int i=0; i < sizeof(data); i++)
		buf[i] = data >> (i<<3);
}

void print_LE64(uint8_t *buf, uint64_t data)
{
	for (int i=0; i < sizeof(data); i++)
		buf[i] = data >> (i<<3);
}

void print_BE16(uint8_t *buf, uint16_t data)
{
	for (int i=0; i < 2; i++)
		buf[1-i] = data >> (i<<3);
}

void print_BE24(uint8_t *buf, uint32_t data)
{
	for (int i=0; i < 3; i++)
		buf[2-i] = data >> (i<<3);
}

void print_BE32(uint8_t *buf, uint32_t data)
{
	for (int i=0; i < 4; i++)
		buf[3-i] = data >> (i<<3);
}

void print_BE64(uint8_t *buf, uint64_t data)
{
	for (int i=0; i < 8; i++)
		buf[7-i] = data >> (i<<3);
}
