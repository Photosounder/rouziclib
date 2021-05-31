function js_string_to_c_string(js_string)
{
	var len = lengthBytesUTF8(js_string);
	var c_string = _malloc(len+1);
	stringToUTF8(js_string, c_string, len);

	return c_string;
}

function em_exit_fullscreen_ret_promise()	// adapted from _emscripten_exit_fullscreen to return the promise
{
	if (!JSEvents.fullscreenEnabled())
		return null;

	// Make sure no queued up calls will fire after this.
	JSEvents.removeDeferredCalls(_JSEvents_requestFullscreen);

	if (document.exitFullscreen)
	{
		if (document.fullscreenElement)
			return document.exitFullscreen();
	}
	else if (document.webkitExitFullscreen)
	{
		if (document.webkitFullscreenElement)
			return document.webkitExitFullscreen();
	}

	return null;
}

function em_sync_by_mutex_js(lock)
{
	Module.ccall('em_sync_by_mutex', 'undefined', ['number'], [lock], { async: true });
}
