typedef struct
{
	double guiscale, zoomscale, scrscale, iscrscale, scrscale_unzoomed;
	xy_t limit_u, drawlim_u, offset_u;
	rect_t corners, corners_dl;
	int32_t zoom_key_time;
	framebuffer_t *fb;
	mouse_t *mouse;
	double drawing_thickness;
} zoom_t;
