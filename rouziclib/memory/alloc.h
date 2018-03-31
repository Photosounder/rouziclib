extern void alloc_enough(void **buffer, int needed_count, int *alloc_count, int size_elem, double inc_ratio);
extern void free_null(void **ptr);
extern void free_2d(void **ptr, const int count);
extern void *copy_alloc(void *b0, size_t size);
extern size_t next_aligned_offset(size_t offset, const int size_elem);
