#ifdef RL_SDL

#if RL_SDL == 3
//#include <SDL3/SDL_syswm.h>
#else
#include <SDL2/SDL_syswm.h>
#endif

#if defined(_WIN32) && RL_SDL == 3 && !defined(RL_SDL3_EXCL_WINMAIN)		// /SUBSYSTEM:WINDOWS workaround
extern int main(int argc, char *argv[]);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
{
	return main(0, NULL);
}
#endif

SDL_Rect make_sdl_rect(int x, int y, int w, int h)
{
	SDL_Rect rs;

	rs.x = x;
	rs.y = y;
	rs.w = w;
	rs.h = h;

	return rs;
}

SDL_Rect recti_to_sdl_rect(recti_t ri)
{
	ri = sort_recti(ri);
	return make_sdl_rect(ri.p0.x, ri.p0.y, ri.p1.x - ri.p0.x + 1, ri.p1.y - ri.p0.y + 1);
}

// Display/window
double sdl_get_window_hz(SDL_Window *window)
{
#if RL_SDL == 3
	const SDL_DisplayMode *mode = SDL_GetWindowFullscreenMode(window);

	if (mode == NULL)
		return -1;
	return mode->refresh_rate;
#else
	SDL_DisplayMode mode;

	if (SDL_GetWindowDisplayMode(window, &mode) < 0)
		fprintf_rl(stderr, "SDL_GetWindowDisplayMode failed: %s\n", SDL_GetError());

	return mode.refresh_rate;
#endif
}

recti_t sdl_get_window_rect(SDL_Window *window)
{
	recti_t r;

	SDL_GetWindowSize(window, &r.p1.x, &r.p1.y);
	SDL_GetWindowPosition(window, &r.p0.x, &r.p0.y);
	r.p1 = sub_xyi(add_xyi(r.p0, r.p1), set_xyi(1));

	return r;
}

void sdl_set_window_rect(SDL_Window *window, recti_t r)
{
	SDL_SetWindowSize(window, get_recti_dim(r).x, get_recti_dim(r).y);
	SDL_SetWindowPosition(window, r.p0.x, r.p0.y);
}

xyi_t sdl_get_display_dim(int display_id)
{
#if RL_SDL == 3
	int num_displays;
	SDL_DisplayID *displays = SDL_GetDisplays(&num_displays);	
	if (display_id < num_displays)
	{
		const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(displays[display_id]);

		if (mode)
			return xyi(mode->w, mode->h);
	}

	fprintf_rl(stderr, "SDL_GetCurrentDisplayMode failed: %s\n", SDL_GetError());
#else
	SDL_DisplayMode mode={0};

	if (SDL_GetCurrentDisplayMode(display_id, &mode)==0)
		return xyi(mode.w, mode.h);
#endif

	return xyi(-1, -1);
}

int sdl_get_display_count()
{
#if RL_SDL == 3
	int count;
	SDL_GetDisplays(&count);
	return count;
#else
	return SDL_GetNumVideoDisplays();
#endif
}

