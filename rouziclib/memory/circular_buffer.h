extern int32_t circ_index(int32_t index, int32_t size);
extern void *memset_circular(void *s, int c, int32_t esize, int32_t num, int32_t pos, int32_t limit);
extern void *memcpy_circular(void *dest, void *src, int32_t esize, int32_t num, int32_t pos, int32_t limit);
