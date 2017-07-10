typedef struct
{
	int over, down, once, uponce;
} ctrl_button_state_t;

typedef struct
{
	double vert_delta;
	int down, uponce;
} ctrl_knob_state_t;

typedef struct
{
	xy_t pos, offset, freedom;
	int down;
} ctrl_drag_state_t;

typedef struct
{
	int id;		// identifies different controls with the same box
	rect_t box;	// rect coordinates that define the control
} ctrl_id_t;
