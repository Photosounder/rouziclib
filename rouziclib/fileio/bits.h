#define get_bit(word, pos)	(((word) >> (pos)) & 1)

extern int get_bit_32(const uint32_t word, const int pos);
extern uint64_t get_bits_in_stream(uint8_t *stream, int64_t start_bit, int bit_count);
