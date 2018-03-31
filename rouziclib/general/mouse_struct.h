typedef struct
{
	ctrl_id_t hover, hover_new, current;
	rect_t box;
} mouse_ctrl_id_t;

typedef struct
{
	int lmb, mmb, rmb, wheel, clicks;
	xy_t orig;		// coordinates of the original click
} mousebut_t;

enum
{
	mouse_mod_ctrl,
	mouse_mod_alt,
	mouse_mod_shift,
	mouse_mod_gui,
	mouse_mod_count
};

typedef struct
{
	xy_t a, u, d, du, u_stored, prev_u;
	int window_focus_flag, mouse_focus_flag, window_minimised_flag, zoom_flag;
	int8_t mod_key[mouse_mod_count];
	mousebut_t b;
	mouse_ctrl_id_t *ctrl_id;
} mouse_t;
