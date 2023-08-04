#ifndef __wasm__

#ifndef _WIN32
#include <dlfcn.h>
#endif

void *dynlib_open(const char *path)
{
#ifdef _WIN32
	wchar_t *wpath = utf8_to_wchar(path, NULL);
	if (wpath==NULL)
		return NULL;

	SetErrorMode(SEM_FAILCRITICALERRORS);
	void *module = LoadLibraryW(wpath);
	SetErrorMode(0);

	free(wpath);
	return module;
#else
	return dlopen(path, RTLD_LOCAL);
#endif
}

void *dynlib_find_symbol(void *module, const char *symbol_name)
{
#ifdef _WIN32
	return GetProcAddress(module, symbol_name);
#else
	return dlsym(module, symbol_name);
#endif
}

void dynlib_close(void **module)
{
#ifdef _WIN32
	FreeLibrary(*module);
#else
	dlclose(*module);
#endif

	*module = NULL;
}

#endif
