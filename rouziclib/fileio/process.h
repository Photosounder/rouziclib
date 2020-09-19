#ifdef _WIN32
extern PROCESS_INFORMATION create_process_direct(const char *cmd, DWORD flags);
extern PROCESS_INFORMATION create_process_flags(const char *cmd, DWORD flags);
extern void wait_process_end(PROCESS_INFORMATION *procinf);
extern void send_SIGINT(HANDLE hProcess);

#define create_process(cmd)	create_process_flags(cmd, 0)
#else

#define create_process(cmd)	fprintf_rl(stderr, "create_process() not implemented on this platform\n")

#endif

extern char **get_argv(int *argc);
