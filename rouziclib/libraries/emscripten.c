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

#endif
