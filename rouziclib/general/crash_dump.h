#ifdef RL_CRASHDUMP
#ifdef _WIN32
#include <DbgHelp.h>

#ifdef _MSC_VER
#pragma comment (lib, "DbgHelp.lib")
#endif

extern wchar_t *crashdump_make_dump_path();
extern void crashdump_write_minidump(EXCEPTION_POINTERS *exception_ptr);
extern LONG WINAPI crashdump_callback(EXCEPTION_POINTERS *exception_ptr);
extern void crashdump_init(const char *path);

#endif
#endif
