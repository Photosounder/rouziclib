typedef size_t (*fread_func_t) (void *, size_t, size_t, void *);
typedef size_t (*fwrite_func_t)(const void *, size_t, size_t, void *);
typedef void * (*fopen_func_t) (const char *, const char *);
typedef int    (*fclose_func_t)(void *);

extern _Thread_local fread_func_t  fread_override;
extern _Thread_local fwrite_func_t fwrite_override;
extern _Thread_local fopen_func_t  fopen_override;
extern _Thread_local fclose_func_t fclose_override;

extern size_t fread_buffer(void *ptr, size_t size, size_t nmemb, void *stream);
extern size_t fwrite_buffer(const void *ptr, size_t size, size_t nmemb, void *stream);
extern void *fopen_buffer(const char *path, const char *mode);
extern int fclose_buffer(void *stream);

extern void io_override_set_FILE();
extern void io_override_set_buffer();
