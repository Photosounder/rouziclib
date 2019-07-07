typedef struct
{
	double zoomscale, scrscale, iscrscale, scrscale_unzoomed;
	xy_t limit_u, drawlim_u, offset_u;
	rect_t corners, corners_dl;
	int zoom_key_time, just_reset;
	mouse_t *mouse;
	double drawing_thickness;
} zoom_t;
