#ifdef RL_SDL

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

int sdl_get_window_hz(SDL_Window *window)
{
	SDL_DisplayMode mode;

	if (SDL_GetWindowDisplayMode(window, &mode) < 0)
		fprintf_rl(stderr, "SDL_GetWindowDisplayMode failed: %s\n", SDL_GetError());

	return MAXN(mode.refresh_rate, 60);
}

int sdl_vsync_sleep(SDL_Window *window, uint32_t time_last_vsync)
{
	int hz = sdl_get_window_hz(window);				// refresh rate (e.g. 60 Hz)
	int ms = (100000 / sdl_get_window_hz(window) - 60) / 100;	// total time needed (e.g. 60 Hz -> 16 ms)
	int elapsed = SDL_GetTicks() - time_last_vsync;			// ms since last vsync
	int delay = ms - elapsed;

	if (delay > 0)			// if sleeping is needed
		SDL_Delay(delay);

	return delay;
}

recti_t sdl_get_window_rect(SDL_Window *window)
{
	recti_t r;

	SDL_GetWindowSize(window, &r.p1.x, &r.p1.y);
	SDL_GetWindowPosition(window, &r.p0.x, &r.p0.y);
	r.p1 = add_xyi(r.p0, r.p1);

	return r;
}

recti_t sdl_get_display_rect(int display_id)
{
	SDL_Rect r;

	if (display_id < SDL_GetNumVideoDisplays())
		if (SDL_GetDisplayBounds(display_id, &r)==0)
			return recti( xyi(r.x, r.y), xyi(r.x+r.w-1, r.y+r.h-1));

	return recti(xyi(0,0), xyi(0,0));
}

recti_t sdl_get_display_usable_rect(int display_id)
{
	SDL_Rect r;

	if (display_id < SDL_GetNumVideoDisplays())
		if (SDL_GetDisplayUsableBounds(display_id, &r)==0)
			return recti( xyi(r.x, r.y), xyi(r.x+r.w-1, r.y+r.h-1));

	return recti(xyi(0,0), xyi(0,0));
}

recti_t sdl_screen_max_window_rect()
{
	int i;
	recti_t dr, wr=recti(xyi(0,0), xyi(0,0));

	for (i=0; i < SDL_GetNumVideoDisplays(); i++)
	{
		dr = sdl_get_display_rect(i);
		wr.p0 = min_xyi(wr.p0, dr.p0);
		wr.p1 = max_xyi(wr.p1, dr.p1);
	}

	return wr;
}

xyi_t sdl_screen_max_window_size()
{
	return get_recti_dim(sdl_screen_max_window_rect());
}

recti_t sdl_get_window_border(SDL_Window *window)
{
	recti_t r;

	SDL_GetWindowBordersSize(window, &r.p0.y, &r.p0.x, &r.p1.y, &r.p1.x);

	return r;
}