recti_t sdl_get_display_rect(int display_id)
{
	SDL_Rect r;

#if RL_SDL == 2
	if (display_id < sdl_get_display_count())
		if (SDL_GetDisplayBounds(display_id, &r)==0)
		{
#else
	int num_displays;
	SDL_DisplayID *displays = SDL_GetDisplays(&num_displays);	
	if (display_id < num_displays)
		if (SDL_GetDisplayBounds(displays[display_id], &r))
		{
			SDL_free(displays);
#endif
			return recti( xyi(r.x, r.y), xyi(r.x+r.w-1, r.y+r.h-1));
		}

	return recti(xyi(0,0), xyi(0,0));
}

recti_t sdl_get_display_usable_rect(int display_id)
{
	SDL_Rect r;

#if RL_SDL == 2
	if (display_id < sdl_get_display_count())
		if (SDL_GetDisplayUsableBounds(display_id, &r)==0)
		{
#else
	int num_displays;
	SDL_DisplayID *displays = SDL_GetDisplays(&num_displays);	
	if (display_id < num_displays)
		if (SDL_GetDisplayUsableBounds(displays[display_id], &r))
		{
			SDL_free(displays);
#endif
			return recti( xyi(r.x, r.y), xyi(r.x+r.w-1, r.y+r.h-1));
		}

	return recti(XYI0, XYI0);
}

recti_t sdl_screen_max_window_rect()
{
	int i;
	recti_t dr, wr=recti(xyi(0,0), xyi(0,0));

	for (i=0; i < sdl_get_display_count(); i++)
	{
		dr = sdl_get_display_rect(i);
		wr.p0 = min_xyi(wr.p0, dr.p0);
		wr.p1 = max_xyi(wr.p1, dr.p1);
	}

	return wr;
}

xyi_t sdl_screen_max_window_size()
{
	return get_recti_dim(sdl_get_display_rect(0));
	//return get_recti_dim(sdl_screen_max_window_rect());
}

recti_t sdl_get_window_border(SDL_Window *window)
{
	recti_t r;

	SDL_GetWindowBordersSize(window, &r.p0.y, &r.p0.x, &r.p1.y, &r.p1.x);

	return r;
}

int sdl_find_point_within_display(xyi_t p)
{
	int i;

	for (i=0; i < sdl_get_display_count(); i++)
		if (check_point_within_box_int(p, sdl_get_display_rect(i)))
			return i;

	return -1;
}

int sdl_get_window_cur_display()	// determined by the max pixel area in a display
{
	int i, area, max_area=0, display_index=0;

	for (i=0; i < sdl_get_display_count(); i++)
	{
		area = mul_x_by_y_xyi(get_recti_dim(recti_intersection(sdl_get_window_rect(fb->window), sdl_get_display_rect(i))));

		if (max_area < area)
		{
			max_area = area;
			display_index = i;
		}
	}

	return display_index;
}

int sdl_is_window_maximised(SDL_Window *window)
{
	return SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED;
}

#ifdef _WIN32
HWND sdl_get_window_hwnd(SDL_Window *window)
{
#if RL_SDL == 3
	return (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
#else
	SDL_SysWMinfo wmInfo;

	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);

	return wmInfo.info.win.window;
#endif
}
#endif

// Mouse/keyboard input
void sdl_update_mouse(SDL_Window *window, mouse_t *mouse)	// gives the mouse position wrt the window coordinates, even outside the window
{
#if RL_SDL == 3
	float mpos_x, mpos_y;
#else
	int mpos_x, mpos_y;
#endif
	xy_t mpos;
	rect_t wr;
	int inside_window, but_state;

	but_state = SDL_GetGlobalMouseState(&mpos_x, &mpos_y);	// 1 = lmb, 2 = mmb, 4 = rmb
	mpos = xy(mpos_x, mpos_y);
	wr = recti_to_rect(sdl_get_window_rect(fb->window));

	#ifdef __EMSCRIPTEN__					// emscripten doesn't have SDL_GetGlobalMouseState
	but_state = SDL_GetMouseState(&mpos_x, &mpos_y);
	#endif

	mpos = xy(mpos_x, mpos_y);
	mpos = sub_xy(mpos, wr.p0);
	wr.p1 = sub_xy(wr.p1, wr.p0);
	wr.p0 = XY0;

	inside_window = check_point_within_box(mpos, wr);

	if (inside_window && mouse->mouse_focus_flag <= 0)	// if the mouse just entered the window
		mouse->mouse_focus_flag = 2;

	if (inside_window==0 && mouse->mouse_focus_flag >= 0)	// if the mouse just left the window
		mouse->mouse_focus_flag = -2;

	if (mouse->warp_if_move && is0_xy(mouse->d)==0)
	{
		mouse->showcursor = -1;
		mouse->warp = 1;
	}

	if (mouse->warp==0)
		mouse->a = div_xy(mpos, set_xy(fb->pixel_scale));

	if (mouse->mouse_focus_flag <= 0)	// only process global buttons if the mouse is outside the window
	{
		mouse_button_update(&mouse->b.lmb, &mouse->b.lmf, get_bit(but_state, 0), 0, mouse);
		mouse_button_update(&mouse->b.rmb, &mouse->b.rmf, get_bit(but_state, 2), 2, mouse);
	}
	mouse_button_update(&mouse->b.mmb, &mouse->b.mmf, get_bit(but_state, 1), 1, mouse);
}

void sdl_mouse_event_proc(mouse_t *mouse, SDL_Event event, zoom_t *zc)
{
	SDL_Keymod mod_state;

	if (event.type==SDL_DROPBEGIN)
		SDL_RaiseWindow(fb->window);

#if RL_SDL == 3
	if (event.type==SDL_WINDOWEVENT_FOCUS_GAINED)
		mouse->window_focus_flag = 2;

	if (event.type==SDL_WINDOWEVENT_FOCUS_LOST)
		mouse->window_focus_flag = -2;

	if (event.type==SDL_WINDOWEVENT_MINIMIZED)
		mouse->window_minimised_flag = 2;

	if (event.type==SDL_WINDOWEVENT_RESTORED)
		mouse->window_minimised_flag = -2;
#else
	if (event.type==SDL_WINDOWEVENT)
	{
		// SDL bug: SDL_WINDOWEVENT_ENTER and SDL_WINDOWEVENT_LEAVE aren't always triggered when holding the mouse button down
		/*if (event.window.event==SDL_WINDOWEVENT_ENTER)
			mouse->mouse_focus_flag = 2;

		if (event.window.event==SDL_WINDOWEVENT_LEAVE)
			mouse->mouse_focus_flag = -2;*/

		if (event.window.event==SDL_WINDOWEVENT_FOCUS_GAINED)
			mouse->window_focus_flag = 2;

		if (event.window.event==SDL_WINDOWEVENT_FOCUS_LOST)
			mouse->window_focus_flag = -2;

		if (event.window.event==SDL_WINDOWEVENT_MINIMIZED)
			mouse->window_minimised_flag = 2;

		if (event.window.event==SDL_WINDOWEVENT_RESTORED)
			mouse->window_minimised_flag = -2;
	}
#endif

	if (event.type==SDL_MOUSEMOTION)
	{
		if (mouse->discard_warp_first_move==0 || abs(event.motion.xrel)+abs(event.motion.yrel) < 40)
			mouse->d = add_xy(mouse->d, xy(event.motion.xrel, event.motion.yrel));			// only works when the cursor is inside the window or with mouse.warp
		mouse->discard_warp_first_move = 0;
	}

	// Mouse button events
	if (event.type==SDL_MOUSEBUTTONDOWN)
	{
		if (event.button.button==SDL_BUTTON_LEFT)
			mouse_button_event(&mouse->b.lmb, &mouse->b.lmf, 1);

		if (event.button.button==SDL_BUTTON_RIGHT)
			mouse_button_event(&mouse->b.rmb, &mouse->b.rmf, 1);

		if (event.button.button==SDL_BUTTON_MIDDLE)
			mouse_button_event(&mouse->b.mmb, &mouse->b.mmf, 1);

		if (event.button.clicks)
			mouse->b.clicks = event.button.clicks;
	}

	if (event.type==SDL_MOUSEBUTTONUP)
	{
		if (event.button.button==SDL_BUTTON_LEFT)
			mouse_button_event(&mouse->b.lmb, &mouse->b.lmf, -1);

		if (event.button.button==SDL_BUTTON_RIGHT)
			mouse_button_event(&mouse->b.rmb, &mouse->b.rmf, -1);

		if (event.button.button==SDL_BUTTON_MIDDLE)
			mouse_button_event(&mouse->b.mmb, &mouse->b.mmf, -1);
	}

	if (event.type==SDL_MOUSEWHEEL)
	{
		#ifdef __EMSCRIPTEN__
		event.wheel.y /= abs(event.wheel.y);		// emscripten reports crazy values like 3 or 100 instead of 1, that fixes it
		#endif
		mouse->b.wheel += event.wheel.y;

		zoom_wheel(zc, mouse->zoom_flag, event.wheel.y);
	}

	// state of modifier keys
	mod_state = SDL_GetModState();

	mouse->mod_key[mouse_mod_ctrl] = (mod_state & KMOD_CTRL) != 0;
	mouse->mod_key[mouse_mod_alt] = (mod_state & KMOD_ALT) != 0;
	mouse->mod_key[mouse_mod_shift] = (mod_state & KMOD_SHIFT) != 0;
	mouse->mod_key[mouse_mod_gui] = (mod_state & KMOD_GUI) != 0;
}

void sdl_keyboard_event_proc(mouse_t *mouse, SDL_Event event)
{
#if RL_SDL == 2
	int scancode = event.key.keysym.scancode;
#else
	int scancode = event.key.scancode;
#endif

	if (event.type == SDL_KEYDOWN)
		keyboard_button_event(&mouse->key_state[scancode], &mouse->key_quick[scancode], 1, event.key.repeat);

	if (event.type == SDL_KEYUP)
		keyboard_button_event(&mouse->key_state[scancode], &mouse->key_quick[scancode], -1, event.key.repeat);

	if (event.type == SDL_TEXTINPUT)
		textedit_add(cur_textedit, event.text.text);
}

void sdl_set_mouse_pos_screen(xy_t pos)
{
#if RL_SDL == 3
	SDL_WarpMouseInWindow(NULL, pos.x*fb->pixel_scale, pos.y*fb->pixel_scale);
#else
	SDL_WarpMouseInWindow(NULL, nearbyint(pos.x*fb->pixel_scale), nearbyint(pos.y*fb->pixel_scale));
#endif
}

void sdl_set_mouse_pos_world(xy_t world_pos)
{
	sdl_set_mouse_pos_screen(sc_xy(world_pos));
	mouse.u = world_pos;
}

// Window/graphics

#if RL_SDL == 2
int get_sdl_renderer_index(const char *name)
{
	int i, n;
	SDL_RendererInfo info;

	n = SDL_GetNumRenderDrivers();

	for (i=0; i<n; i++)
	{
		SDL_GetRenderDriverInfo(i, &info);

		if (strcmp(name, info.name)==0)
			return i;
	}

	return -1;
}

int get_sdl_opengl_renderer_index()
{
	int index;

	index = get_sdl_renderer_index("opengl");

	if (index==-1)
	{
		index = get_sdl_renderer_index("opengles2");

		if (index==-1)
			fprintf_rl(stderr, "OpenGL renderer not found, fell back to default (in get_sdl_opengl_renderer_index())\n");
		else
			fprintf_rl(stderr, "OpenGL renderer not found, fell back to OpenGLES2 (in get_sdl_opengl_renderer_index())\n");
	}

	return index;
}
#endif

double get_sdl_window_screen_refresh_rate(SDL_Window *window)
{
#if RL_SDL == 3
	const SDL_DisplayMode *mode = SDL_GetDesktopDisplayMode(SDL_GetDisplayForWindow(window));
	if (mode)
		return (double) mode->refresh_rate_numerator / (double) mode->refresh_rate_denominator;
#endif
	return NAN;
}

SDL_GLContext init_sdl_gl(SDL_Window *window)
{
	SDL_GLContext ctx=NULL;
#ifdef RL_OPENCL_GL
	GLenum glewError;

	ctx = SDL_GL_CreateContext(window);		// Create context
	if (ctx==NULL)
	{
		fprintf_rl(stderr, "OpenGL context could not be created. SDL Error: %s\n", SDL_GetError());
		return ctx;
	}

	// initialize GLEW
	glewExperimental = 1; 
	glewError = glewInit();
	if (glewError != GLEW_OK)
		fprintf_rl(stderr, "Error initializing GLEW. %s\n", glewGetErrorString(glewError));

#endif
	return ctx;
}

/*// Things required by SetProcessDpiAwareness
#ifdef _WIN32
#include <shellscalingapi.h>
#ifdef _MSC_VER
#pragma comment (lib, "Shcore.lib")
#endif
#endif*/

static uint32_t sdl_graphics_texture_format = SDL_PIXELFORMAT_ARGB8888;
static int sdl_graphics_srgb_order = ORDER_BGRA;
static int sdl_graphics_window_is_foreign = 0;
static uint64_t sdl_graphics_window_flags = 0;

static void sdl_init()
{
	static int initialised=0;

	if (initialised)
		return;

	// Initialise every SDL subsystem used by rouziclib once
	initialised = 1;
#if RL_SDL == 2
	// Tell SDL that rouziclib provides the process entry point
	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO))
#else
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
#endif
		fprintf_rl(stderr, "SDL_Init failed: %s\n", SDL_GetError());

	#ifdef __EMSCRIPTEN__
	// Capture middle mouse button events correctly in the browser
	em_mmb_capture();
	#endif
}

