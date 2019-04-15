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
