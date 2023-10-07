#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <emscripten/html5.h>

extern int em_get_memory_size();
extern void em_disable_context_menu();
__attribute__((import_name("em_mmb_capture"))) extern void em_mmb_capture();
__attribute__((import_name("em_capture_cursor"))) extern void em_capture_cursor();
__attribute__((import_name("em_release_cursor"))) extern void em_release_cursor();
extern void em_sync_by_mutex(int lock);
__attribute__((import_name("em_browser_toggle_fullscreen"))) extern void em_browser_toggle_fullscreen(int state);
extern void em_fit_canvas_to_innerwindow();
extern void em_enumerate_av_devices();
extern char *em_string_registry(int id, char *str);

#endif
