#ifdef RL_SDL

#ifdef _MSC_VER
#pragma comment (lib, "SDL2.lib")
#pragma comment (lib, "SDL2main.lib")
#endif

#include <SDL.h>
#include <SDL_audio.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <SDL_opengl.h>

extern SDL_Rect make_sdl_rect(int x, int y, int w, int h);
extern SDL_Rect recti_to_sdl_rect(recti_t ri);
extern int sdl_get_window_hz(SDL_Window *window);
extern int sdl_vsync_sleep(SDL_Window *window, uint32_t time_last_vsync);
extern recti_t sdl_get_window_rect(SDL_Window *window);
extern recti_t sdl_get_display_rect(int display_id);
extern recti_t sdl_get_display_usable_rect(int display_id);
extern recti_t sdl_screen_max_window_rect();
extern xyi_t sdl_screen_max_window_size();
extern recti_t sdl_get_window_border(SDL_Window *window);
extern void mouse_event_proc(mouse_t *mouse, SDL_Event event, zoom_t *zc);
extern int get_sdl_renderer_index(const char *name);
extern int get_sdl_opengl_renderer_index();
extern SDL_GLContext init_sdl_gl(SDL_Window *window);
extern void sdl_graphics_init_full(raster_t *fb, const char *window_name, xyi_t dim, xyi_t pos, int flags);
extern void sdl_graphics_init_autosize(raster_t *fb, const char *window_name, int flags, int window_index);

#endif