enum sdl_graphics_window_api
{
	SDL_GRAPHICS_WINDOW_API_SDL,
	SDL_GRAPHICS_WINDOW_API_OPENGL,
	SDL_GRAPHICS_WINDOW_API_VULKAN
};

static int sdl_graphics_window_api_for_mode(int mode)
{
	#if defined(_WIN32) && defined(RL_OPENCL_GL) && defined(RL_VULKAN)
	// Share one OpenGL-capable native window across every Windows backend
	(void) mode;
	return SDL_GRAPHICS_WINDOW_API_OPENGL;
	#else
	#ifdef RL_OPENCL_GL
	// OpenCL interop requires a window created for OpenGL
	if (mode==DRAWQ_MODE_OPENCL)
		return SDL_GRAPHICS_WINDOW_API_OPENGL;
	#endif

	#ifdef RL_VULKAN
	// Vulkan presentation requires a window created for Vulkan
	if (mode==DRAWQ_MODE_VULKAN)
		return SDL_GRAPHICS_WINDOW_API_VULKAN;
	#endif

	return SDL_GRAPHICS_WINDOW_API_SDL;
	#endif
}

static uint64_t sdl_graphics_window_flags_for_mode(uint64_t flags, int mode)
{
	// Remove presentation flags before selecting the requested API
	flags &= ~(uint64_t) SDL_WINDOW_OPENGL;
	#ifdef RL_VULKAN
	flags &= ~(uint64_t) SDL_WINDOW_VULKAN;
	#endif

	#if defined(_WIN32) && defined(RL_OPENCL_GL) && defined(RL_VULKAN)
	// Keep the persistent Windows window compatible with OpenGL interop
	(void) mode;
	flags |= SDL_WINDOW_OPENGL;
	#else
	#ifdef RL_OPENCL_GL
	// Select OpenGL only for the OpenCL interop path
	if (mode==DRAWQ_MODE_OPENCL)
		flags |= SDL_WINDOW_OPENGL;
	#endif

	#ifdef RL_VULKAN
	// Select Vulkan only for the Vulkan backend
	if (mode==DRAWQ_MODE_VULKAN)
		flags |= SDL_WINDOW_VULKAN;
	#endif
	#endif

	return flags;
}
static int sdl_graphics_mode_available(int mode)
{
	// Direct drawing is always available
	if (mode==DRAWQ_MODE_DIRECT)
		return 1;

	#ifdef RL_OPENCL
	// Probe the OpenCL platform before offering its backend
	if (mode==DRAWQ_MODE_OPENCL)
		return check_opencl();
	#endif

	// Check the instruction sets required by the software draw queue
	if (mode==DRAWQ_MODE_SOFTWARE)
		return check_ssse3() && check_sse41();

	#ifdef RL_VULKAN
	// Defer Vulkan device validation until the SDL surface exists
	if (mode==DRAWQ_MODE_VULKAN)
		return 1;
	#endif

	return 0;
}

static int sdl_graphics_fallback_mode(int mode)
{
	// Prefer OpenCL when Vulkan is unavailable
	if (mode==DRAWQ_MODE_VULKAN)
		return DRAWQ_MODE_OPENCL;

	// Fall back from either hardware queue to software
	if (mode==DRAWQ_MODE_OPENCL)
		return DRAWQ_MODE_SOFTWARE;

	return DRAWQ_MODE_DIRECT;
}

static void sdl_graphics_set_maxdim()
{
	// Set the largest framebuffer allocation needed across all displays
	fb->maxdim = sdl_screen_max_window_size();
	if (fb->use_drawq==DRAWQ_MODE_OPENCL || fb->use_drawq==DRAWQ_MODE_VULKAN)
		fb->maxdim = and_xyi(add_xyi(fb->maxdim, set_xyi(31)), 0xFFFFFFE0);
}
static int sdl_graphics_recreate_window(int mode)
{
	SDL_Window *old_window, *new_window;
	recti_t old_rect;
	uint64_t old_flags, new_flags;
	const char *old_title;
	char *title;
	int was_hidden, was_maximised, was_minimised;

	// Reject cross-API switches for caller-owned native windows
	if (sdl_graphics_window_is_foreign)
	{
		fprintf_rl(stderr, "sdl_graphics_recreate_window(): cannot recreate a foreign window for draw queue mode %d\n", mode);
		return 0;
	}

	// Preserve the visible window state before replacing its backend flags
	old_window = fb->window;
	old_rect = sdl_get_window_rect(old_window);
	old_flags = SDL_GetWindowFlags(old_window);
	was_hidden = !!(old_flags & SDL_WINDOW_HIDDEN);
	was_maximised = !!(old_flags & SDL_WINDOW_MAXIMIZED);
	was_minimised = !!(old_flags & SDL_WINDOW_MINIMIZED);
	old_title = SDL_GetWindowTitle(old_window);

	// Copy the title because SDL owns the original string
	title = calloc(strlen(old_title)+1, 1);
	if (title==NULL)
	{
		fprintf_rl(stderr, "sdl_graphics_recreate_window(): title allocation failed\n");
		return 0;
	}
	strcpy(title, old_title);

	// Select mutually exclusive window flags for the target presentation API
	new_flags = sdl_graphics_window_flags_for_mode(sdl_graphics_window_flags, mode);
	new_flags &= ~((uint64_t) SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED | SDL_WINDOW_MAXIMIZED);
	#if RL_SDL == 2
	new_flags &= ~((uint64_t) SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN);
	#else
	new_flags &= ~(uint64_t) SDL_WINDOW_FULLSCREEN;
	#endif
	new_flags |= SDL_WINDOW_HIDDEN;
	if (was_maximised)
		new_flags |= SDL_WINDOW_MAXIMIZED;
	else if (was_minimised)
		new_flags |= SDL_WINDOW_MINIMIZED;

	// Create the replacement before releasing the current compatible window
	#if RL_SDL == 2
	new_window = SDL_CreateWindow(title, old_rect.p0.x, old_rect.p0.y, get_recti_dim(old_rect).x, get_recti_dim(old_rect).y, (Uint32) new_flags);
	#else
	new_window = SDL_CreateWindow(title, get_recti_dim(old_rect).x, get_recti_dim(old_rect).y, (SDL_WindowFlags) new_flags);
	if (new_window)
		SDL_SetWindowPosition(new_window, old_rect.p0.x, old_rect.p0.y);
	#endif
	free(title);
	if (new_window==NULL)
	{
		fprintf_rl(stderr, "sdl_graphics_recreate_window(): SDL_CreateWindow failed: %s\n", SDL_GetError());
		return 0;
	}

	// Replace the old window after successful creation
	fb->window = new_window;
	SDL_DestroyWindow(old_window);
	SDL_SetWindowMaximumSize(fb->window, fb->maxdim.x, fb->maxdim.y);
	#if RL_SDL == 3
	SDL_StartTextInput(fb->window);
	#endif

	// Restore fullscreen state on the replacement
	#if RL_SDL == 2
	if (old_flags & SDL_WINDOW_FULLSCREEN)
		SDL_SetWindowFullscreen(fb->window, (old_flags & SDL_WINDOW_FULLSCREEN_DESKTOP)==SDL_WINDOW_FULLSCREEN_DESKTOP ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN);
	#else
	if (old_flags & SDL_WINDOW_FULLSCREEN)
		SDL_SetWindowFullscreen(fb->window, 1);
	#endif

	// Restore visibility after all presentation-specific setup is complete
	if (!was_hidden)
		SDL_ShowWindow(fb->window);

	return 1;
}

