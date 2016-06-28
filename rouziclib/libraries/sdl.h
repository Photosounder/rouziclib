#ifdef RL_SDL

#ifdef _MSC_VER
#pragma comment (lib, "SDL2.lib")
#pragma comment (lib, "SDL2main.lib")
#endif

#include <SDL.h>
#include <SDL_audio.h>
#include <gl/glew.h>
#include <SDL_opengl.h>
#include <gl/glut.h>

extern int sdl_get_window_hz(SDL_Window *window);
extern int sdl_vsync_sleep(SDL_Window *window, uint32_t time_last_vsync);

#endif
