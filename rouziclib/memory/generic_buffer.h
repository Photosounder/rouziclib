// in general/struct.h:
// buffer_t

extern char *bufprintf(buffer_t *s, const char *format, ...);
extern char *bufnprintf(buffer_t *s, size_t n, const char *format, ...);
extern char *bufwrite(buffer_t *s, uint8_t *ptr, size_t size);
extern void free_buf(buffer_t *s);
extern void bufprint_gmtime(buffer_t *s, time_t t);