static int sdl_graphics_create_renderer(int opengl)
{
	// Create the SDL renderer appropriate for the selected presentation path
#if RL_SDL == 3
	fb->renderer = SDL_CreateRenderer(fb->window, opengl ? "opengl" : NULL);
	if (fb->renderer)
		SDL_SetRenderVSync(fb->renderer, SDL_RENDERER_VSYNC_ADAPTIVE);
#else
	fb->renderer = SDL_CreateRenderer(fb->window, opengl ? get_sdl_opengl_renderer_index() : -1, SDL_RENDERER_PRESENTVSYNC);
#endif

	if (fb->renderer==NULL)
	{
		fprintf_rl(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
		return 0;
	}

	return 1;
}

static int sdl_graphics_create_texture()
{
	// Create the streaming texture used by CPU and non-interop rendering
	fb->texture = SDL_CreateTexture(fb->renderer, sdl_graphics_texture_format, SDL_TEXTUREACCESS_STREAMING, fb->w, fb->h);
	if (fb->texture==NULL)
	{
		fprintf_rl(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
		return 0;
	}

	return 1;
}

static int sdl_graphics_init_mode()
{
	int raster_mode = fb->r.use_frgb ? IMAGE_USE_FRGB : IMAGE_USE_LRGB;

	// Initialise direct drawing with a renderer matching the persistent window API
	if (fb->use_drawq==DRAWQ_MODE_DIRECT)
	{
		int opengl = sdl_graphics_window_api_for_mode(fb->use_drawq)==SDL_GRAPHICS_WINDOW_API_OPENGL;
		if (!sdl_graphics_create_renderer(opengl) || !sdl_graphics_create_texture())
			return 0;
		fb->r = make_raster(NULL, XYI0, fb->maxdim, raster_mode);
	}

	#ifdef RL_OPENCL
	// Initialise OpenCL with either GL interop or an SDL streaming texture
	if (fb->use_drawq==DRAWQ_MODE_OPENCL)
	{
		#ifdef RL_OPENCL_GL
		fb->gl_ctx = init_sdl_gl(fb->window);
		if (fb->gl_ctx==NULL || !sdl_graphics_create_renderer(1))
			return 0;
		#else
		if (!sdl_graphics_create_renderer(0) || !sdl_graphics_create_texture())
			return 0;
		#endif

		if (init_fb_cl() != CL_SUCCESS)
			return 0;
	}
	#endif

	// Initialise software presentation with a renderer matching the persistent window API
	if (fb->use_drawq==DRAWQ_MODE_SOFTWARE)
	{
		int opengl = sdl_graphics_window_api_for_mode(fb->use_drawq)==SDL_GRAPHICS_WINDOW_API_OPENGL;
		if (!sdl_graphics_create_renderer(opengl) || !sdl_graphics_create_texture())
			return 0;
	}

	#ifdef RL_VULKAN
	// Initialise Vulkan without creating SDL renderer resources
	if (fb->use_drawq==DRAWQ_MODE_VULKAN)
		if (vk_init() != VK_SUCCESS)
			return 0;
	#endif

	// Reject a backend that was not compiled into this build
	if (!sdl_graphics_mode_available(fb->use_drawq))
		return 0;

	// Allocate shared draw queue state after its backend is ready
	if (fb->use_drawq!=DRAWQ_MODE_DIRECT)
		drawq_alloc();

	// Reset frame state for the newly initialised graphics mode
	fb->srgb_order = sdl_graphics_srgb_order;
	fb->r.dim = xyi(fb->w, fb->h);
	fb->first_frame_done = 0;
	fb->pixel_scale_oldframe = 0;
	fb->tex_lock = 0;
	return 1;
}

static void sdl_graphics_deinit_mode()
{
	int old_mode = fb->use_drawq;

	// Stop software work before releasing its destination texture
	if (old_mode==DRAWQ_MODE_SOFTWARE)
		drawq_soft_quit();

	#ifdef RL_OPENCL
	// Release OpenCL resources only when OpenCL owns them
	if (old_mode==DRAWQ_MODE_OPENCL)
		deinit_fb_cl();
	#endif

	#ifdef RL_VULKAN
	// Quiesce Vulkan while retaining its native-window presentation association
	if (old_mode==DRAWQ_MODE_VULKAN)
		vk_suspend();
	#endif

	// Unlock a streaming texture after all writers have finished
	if (fb->tex_lock && fb->texture)
		SDL_UnlockTexture(fb->texture);
	fb->tex_lock = 0;

	// Detach SDL-owned texture memory before freeing raster-owned buffers
	fb->r.srgb = NULL;

	// Release shared draw queue allocations and threads
	if (old_mode!=DRAWQ_MODE_DIRECT)
		drawq_deinit();

	// Release the framebuffer owned by direct rendering
	if (old_mode==DRAWQ_MODE_DIRECT)
		free_raster(&fb->r);

	// Destroy SDL and OpenGL presentation resources while keeping the window
	SDL_DestroyTexture(fb->texture);
	SDL_DestroyRenderer(fb->renderer);
	fb->texture = NULL;
	fb->renderer = NULL;

	#ifdef RL_OPENCL_GL
	if (fb->gl_ctx)
		SDL_GL_DeleteContext(fb->gl_ctx);
	fb->gl_ctx = NULL;
	#endif

	// Reset frame state that cannot carry across graphics modes
	fb->first_frame_done = 0;
	fb->pixel_scale_oldframe = 0;
}
static int sdl_graphics_init_with_fallback(const char *caller)
{
	int use_frgb = fb->r.use_frgb;
	int failed_mode;

	// Try successively simpler modes until one initialises
	while (fb->use_drawq >= 0 && fb->use_drawq < DRAWQ_MODE_COUNT)
	{
		if (!sdl_graphics_mode_available(fb->use_drawq))
		{
			fprintf_rl(stderr, "%s: draw queue mode %d is unavailable, falling back\n", caller, fb->use_drawq);
			fb->use_drawq = sdl_graphics_fallback_mode(fb->use_drawq);
			continue;
		}

		sdl_graphics_set_maxdim();
		if (sdl_graphics_init_mode())
			return 1;

		// Tear down the failed mode before changing its window presentation API
		failed_mode = fb->use_drawq;
		fprintf_rl(stderr, "%s: draw queue mode %d failed to initialise, falling back\n", caller, failed_mode);
		sdl_graphics_deinit_mode();
		fb->r.use_frgb = use_frgb;
		if (failed_mode==DRAWQ_MODE_DIRECT)
			return 0;
		fb->use_drawq = sdl_graphics_fallback_mode(failed_mode);

		// Recreate an internally owned window when fallback changes presentation API
		if (sdl_graphics_window_api_for_mode(failed_mode) != sdl_graphics_window_api_for_mode(fb->use_drawq))
			if (!sdl_graphics_recreate_window(fb->use_drawq))
				return 0;
	}

	// Recover invalid startup values by trying direct rendering
	fprintf_rl(stderr, "%s: invalid draw queue mode %d, using mode 0\n", caller, fb->use_drawq);
	fb->use_drawq = DRAWQ_MODE_DIRECT;
	sdl_graphics_set_maxdim();
	return sdl_graphics_init_mode();
}

int sdl_graphics_set_drawq_mode(int mode)
{
	int old_mode, use_frgb, api_changed;

	// Reject invalid and unavailable modes without disturbing the current mode
	if (mode < 0 || mode >= DRAWQ_MODE_COUNT)
	{
		fprintf_rl(stderr, "sdl_graphics_set_drawq_mode(): invalid draw queue mode %d\n", mode);
		return 0;
	}
	if (fb->window==NULL)
	{
		fprintf_rl(stderr, "sdl_graphics_set_drawq_mode(): graphics have not been initialised\n");
		return 0;
	}
	if (mode==fb->use_drawq)
		return 1;
	if (!sdl_graphics_mode_available(mode))
	{
		fprintf_rl(stderr, "sdl_graphics_set_drawq_mode(): draw queue mode %d is unavailable\n", mode);
		return 0;
	}

	// Save configuration needed to restore or initialise raster mode
	old_mode = fb->use_drawq;
	use_frgb = fb->r.use_frgb;
	api_changed = sdl_graphics_window_api_for_mode(old_mode) != sdl_graphics_window_api_for_mode(mode);

	// Refuse a cross-API switch when the native window belongs to the caller
	if (api_changed && sdl_graphics_window_is_foreign)
	{
		fprintf_rl(stderr, "sdl_graphics_set_drawq_mode(): cannot change the presentation API of a foreign window\n");
		return 0;
	}

	// Replace backend resources while retaining a compatible SDL window
	sdl_graphics_deinit_mode();
	fb->use_drawq = mode;
	fb->r.use_frgb = use_frgb;
	sdl_graphics_set_maxdim();

	// Recreate only on platforms whose presentation APIs require distinct windows
	if (api_changed && !sdl_graphics_recreate_window(mode))
	{
		// Restore the old mode on its unchanged compatible window
		fb->use_drawq = old_mode;
		fb->r.use_frgb = use_frgb;
		sdl_graphics_set_maxdim();
		if (!sdl_graphics_init_mode())
			fprintf_rl(stderr, "sdl_graphics_set_drawq_mode(): failed to restore draw queue mode %d\n", old_mode);
		SDL_SetWindowMaximumSize(fb->window, fb->maxdim.x, fb->maxdim.y);
		return 0;
	}

	// Complete the requested switch when its backend initializes successfully
	if (sdl_graphics_init_mode())
	{
		SDL_SetWindowMaximumSize(fb->window, fb->maxdim.x, fb->maxdim.y);
		return 1;
	}

	// Tear down the partial target mode before restoring the previous mode
	fprintf_rl(stderr, "sdl_graphics_set_drawq_mode(): draw queue mode %d failed to initialise, restoring mode %d\n", mode, old_mode);
	sdl_graphics_deinit_mode();
	fb->use_drawq = old_mode;
	fb->r.use_frgb = use_frgb;
	sdl_graphics_set_maxdim();

	// Restore the previous API window only on platforms that replaced it
	if (api_changed && !sdl_graphics_recreate_window(old_mode))
	{
		fprintf_rl(stderr, "sdl_graphics_set_drawq_mode(): failed to restore the window for draw queue mode %d\n", old_mode);
		return 0;
	}

	// Restore the previous backend after the failed switch
	if (!sdl_graphics_init_mode())
		fprintf_rl(stderr, "sdl_graphics_set_drawq_mode(): failed to restore draw queue mode %d\n", old_mode);
	SDL_SetWindowMaximumSize(fb->window, fb->maxdim.x, fb->maxdim.y);
	return 0;
}

void sdl_graphics_init_from_handle(const void *window_handle, int flags)
{
	// Record that backend-specific window recreation is not permitted
	sdl_graphics_window_is_foreign = 1;
	sdl_graphics_window_flags = flags;

	// Initialise SDL and shared graphics defaults
	sdl_init();
	SDL_SetHint(SDL_HINT_MOUSE_DOUBLE_CLICK_RADIUS, "4");
	if (fb->pixel_scale == 0)
		fb->pixel_scale = 1;
	sdl_graphics_texture_format = SDL_PIXELFORMAT_RGBA32;
	sdl_graphics_srgb_order = ORDER_RGBA;

	// Create a temporary window with the selected presentation API
	int temp_flags = (int) sdl_graphics_window_flags_for_mode(flags, fb->use_drawq);
#if RL_SDL == 2
	SDL_Window *temp_window = SDL_CreateWindow("", 0, 0, 1, 1, temp_flags | SDL_WINDOW_HIDDEN);
#else
	SDL_Window *temp_window = SDL_CreateWindow("", 1, 1, temp_flags | SDL_WINDOW_HIDDEN);
#endif
	char hint[32];
	sprintf(hint, "%p", temp_window);
#if RL_SDL == 2
	SDL_SetHint(SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT, hint);
#else
	SDL_SetHint(SDL_PROP_WINDOW_CREATE_WIN32_PIXEL_FORMAT_HWND_POINTER, hint);
#endif

	#ifdef RL_VULKAN
	// Mark an adopted SDL 2 window as Vulkan-compatible when Vulkan owns its surface
	#if RL_SDL == 2
	if (sdl_graphics_window_api_for_mode(fb->use_drawq)==SDL_GRAPHICS_WINDOW_API_VULKAN)
		SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_VULKAN, "1");
	#endif
	#endif

	#ifdef RL_OPENCL_GL
	// Mark an adopted window as OpenGL-compatible whenever the shared API requires it
	if (sdl_graphics_window_api_for_mode(fb->use_drawq)==SDL_GRAPHICS_WINDOW_API_OPENGL)
		#if RL_SDL == 2
		SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_OPENGL, "1");
		#else
		SDL_SetHint(SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, "1");
		#endif
	#endif
#if RL_SDL == 2
	// Adopt the foreign window and release the temporary format donor
	fb->window = SDL_CreateWindowFrom(window_handle);
	if (fb->window==NULL)
	{
		fprintf_rl(stderr, "SDL_CreateWindowFrom failed: %s\n", SDL_GetError());
		SDL_DestroyWindow(temp_window);
		return;
	}
	SDL_SetHint(SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT, NULL);
	#ifdef RL_VULKAN
	SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_VULKAN, NULL);
	#endif
	#ifdef RL_OPENCL_GL
	SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_OPENGL, NULL);
	#endif
	SDL_DestroyWindow(temp_window);

	// Initialise the selected graphics mode for the adopted window
	SDL_GetWindowSizeInPixels(fb->window, &fb->w, &fb->h);
	if (!sdl_graphics_init_with_fallback("sdl_graphics_init_from_handle()"))
		return;
	SDL_SetWindowSize(fb->window, fb->w, fb->h);
	SDL_GetWindowSize(fb->window, &fb->w, &fb->h);
	fb->r.dim = xyi(fb->w, fb->h);
	SDL_SetWindowMaximumSize(fb->window, fb->maxdim.x, fb->maxdim.y);

	#ifdef __EMSCRIPTEN__
	// Direct keyboard events to the Emscripten canvas
	SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#screen");
	#endif
