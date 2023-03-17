#define ZOOM_Q

typedef struct
{
	double zoomscale, scrscale, iscrscale, scrscale_raw, iscrscale_raw;
	xy_t limit_u, drawlim_u, offset_u;
	xyq_t offset_uq;
	rect_t corners, corners_dl;
	int zoom_key_time, just_reset, overlay_ctrl;
	mouse_t *mouse;
	double drawing_thickness;
} zoom_t;
