extern int circ_index(int index, const int size);
extern void *memset_circular(void *s, int c, int esize, int num, int pos, int limit);
extern void *memcpy_circular(void *dest, void *src, int esize, int num, int pos, int limit);