#else
	// Leave foreign-window support disabled until its SDL 3 path is implemented
	SDL_DestroyWindow(temp_window);
#endif
}
void sdl_graphics_init_full(const char *window_name, xyi_t dim, xyi_t pos, int flags)
{
	// Record the creation flags used for future backend-specific replacement windows
	sdl_graphics_window_is_foreign = 0;
	sdl_graphics_window_flags = flags;

	// Initialise SDL and shared graphics defaults
	sdl_init();
	SDL_SetHint(SDL_HINT_MOUSE_DOUBLE_CLICK_RADIUS, "4");
	if (fb->pixel_scale == 0)
		fb->pixel_scale = 1;
	sdl_graphics_texture_format = SDL_PIXELFORMAT_ARGB8888;
	sdl_graphics_srgb_order = ORDER_BGRA;
	fb->w = dim.x;
	fb->h = dim.y;

	// Select an available startup mode before sizing the initial window
	if (fb->use_drawq < 0 || fb->use_drawq >= DRAWQ_MODE_COUNT)
	{
		fprintf_rl(stderr, "sdl_graphics_init_full(): invalid draw queue mode %d, using mode 0\n", fb->use_drawq);
		fb->use_drawq = DRAWQ_MODE_DIRECT;
	}
	while (!sdl_graphics_mode_available(fb->use_drawq) && fb->use_drawq)
	{
		fprintf_rl(stderr, "sdl_graphics_init_full(): draw queue mode %d is unavailable, falling back\n", fb->use_drawq);
		fb->use_drawq = sdl_graphics_fallback_mode(fb->use_drawq);
	}
	sdl_graphics_set_maxdim();

	// Select mutually exclusive creation flags for the initial presentation API
	uint64_t window_flags = sdl_graphics_window_flags_for_mode(flags, fb->use_drawq);

	// Create the initial window for the selected presentation backend
#if RL_SDL == 3
	fb->window = SDL_CreateWindow(window_name, fb->w, fb->h, (SDL_WindowFlags) window_flags);
#else
	fb->window = SDL_CreateWindow(window_name, -fb->maxdim.x-100, SDL_WINDOWPOS_UNDEFINED, fb->maxdim.x, fb->maxdim.y, (Uint32) window_flags);
#endif
	if (fb->window==NULL)
	{
		fprintf_rl(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
		return;
	}

	// Initialise the selected graphics mode through the shared path
	if (!sdl_graphics_init_with_fallback("sdl_graphics_init_full()"))
		return;

	// Move and resize the oversized hidden initial window to its requested rectangle
	#ifndef __APPLE__
	SDL_SetWindowPosition(fb->window, pos.x, pos.y);
	#endif
	SDL_SetWindowSize(fb->window, fb->w, fb->h);
	SDL_GetWindowSize(fb->window, &fb->w, &fb->h);
	fb->r.dim = xyi(fb->w, fb->h);
	SDL_SetWindowMaximumSize(fb->window, fb->maxdim.x, fb->maxdim.y);

#if RL_SDL == 3
	// Enable text input for the newly created SDL 3 window
	SDL_StartTextInput(fb->window);
#endif

	#ifdef __EMSCRIPTEN__
	// Direct keyboard events to the Emscripten canvas
	SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#screen");
	#endif
}

void sdl_graphics_init_autosize(const char *window_name, int flags, int window_index)
{
	recti_t r;

	// Initialise SDL before querying display geometry
	sdl_init();
	r = sdl_get_display_usable_rect(window_index);
	r = recti_add_margin(r, xyi(-8, -16));
	r.p0.y += 14;
	sdl_graphics_init_full(window_name, get_recti_dim(r), r.p0, flags);	// initialise SDL as well as the framebuffer
}

int sdl_handle_window_resize(zoom_t *zc)
{
	int w, h;

	SDL_GetWindowSize(fb->window, &w, &h);

	if (fb->w == w && fb->h == h && fb->pixel_scale == fb->pixel_scale_new)
		return 0;

	// Restore the base direct framebuffer before changing dimensions
	if (fb->use_drawq==DRAWQ_MODE_DIRECT && (fb->drawq_direct_bracket_depth || fb->drawq_direct_bracket_overflow))
		drawq_bracket_deinit_direct();

	// Finish drawqueue processing
	#ifdef RL_OPENCL_GL
	// Finish OpenCL interop before replacing its presentation surface
	if (fb->use_drawq==DRAWQ_MODE_OPENCL)
		clFinish_wrap(fb->clctx.command_queue);	// wait for end of queue
	#endif

	if (fb->use_drawq==DRAWQ_MODE_SOFTWARE)
		drawq_soft_finish();

	// Set new dimensions
	fb->w = w;
	fb->h = h;
	fb->r.dim = xyi(fb->w, fb->h);

	// Handle pixel scale
	if (fb->pixel_scale_new == 0)
		fb->pixel_scale_new = fb->pixel_scale;
	else
		fb->pixel_scale = fb->pixel_scale_new;
	fb->real_dim = fb->r.dim;
	fb->r.dim = idiv_ceil_xyi(fb->r.dim, set_xyi(fb->pixel_scale));
	fb->w = fb->r.dim.x;
	fb->h = fb->r.dim.y;

	// Remake texture
	int remake_tex = 1;
	#ifdef RL_OPENCL_GL
	// OpenCL interop renders without an SDL streaming texture
	if (fb->use_drawq==DRAWQ_MODE_OPENCL)
		remake_tex = 0;
	#endif
	#ifdef RL_VULKAN
	// Let Vulkan recreate its swapchain lazily instead of constructing an SDL texture
	if (fb->use_drawq==DRAWQ_MODE_VULKAN)
	{
		remake_tex = 0;
		vk_mark_swapchain_dirty();
	}
	#endif

	if (remake_tex)
	{
		if (fb->tex_lock)
		{
			SDL_UnlockTexture(fb->texture);
			fb->tex_lock = 0;
		}

		SDL_DestroyTexture(fb->texture);

		fb->texture = SDL_CreateTexture(fb->renderer, sdl_graphics_texture_format, SDL_TEXTUREACCESS_STREAMING, fb->w, fb->h);
		if (fb->texture==NULL)
			fprintf_rl(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());

		// Blank the resized texture
		int pitch;
		SDL_LockTexture(fb->texture, NULL, &fb->r.srgb, &pitch);
		memset(fb->r.srgb, 0, pitch * fb->r.dim.y);
		SDL_UnlockTexture(fb->texture);
	}

	// Recalc screen limits
	calc_screen_limits(zc);

	return 1;
}

static void sdl_flip_fb_internal(int start_drawq)
{
	if (fb->timing==NULL)
		fb->timing = calloc(fb->timing_as = fb->timing_count = 120, sizeof(frame_timing_t));
	// Clear timing carried by this reused circular slot
	fb->timing[fb->timing_index].thread_start = 0.;
	fb->timing[fb->timing_index].thread_end = 0.;
	fb->timing[fb->timing_index].func_end = get_time_hr();
	fb->timing[fb->timing_index].flip_end = fb->timing[fb->timing_index].func_end;

	#ifdef RL_VULKAN
	// Vulkan has no separate work in the serial flip phase
	if (fb->use_drawq==DRAWQ_MODE_VULKAN)
	{
		fb->timing[fb->timing_index].flip_end = fb->timing[fb->timing_index].func_end;
		fb->first_frame_done = 1;
	}
	#endif

	#ifdef RL_OPENCL
	// Present the previous OpenCL frame before enqueuing the next one
	if (fb->use_drawq==DRAWQ_MODE_OPENCL)
	{
		if (fb->first_frame_done && mouse.window_minimised_flag <= 0)
		{
			cl_int ret=0;
			#ifdef RL_OPENCL_GL
			if (fb->opt_interop && fb->cl_srgb_acquired)
			{
				ret = clEnqueueReleaseGLObjects_wrap(fb->clctx.command_queue, 1, &fb->cl_srgb, 0, 0, NULL);		// release the ownership (back to GL)
				CL_ERR_NORET("clEnqueueReleaseGLObjects in sdl_flip_fb()", ret);
				fb->cl_srgb_acquired = 0;
			}
			#endif

			// display srgb
			if (fb->opt_clfinish)
			{
				ret = clFinish_wrap(fb->clctx.command_queue);
				CL_ERR_NORET("clFinish in sdl_flip_fb()", ret);
			}

			#ifdef RL_OPENCL_GL
			if (fb->pixel_scale_oldframe == 0)
				fb->pixel_scale_oldframe = fb->pixel_scale;
			float tscale = 1.f / (float) fb->pixel_scale_oldframe;
			fb->pixel_scale_oldframe = fb->pixel_scale;
			float hoff = 2. * (fb->real_dim.y - fb->maxdim.y) / (double) fb->maxdim.y;
			glLoadIdentity();             // Reset the projection matrix
			glViewport(0, 0, fb->maxdim.x, fb->maxdim.y);

			glBegin(GL_QUADS);
			glTexCoord2f(0.f, 0.f);		glVertex2f(-1., 1.+hoff);
			glTexCoord2f(tscale, 0.f);	glVertex2f(1., 1.+hoff);
			glTexCoord2f(tscale, tscale);	glVertex2f(1., -1.+hoff);
			glTexCoord2f(0.f, tscale);	glVertex2f(-1., -1.+hoff);
			glEnd();

			SDL_GL_SwapWindow(fb->window);
			fb->timing[fb->timing_index].flip_end = get_time_hr();

			#else

			if (fb->tex_lock)
			{
				SDL_UnlockTexture(fb->texture);
				fb->tex_lock = 0;
			}

			SDL_RenderClear(fb->renderer);
			SDL_RenderCopy(fb->renderer, fb->texture, NULL, NULL);
			SDL_RenderPresent(fb->renderer);
			fb->timing[fb->timing_index].flip_end = get_time_hr();
			#endif
			#ifdef RL_VULKAN
			// Reveal the completed OpenCL frame after switching away from Vulkan
			vk_hide_surface_window();
			#endif
		}
		fb->first_frame_done = 1;
	}
	#endif

	if (fb->use_drawq==DRAWQ_MODE_SOFTWARE)
	{
		if (fb->first_frame_done)
		{
			drawq_soft_finish();

			if (fb->tex_lock)
			{
				SDL_UnlockTexture(fb->texture);
				fb->tex_lock = 0;
			}

			SDL_RenderClear(fb->renderer);
			SDL_RenderCopy(fb->renderer, fb->texture, NULL, NULL);
			SDL_RenderPresent(fb->renderer);
			#ifdef RL_VULKAN
			// Reveal the completed software frame after switching away from Vulkan
			vk_hide_surface_window();
			#endif
			fb->timing[fb->timing_index].flip_end = get_time_hr();
		}
		fb->first_frame_done = 1;
	}

	if (fb->use_drawq)
	{
		// Collapse skipped queue phases at the end of a mode-switch frame
		if (!start_drawq)
		{
			fb->timing[fb->timing_index].interop_sync_end = fb->timing[fb->timing_index].flip_end;
			fb->timing[fb->timing_index].dq_comp_end = fb->timing[fb->timing_index].flip_end;
			fb->timing[fb->timing_index].cl_copy_end = fb->timing[fb->timing_index].flip_end;
			fb->timing[fb->timing_index].cl_enqueue_end = fb->timing[fb->timing_index].flip_end;
		}

		// Start the next draw queue only when the current backend will remain active
		if (start_drawq)
		{
			if (mouse.window_minimised_flag <= 0)
				drawq_run();
			else				// if the window is minimised just don't do any drawq stuff
			{
				drawq_reinit();
				SDL_Delay(80);		// there's no more vsync therefore the loop can easily run at 500+ FPS without the delay
			}
		}
	}
	else
	{
		// Recover unbalanced direct brackets before converting the base framebuffer
		if (fb->drawq_direct_bracket_depth || fb->drawq_direct_bracket_overflow)
			drawq_bracket_deinit_direct();

		// Blits framebuffer to screen
		int pitch;
		SDL_Rect rs;
		rs = make_sdl_rect(0, 0, fb->w, fb->h);
		SDL_LockTexture(fb->texture, &rs, &fb->r.srgb, &pitch);
		#ifdef LRGB_NODITHER
		convert_linear_rgb_to_output(NODITHER);
		#else
		convert_linear_rgb_to_output(DITHER);
		#endif
		SDL_UnlockTexture(fb->texture);
		//SDL_UpdateTexture(fb->texture, NULL, fb->srgb, fb->w * 4);

		SDL_RenderClear(fb->renderer);
		SDL_RenderCopy(fb->renderer, fb->texture, NULL, NULL);
		SDL_RenderPresent(fb->renderer);
		#ifdef RL_VULKAN
		// Reveal the completed direct frame after switching away from Vulkan
		vk_hide_surface_window();
		#endif
		fb->timing[fb->timing_index].interop_sync_end = get_time_hr();
		fb->timing[fb->timing_index].dq_comp_end = get_time_hr();
		fb->timing[fb->timing_index].cl_copy_end = get_time_hr();
		fb->timing[fb->timing_index].cl_enqueue_end = get_time_hr();
		fb->timing[fb->timing_index].flip_end = get_time_hr();

		screen_blank();
	}

	double t = get_time_hr();
	fb->timing[fb->timing_index].end = t;
	fb->timing_index = circ_index(fb->timing_index+1, fb->timing_count);
	fb->timing[fb->timing_index].start = t;
	sleep_hr(rangelimit(fb->start_sleep_dur - t + fb->timing[circ_index(fb->timing_index-1, fb->timing_count)].flip_end, 0., fb->start_sleep_dur));
	fb->timing[fb->timing_index].start_sleep = get_time_hr();

	fb->frame_count++;
}

void sdl_flip_fb()
{
	// Present the completed frame and start work for the following frame
	sdl_flip_fb_internal(1);
}

void sdl_flip_fb_mode_switch_pending()
{
	// Present the completed frame without starting work on the backend being replaced
	sdl_flip_fb_internal(0);
}

void sdl_flip_fb_srgb(srgb_t *sfb)
{
	// Blits framebuffer to screen
	int pitch;
	SDL_Rect rs;
	rs = make_sdl_rect(0, 0, fb->w, fb->h);
	SDL_LockTexture(fb->texture, &rs, &fb->r.srgb, &pitch);
	//memcpy(fb->r.srgb, sfb, mul_x_by_y_xyi(fb->r.dim) * sizeof(srgb_t));
	srgb_change_order(sfb, fb->r.srgb, mul_x_by_y_xyi(fb->r.dim), ORDER_BGRA);
	SDL_UnlockTexture(fb->texture);

	SDL_RenderClear(fb->renderer);
	SDL_RenderCopy(fb->renderer, fb->texture, NULL, NULL);
	SDL_RenderPresent(fb->renderer);

	screen_blank();
}

int sdl_toggle_borderless_fullscreen()
{
	fb->fullscreen_on ^= 1;

	if (fb->fullscreen_on)
	{
#if RL_SDL == 2
		fb->wind_rect = sdl_get_window_rect(fb->window);

		SDL_SetWindowResizable(fb->window, SDL_FALSE);
		SDL_SetWindowBordered(fb->window, SDL_TRUE);
		sdl_set_window_rect(fb->window, sdl_get_display_rect(sdl_get_window_cur_display()));
#else
		SDL_SetWindowFullscreen(fb->window, 1);
#endif
	}
	else
	{
#if RL_SDL == 2
		sdl_set_window_rect(fb->window, fb->wind_rect);

		SDL_SetWindowResizable(fb->window, SDL_TRUE);
		SDL_SetWindowBordered(fb->window, SDL_TRUE);
#else
		SDL_SetWindowFullscreen(fb->window, 0);
#endif
	}

	#ifdef __EMSCRIPTEN__
	//em_sync_by_mutex(1);
	em_browser_toggle_fullscreen(fb->fullscreen_on);
	//em_sync_by_mutex(1);
	//em_sync_by_mutex(0);
	//if (fb->fullscreen_on == 0)
	//	SDL_SetWindowSize(fb->window, 900, 600);
	fprintf_rl(stdout, "fb->fullscreen_on %d %d\n", fb->fullscreen_on, rand()&63);
	#endif

	sdl_handle_window_resize(&zc);

	return fb->fullscreen_on;
}

void sdl_quit_actions()
{
	// Release the active graphics mode through the shared teardown path
	sdl_graphics_deinit_mode();

	#ifdef RL_VULKAN
	// Release retained Vulkan objects before destroying their persistent window
	vk_deinit();
	#endif

	// Destroy the persistent window before shutting down SDL
	SDL_DestroyWindow(fb->window);
	fb->window = NULL;
	SDL_Quit();
}

// Audio
void sdl_init_audio_not_wasapi()
{
	int i;

#if RL_SDL == 3
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
#else
	SDL_AudioQuit();	// quit the current audio driver, probably wasapi
#endif

	// Init the first driver that isn't wasapi
	for (i=0; i < SDL_GetNumAudioDrivers(); i++)
		if (strcmp("wasapi", SDL_GetAudioDriver(i)))	// if the driver isn't called "wasapi"
		{
		#if RL_SDL == 3
			SDL_SetHint("SDL_AUDIO_DRIVER", SDL_GetAudioDriver(i));
			SDL_InitSubSystem(SDL_INIT_AUDIO);
		#else
			SDL_AudioInit(SDL_GetAudioDriver(i));	// initialise it
		#endif
			return;
		}
}

// Clipboard
char *sdl_get_clipboard_dos_conv()
{
	char *orig, *edited, byte0, byte1=0;
	int i, offset=0, len;

	orig = SDL_GetClipboardText();
	if (orig==NULL)
		return NULL;

	len = strlen(orig);

	edited = calloc (len+1, sizeof(char));

	for (i=0; i < len; i++)
	{
		byte0 = byte1;
		byte1 = orig[i];

		if (byte0=='\r' && byte1=='\n')
			offset++;

		edited[i-offset] = byte1;
	}

	edited = realloc(edited, len-offset+1);
	SDL_free(orig);

	return edited;
}

// Misc
void sdl_print_sdl_version()
{
#if RL_SDL == 2
	SDL_version compiled, linked;

	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);
	fprintf_rl(stdout, "Compilation SDL version %d.%d.%d\n", compiled.major, compiled.minor, compiled.patch);
	fprintf_rl(stdout, "Linked SDL version %d.%d.%d %s\n", linked.major, linked.minor, linked.patch, SDL_GetRevision());
#else
	int v = SDL_VERSION;
	fprintf_rl(stdout, "Compilation SDL version %d.%d.%d\n", SDL_VERSIONNUM_MAJOR(v), SDL_VERSIONNUM_MINOR(v), SDL_VERSIONNUM_MICRO(v));
	v = SDL_GetVersion();
	fprintf_rl(stdout, "Linked SDL version %d.%d.%d %s\n", SDL_VERSIONNUM_MAJOR(v), SDL_VERSIONNUM_MINOR(v), SDL_VERSIONNUM_MICRO(v), SDL_GetRevision());
#endif
}

