#define ZOOM_Q

typedef struct
{
	double zoomscale, scrscale, iscrscale;
	xy_t limit_u, drawlim_u, offset_u;
	xyq_t offset_uq;
	rect_t corners, corners_dl;
	double zoom_key_time;
	int just_reset, overlay_ctrl;
	mouse_t *mouse;
	double drawing_thickness;
} zoom_t;

typedef struct
{
	int focus_index;
	xy_t pos0, pos1;
	double zoomscale_base, zoomscale_top;
	double time0, time1, time2, time3;
	double result_id_v;
	int zoom_transition_on, filter_count;
} filter_focus_t;
