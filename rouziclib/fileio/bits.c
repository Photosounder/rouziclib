int get_bit_32(const uint32_t word, const int pos)
{
	return (word >> pos) & 1;
}

uint64_t get_bits_in_stream(uint8_t *stream, int64_t start_bit, int bit_count)
{
	uint64_t r = 0, b;
	int64_t start_byte, actual_start_bit;
	int bits_to_read, b_sh;
	uint8_t mask;

	start_byte = start_bit >> 3;
	start_bit &= 7;
	bits_to_read = MINN(8-start_bit, bit_count);
	b_sh = MAXN(0, bit_count - bits_to_read);

	while (bit_count > 0)	// AAAaaaBB BbbbCCCc logic
	{
		bits_to_read = MINN(8-start_bit, bit_count);			// how many bits to read in this byte
		actual_start_bit = 8-start_bit - bits_to_read;
		mask = (((1<<bits_to_read)-1) << actual_start_bit);
		b = (stream[start_byte] & mask) >> actual_start_bit;
		r |= b << b_sh;

		b_sh = MAXN(0, b_sh-8);
		bit_count -= bits_to_read;				// decrement count of bits left to write

		start_bit = 0;
		start_byte++;
	}

	return r;
}

uint64_t get_bits_in_stream_inc(uint8_t *stream, int64_t *start_bit, int bit_count)
{
	uint64_t r = get_bits_in_stream(stream, *start_bit, bit_count);
	*start_bit += bit_count;
	return r;
}

void set_bits_in_stream(uint8_t *stream, int64_t start_bit, int bit_count, uint64_t b)
{
	int64_t start_byte, actual_start_bit;
	int bits_to_write, b_sh;
	uint8_t mask;

	start_byte = start_bit >> 3;
	start_bit &= 7;
	bits_to_write = MINN(8-start_bit, bit_count);
	b_sh = MAXN(0, bit_count - bits_to_write);

	/*while (bit_count > 0)	// bbAAAaaa CcccBBBb logic
	{
		bits_to_write = MINN(8-start_bit, bit_count);		// how many bits to write in this byte
		mask = (((1<<bits_to_write)-1) << start_bit);
		stream[start_byte] &= ~mask;				// mask bits to write
		stream[start_byte] |= (b << start_bit) & mask;		// add bits to write

		b >>= bits_to_write;					// remove bits just written
		bit_count -= bits_to_write;				// decrement count of bits left to write

		start_bit = 0;
		start_byte++;
	}*/

	while (bit_count > 0)	// AAAaaaBB BbbbCCCc logic
	{
		bits_to_write = MINN(8-start_bit, bit_count);			// how many bits to write in this byte
		actual_start_bit = 8-start_bit - bits_to_write;
		mask = (((1<<bits_to_write)-1) << actual_start_bit);
		stream[start_byte] &= ~mask;					// mask bits to write
		stream[start_byte] |= ((b>>b_sh) << actual_start_bit) & mask;	// add bits to write

		b_sh = MAXN(0, b_sh-8);
		bit_count -= bits_to_write;				// decrement count of bits left to write

		start_bit = 0;
		start_byte++;
	}
}

void set_bits_in_stream_inc(uint8_t *stream, int64_t *start_bit, int bit_count, uint64_t b)
{
	set_bits_in_stream(stream, *start_bit, bit_count, b);
	*start_bit += bit_count;
}

uint32_t reverse_bits32(uint32_t v)	// from http://graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
{
	v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
	v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
	v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
	v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
	return (v >> 16           ) | ( v              << 16);
}

uint32_t reverse_n_bits32(uint32_t v, int n)
{
	return reverse_bits32(v) >> (32-n);
}