void sdl_box_printf(const char *title, const char *format, ...)
{
	buffer_t s={0};
	va_list args;

	va_start(args, format);
	vbufprintf(&s, format, args);
	va_end(args);

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, s.buf, NULL);
	free_buf(&s);
}

// Drag and drop
dropfile_t dropfile={0};

void dropfile_event_proc(SDL_Event event)
{
	if (dropfile.path_as==0)
		dropfile.id_last = -1;

	if (event.type==SDL_DROPFILE)
	{
		dropfile.id_last++;
		alloc_enough((void **) &dropfile.path, dropfile.id_last+1, &dropfile.path_as, sizeof(char *), 1.5);

#if RL_SDL == 2
		dropfile.path[dropfile.id_last] = make_string_copy(event.drop.file);
		SDL_free(event.drop.file);
#else
		dropfile.path[dropfile.id_last] = make_string_copy(event.drop.data);
#endif
	}
}

int dropfile_get_count()
{
	int i, count=0;

	if (dropfile.path)
		for (i=0; i <= dropfile.id_last; i++)
			if (dropfile.path[i])
				count++;

	return count;
}

char *dropfile_pop_first()
{
	int i;
	char *p=NULL;

	if (dropfile.path)
		for (i=0; i <= dropfile.id_last; i++)
			if (dropfile.path[i])
			{
				p = dropfile.path[i];
				dropfile.path[i] = NULL;

				if (i==dropfile.id_last)
					dropfile.id_last = -1;

				break;
			}

	return p;
}

