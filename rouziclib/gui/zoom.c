zoom_t init_zoom(raster_t *fb, mouse_t *mouse, double drawing_thickness)
{
	zoom_t zc;

	memset(&zc, 0, sizeof(zc));

	zc.guiscale = 1.;
	zc.zoomscale = 1.;
	zc.fb = fb;
	zc.mouse = mouse;
	zc.drawing_thickness = drawing_thickness;

	return zc;
}

double to_screen_coord_x(zoom_t zc, double x)
{
	x -= zc.offset_u.x;

	return x * zc.scrscale + 0.5*(double)zc.fb->w - 0.5;
}

double to_screen_coord_y(zoom_t zc, double y)
{
	y -= zc.offset_u.y;
	return -y * zc.scrscale + 0.5*(double)zc.fb->h - 0.5;
}

xy_t to_screen_coord_xy(zoom_t zc, xy_t p)
{
	return xy(to_screen_coord_x(zc, p.x), to_screen_coord_y(zc, p.y));
}

double to_world_coord_x(zoom_t zc, double x)
{
	return (x - 0.5*(double)zc.fb->w + 0.5) * zc.iscrscale + zc.offset_u.x;
}

double to_world_coord_y(zoom_t zc, double y)
{
	return -(y - 0.5*(double)zc.fb->h + 0.5) * zc.iscrscale + zc.offset_u.y;
}

xy_t to_world_coord_xy(zoom_t zc, xy_t p)
{
	return xy(to_world_coord_x(zc, p.x), to_world_coord_y(zc, p.y));
}

rect_t to_screen_coord_rect(zoom_t zc, rect_t r)
{
	return rect( sc_xy(r.p0) , sc_xy(r.p1) );
}

void zoom_key_released(zoom_t *zc, int8_t *flag_zoom_key, int source)	// source 1 is when the button is released, source 2 is while the button is being held down
{
	int32_t td;
	int32_t zko = *flag_zoom_key;						// save the original zoom_key state

	#ifdef RL_SDL
	td = SDL_GetTicks() - zc->zoom_key_time;				// time difference
	#endif

	if (source==1 && *flag_zoom_key==0 && zc->zoom_key_time)		// if the button was just released as the zoom was off and the timer running
		*flag_zoom_key = 1;						// turn the zoom on
	else if (source==1 && *flag_zoom_key && zc->zoom_key_time && td <= 500)	// if the button has been pressed for less than 0.5s and released
		*flag_zoom_key = 0;						// turn off the zoom mode, don't reset the view

	if (source==2 && zc->zoom_key_time && td > 500)				// if the button is held down and for more than 0.5s and the button timer is running
	{
		zc->zoomscale = 1.;						// reset everything
		zc->offset_u = XY0;
		*flag_zoom_key = 0;
		zc->zoom_key_time = 0;
	}

	calc_screen_limits(zc);
	
	if (*flag_zoom_key==0 && zko)				// if the cursor is to be shown and we switched from zoom on to zoom off
	{
		#ifdef RL_SDL
		SDL_SetRelativeMouseMode(0);
		SDL_WarpMouseInWindow(zc->fb->window, zc->fb->w/2, zc->fb->h/2);
		#endif
	}
}

void zoom_wheel(zoom_t *zc, int8_t flag_zoom_key, int y)
{
	double ratio;

	if (flag_zoom_key)			// if the zoom key is down
	{
		ratio = pow (2., 1./2. * (double) y);
		zc->zoomscale *= ratio;

		//zc->offset_u = sub_xy(zc->mouse->u, div_xy(sub_xy(zc->mouse->u, zc->offset_u), set_xy(ratio)));	// zoom using the cursor position as an anchor point

		calc_screen_limits(zc);
	}
}

void calc_screen_limits(zoom_t *zc)
{
	int x, y;

	if (3*zc->fb->w > 4*zc->fb->h)			// if widescreen (more than 4:3 aka 12:9)
		zc->scrscale = (double) zc->fb->h / 18.;	// for 1920x1080 srcscale would be 60
	else
		zc->scrscale = (double) zc->fb->w / 24.;
	zc->scrscale *= zc->guiscale;
	zc->scrscale_unzoomed = zc->scrscale;

	zc->limit_u = mul_xy(xy(zc->fb->w, zc->fb->h), set_xy(0.5/zc->scrscale));

	zc->scrscale *= zc->zoomscale;
	zc->iscrscale = 1. / zc->scrscale;

	//zc->drawlim_u = set_xy(zc->iscrscale * GAUSSRAD(1., zc->drawing_thickness));
	zc->drawlim_u = set_xy(zc->iscrscale * GAUSSRAD_HQ * zc->drawing_thickness);
	zc->border_u = add_xy(zc->limit_u, zc->drawlim_u);

	zc->corners.p0 = sub_xy(zc->offset_u, mul_xy(xy(zc->fb->w, zc->fb->h), set_xy(0.5*zc->iscrscale)));
	zc->corners.p1 = add_xy(zc->offset_u, mul_xy(xy(zc->fb->w, zc->fb->h), set_xy(0.5*zc->iscrscale)));
	zc->corners_dl.p0 = sub_xy(zc->corners.p0, zc->drawlim_u);
	zc->corners_dl.p1 = add_xy(zc->corners.p1, zc->drawlim_u);
	zc->fb->window_dl.p0 = set_xy(-GAUSSRAD_HQ * zc->drawing_thickness);			// drawing limit in pixels
	zc->fb->window_dl.p1 = sub_xy( xy(zc->fb->w-1, zc->fb->h-1), zc->fb->window_dl.p0 );
}

void toggle_guizoom(zoom_t *zc, int on)	// used for temporarily disabling zooming to display objects independently of the zooming system
{
	static double stored_zoomscale=1.;
	static xy_t stored_offset_u;

	if (on)
	{
		zc->zoomscale = stored_zoomscale;
		zc->offset_u = stored_offset_u;
	}
	else
	{
		stored_zoomscale = zc->zoomscale;
		stored_offset_u = zc->offset_u;

		zc->zoomscale = 1.;
		zc->offset_u = XY0;
	}

	calc_screen_limits(zc);
}
