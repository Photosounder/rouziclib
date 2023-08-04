extern void *dynlib_open(const char *path);
extern void *dynlib_find_symbol(void *module, const char *symbol_name);
extern void dynlib_close(void **module);
