int get_bit_32(const uint32_t word, const int pos)
{
	return (word >> pos) & 1;
}

uint64_t get_bits_in_stream(uint8_t *stream, int64_t start_bit, int bit_count)
{
	uint64_t r = 0, b;
	int64_t start_byte;
	int bits_to_read;

	start_byte = start_bit >> 3;
	start_bit &= 7;

	while (bit_count > 0)
	{
		bits_to_read = MINN(8-start_bit, bit_count);	// how many bits to read in this byte
		b = stream[start_byte];				// read byte
		if (bit_count < 8)
			b >>= 8 - bits_to_read;			// shift the final byte
		if (start_bit > 0)
			b &= 0xFF >> start_bit;			// mask the top bits of the first byte

		bit_count -= bits_to_read;			// decrement count of bits left to read
		r |= b << bit_count;				// add to r in the right place

		start_bit = 0;
		start_byte++;
	}

	return r;
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
