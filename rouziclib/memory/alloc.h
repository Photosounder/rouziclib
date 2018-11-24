extern size_t alloc_enough2(void **buffer, size_t needed_count, size_t alloc_count, size_t size_elem, double inc_ratio);
extern void free_null(void **ptr);
extern void **calloc_2d(const size_t ptr_count, const size_t size_buffers, const size_t size_elem);
extern void **memset_2d(void **ptr, const int word, const size_t size, const size_t count);
extern void free_2d(void **ptr, const size_t count);
extern void *copy_alloc(void *b0, size_t size);
extern size_t next_aligned_offset(size_t offset, const size_t size_elem);

// alloc_count was originally a pointer, however callers sent a mix of int and size_t which was a problem
#define alloc_enough(b, nc, acp, se, ir)	(*acp) = alloc_enough2(b, nc, (*acp), se, ir)