void rl_sdl_standard_main_loop(sdl_main_param_t param)
{
	static int exit_flag_s=0, init=1;
	SDL_Event event;

	if (init)
	{
		init = 0;

		if (param.exit_flag==NULL)
			param.exit_flag = &exit_flag_s;

		fb->use_drawq = param.use_drawq;	// OpenCL draw queue

		if (is0_xyi(param.wind_dim))
			sdl_graphics_init_autosize(param.window_name, SDL_WINDOW_RESIZABLE, 0);
		else
			sdl_graphics_init_full(param.window_name, param.wind_dim, param.wind_pos, SDL_WINDOW_RESIZABLE);

		if (param.maximise_window)
			SDL_MaximizeWindow(fb->window);

		zc = init_zoom(&mouse, drawing_thickness);
		calc_screen_limits(&zc);
		init_mouse();

		#ifdef RL_INCL_VECTOR_TYPE_FILEBALL
		vector_font_load_from_header();
		#else
		make_fallback_font(font);
		#endif

		#ifdef __EMSCRIPTEN__
		SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#screen");
		#endif
	}

	#ifdef __EMSCRIPTEN__
	#else
	while (*param.exit_flag == 0)
	#endif
	{
		//********Input handling********

		mouse_pre_event_proc(&mouse);
		keyboard_pre_event_proc(&mouse);
		sdl_handle_window_resize(&zc);

		while (SDL_PollEvent(&event))
		{
			dropfile_event_proc(event);
			sdl_mouse_event_proc(&mouse, event, &zc);
			sdl_keyboard_event_proc(&mouse, event);

			if (event.type==SDL_QUIT)
				*param.exit_flag = 1;
		}

		if (mouse.key_state[RL_SCANCODE_RETURN] == 2 && get_kb_alt() != -1)
			sdl_toggle_borderless_fullscreen();

		textedit_add(cur_textedit, NULL);	// processes the new keystrokes in the current text editor

		mouse_post_event_proc(&mouse, &zc);

		//-------------input-----------------

		param.func();

		#ifndef HIDE_GUI_TOOLBAR
		if (param.gui_toolbar)
			gui_layout_edit_toolbar(mouse.key_state[RL_SCANCODE_F6]==2);
		#endif
		window_manager();

		if (param.show_cursor)
			mouse.showcursor = 1;
		mousecursor_logic_and_draw();

		sdl_flip_fb();
	}
}

#endif
