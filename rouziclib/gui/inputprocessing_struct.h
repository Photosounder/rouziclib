typedef struct
{
	int over, down, once, uponce, doubleclick, orig, out_of_screen, too_small;
} ctrl_button_state_t;

typedef struct
{
	double vert_delta;
	int down, downonce, uponce, doubleclick, rightclick;
} ctrl_knob_state_t;

typedef struct
{
	xy_t pos, dim, offset, freedom, click_offset;
	int down, over, uponce;
} ctrl_drag_state_t;

typedef struct
{
	int open, next_open, sel_id, hover_id, count;
} ctrl_selectmenu_state_t;

enum ctrl_type_t
{
	ctrl_type_rect,
	ctrl_type_polygon,
	ctrl_type_circle
};

#ifndef CTRL_ID_VERTEX_AS
#define CTRL_ID_VERTEX_AS 4
#endif

typedef struct
{
	enum ctrl_type_t type;
	int id;				// identifies different controls with the same characteristics
	rect_t box;			// rect coordinates that define either the rectangular control or the bounding box of the polygonal control
	xy_t pos;			// position of the circle
	double radius;			// radius of the circle
	xy_t vertex[CTRL_ID_VERTEX_AS];	// Vertices that define a polygonal control
	int vertex_count;
} ctrl_id_t;

typedef struct
{
	int init, dragged, prev_dragged;
	ctrl_drag_state_t drag[9];
} ctrl_resize_rect_t;
