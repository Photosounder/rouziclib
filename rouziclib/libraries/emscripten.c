#ifdef __EMSCRIPTEN__

EM_JS(int, em_get_memory_size, (),
{
	return HEAP8.length;
});

EM_JS(void, em_disable_context_menu, (),
{
	document.addEventListener("contextmenu", function(event) {event.preventDefault();}, false);
});

EM_JS(void, em_mmb_capture, (),
{
	document.body.onmousedown = function(e)
	{
		if (e.button === 1)
		{
			canvas.requestPointerLock();
			return false;
		}
	}
});

EM_JS(void, em_capture_cursor, (),
{
	canvas.requestPointerLock();
});

EM_JS(void, em_release_cursor, (),
{
	document.exitPointerLock();
});

void em_sync_by_mutex(int lock)
{
	static int init=1;
	static rl_mutex_t my_mutex;

	if (init)
	{
		init = 0;
		rl_mutex_init(&my_mutex);
	}
fprintf_rl(stdout, "em_sync_by_mutex(%d) %d\n", lock, rand()&63);

	if (lock)
		rl_mutex_lock(&my_mutex);
	else
		rl_mutex_unlock(&my_mutex);
}

/*EM_JS(void, em_browser_toggle_fullscreen, (int state),
{
Asyncify.handleAsync(async () => {
	if (state)
		await canvas.requestFullscreen();
	else
		await em_exit_fullscreen_ret_promise();
});
});*/

EM_JS(void, em_browser_toggle_fullscreen, (int state),
{
	if (state)
		canvas.requestFullscreen();
	else
		em_exit_fullscreen_ret_promise();
});

/*void em_browser_toggle_fullscreen(int state)
{
	if (state)
	{
		EmscriptenFullscreenStrategy strat={0};
		strat.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
		strat.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF;
		strat.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST;
		emscripten_request_fullscreen_strategy(NULL, 1, &strat);
	}
	else
	{
		emscripten_exit_fullscreen();
	}
}*/

void em_fit_canvas_to_innerwindow()
{
	if (fb.fullscreen_on)
		return ;

	SDL_SetWindowSize(fb.window, MINN(fb.maxdim.x, EM_ASM_INT(return window.innerWidth;)), MINN(fb.maxdim.y, EM_ASM_INT(return window.innerHeight;)));
	sdl_handle_window_resize(&zc);
}

EM_JS(void, em_enumerate_av_devices, (),
{
	navigator.mediaDevices.enumerateDevices()
		.then(function(devices) {
			var str="";
			devices.forEach(
			function(device)
			{
				console.log(device.kind + ": " + device.label + " id = " + device.deviceId);
				str += device.kind + " \"" + device.label + "\" id = " + device.deviceId + "\n";
			}
			);
			console.log(str);
			var c_str = js_string_to_c_string(str);		// this is the C string on the WASM heap I'm trying to access in C
			})
		.catch(function(err) { console.log(err.name + ": " + err.message); });
});

#endif
