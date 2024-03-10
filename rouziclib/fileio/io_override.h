typedef size_t   (*fread_func_t)  (void *, size_t, size_t, void *);
typedef size_t   (*fwrite_func_t) (const void *, size_t, size_t, void *);
typedef int      (*fprintf_func_t)(void *, const char *, ...);
typedef void *   (*fopen_func_t)  (const char *, const char *);
typedef int      (*fclose_func_t) (void *);
typedef void     (*rewind_func_t) (void *);
typedef int      (*fseek_func_t)(void *, long int, int);
typedef long int (*ftell_func_t)(void *);

extern _Thread_local fread_func_t   fread_override;
extern _Thread_local fwrite_func_t  fwrite_override;
extern _Thread_local fprintf_func_t fprintf_override;
extern _Thread_local fopen_func_t   fopen_override;
extern _Thread_local fclose_func_t  fclose_override;
extern _Thread_local rewind_func_t  rewind_override;
extern _Thread_local fseek_func_t  fseek_override;
extern _Thread_local ftell_func_t  ftell_override;

extern size_t fread_buffer(void *ptr, size_t size, size_t nmemb, void *stream);
extern size_t fwrite_buffer(const void *ptr, size_t size, size_t nmemb, void *stream);
extern void *fopen_buffer(const char *path, const char *mode);
extern int fclose_buffer(void *stream);
extern void rewind_buffer(void *stream);
extern int fseek_buffer(void *stream, long int offset, int whence);
extern long int ftell_buffer(void *stream);

extern void io_override_set_FILE();
extern void io_override_set_buffer();