void mouse_event_proc(mouse_t *mouse, SDL_Event event, zoom_t *zc)
{
	double mouse_speed = 1.;	// mouse-on-screen speed is about 0.032
	SDL_Keymod mod_state;

	if (event.type==SDL_WINDOWEVENT)
	{
		if (event.window.event==SDL_WINDOWEVENT_ENTER)
			mouse->mouse_focus_flag = 2;

		if (event.window.event==SDL_WINDOWEVENT_LEAVE)
			mouse->mouse_focus_flag = -2;

		if (event.window.event==SDL_WINDOWEVENT_FOCUS_GAINED)
			mouse->window_focus_flag = 2;

		if (event.window.event==SDL_WINDOWEVENT_FOCUS_LOST)
			mouse->window_focus_flag = -2;
	}

	if (event.type==SDL_MOUSEMOTION)
	{
		mouse->a = xy(event.motion.x, event.motion.y);
		mouse->u = to_world_coord_xy(*zc, mouse->a);
		mouse->d = mul_xy(xy(event.motion.xrel, event.motion.yrel), set_xy(mouse_speed));
		mouse->du = mul_xy(xy(event.motion.xrel, -event.motion.yrel), set_xy(zc->iscrscale));

		if (mouse->zoom_flag)	// if we're scrolling the zoom
		{
			zc->offset_u = add_xy(zc->offset_u, mul_xy(mouse->du, set_xy(2.)));
			calc_screen_limits(zc);
		}
	}

	if (event.type==SDL_MOUSEBUTTONDOWN)
	{
		if (mouse->window_focus_flag != 2)	// if the window wasn't just clicked on to gain its focus
		{
			if (event.button.button==SDL_BUTTON_LEFT)
				mouse->b.lmb = 2;

			if (event.button.button==SDL_BUTTON_RIGHT)
				mouse->b.rmb = 2;
		}

		if (event.button.button==SDL_BUTTON_MIDDLE)
		{
			mouse->b.mmb = 2;
			zc->zoom_key_time = SDL_GetTicks();
		}
	}

	if (event.type==SDL_MOUSEBUTTONUP)
	{
		if (event.button.button==SDL_BUTTON_LEFT)
			mouse->b.lmb = -2;

		if (event.button.button==SDL_BUTTON_RIGHT)
			mouse->b.rmb = -2;

		if (event.button.button==SDL_BUTTON_MIDDLE)
		{
			mouse->b.mmb = -2;
			zoom_key_released(zc, &mouse->zoom_flag, 1);
		}
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

	mouse->mod_key[mouse_mod_ctrl] = mod_state & KMOD_CTRL;
	mouse->mod_key[mouse_mod_alt] = mod_state & KMOD_ALT;
	mouse->mod_key[mouse_mod_shift] = mod_state & KMOD_SHIFT;
	mouse->mod_key[mouse_mod_gui] = mod_state & KMOD_GUI;
}

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

SDL_GLContext init_sdl_gl(SDL_Window *window)
{
	SDL_GLContext ctx=NULL;
#ifdef RL_OPENCL
	GLenum glewError;

	ctx = SDL_GL_CreateContext(window);		//Create context
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

void sdl_graphics_init_full(raster_t *fb, const char *window_name, xyi_t dim, xyi_t pos, int flags)
{
	static int init=1;
	SDL_DisplayMode dm;
	SDL_GLContext gl_ctx;

	if (init)
	{
		init = 0;
		if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO))
			fprintf_rl(stderr, "SDL_Init failed: %s\n", SDL_GetError());
	}

	fb->w = dim.x;
	fb->h = dim.y;

	fb->maxw = sdl_screen_max_window_size().x;
	fb->maxh = sdl_screen_max_window_size().y;

	fb->window = SDL_CreateWindow (	window_name,			// window title
					-fb->maxw-100,			// initial x position
					SDL_WINDOWPOS_UNDEFINED,	// initial y position
					fb->maxw,			// width, in pixels
					fb->maxh,			// height, in pixels
					SDL_WINDOW_OPENGL | flags);	// flags - see https://wiki.libsdl.org/SDL_CreateWindow
	if (fb->window==NULL)
		fprintf_rl(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());

	SDL_GetWindowSize(fb->window, &fb->maxw, &fb->maxh);

	if (fb->use_cl)
	{
		#ifdef RL_OPENCL
		gl_ctx = init_sdl_gl(fb->window);
		fb->renderer = SDL_CreateRenderer(fb->window, get_sdl_opengl_renderer_index(), SDL_RENDERER_PRESENTVSYNC);
		if (fb->renderer==NULL)
			fprintf_rl(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
		#endif
	}
	else
	{
		fb->renderer = SDL_CreateRenderer(fb->window, -1, SDL_RENDERER_PRESENTVSYNC);
		if (fb->renderer==NULL)
			fprintf_rl(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());

		fb->texture = SDL_CreateTexture(fb->renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, fb->w, fb->h);
		if (fb->texture==NULL)
			fprintf_rl(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
	}

	if (fb->use_cl==0)
	{
		// fb->srgb doesn't need to be allocated if it points to the SDL surface thanks to SDL_LockTexture
		fb->srgb = NULL;//calloc (fb->maxw*fb->maxh, sizeof(srgb_t));

		if (fb->use_frgb==0)
			fb->l = calloc (fb->maxw*fb->maxh, sizeof(lrgb_t));
		else
			fb->f = calloc (fb->maxw*fb->maxh, sizeof(frgb_t));
	}

	#ifdef RL_OPENCL
	if (fb->use_cl)
		init_fb_cl(fb);
	#endif

	SDL_SetWindowSize(fb->window, fb->w, fb->h);
	SDL_GetWindowSize(fb->window, &fb->w, &fb->h);
	SDL_SetWindowPosition(fb->window, pos.x, pos.y);
}

void sdl_graphics_init_autosize(raster_t *fb, const char *window_name, int flags, int window_index)
{
	recti_t r;

	if (SDL_Init(SDL_INIT_VIDEO))
		fprintf_rl(stderr, "SDL_Init failed: %s\n", SDL_GetError());

	r = sdl_get_display_usable_rect(window_index);
	r = recti_add_margin(r, xyi(-8, -16));
	r.p0.y += 14;
	sdl_graphics_init_full(fb, window_name, get_recti_dim(r), r.p0, flags);	// initialise SDL as well as the framebuffer
}

#endif
