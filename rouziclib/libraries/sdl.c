#ifdef RL_SDL

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

#endif
