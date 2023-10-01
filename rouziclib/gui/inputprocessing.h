// in gui/inputprocessing_struct.h:
// ctrl_button_state_t, ctrl_knob_state_t, ctrl_drag_state_t, ctrl_id_t

extern void ctrl_id_stack_add(mouse_ctrl_id_t *ctrl_id, ctrl_id_t new_ctrl);
extern void ctrl_id_stack_process();
extern int equal_ctrl_id(ctrl_id_t a, ctrl_id_t b, int check_id);
extern int check_ctrl_id(rect_t box, xy_t pos, double radius, enum ctrl_type_t type);
extern void proc_mouse_ctrl_button(int mb, int clicks, ctrl_button_state_t *state, const int cur_point_within_box, const int orig_point_within_box);
extern ctrl_button_state_t *proc_mouse_rect_ctrl_lrmb(rect_t box);
extern ctrl_button_state_t *proc_mouse_polygon_ctrl_lrmb(rect_t box, xy_t *p, int p_count);
extern ctrl_button_state_t *proc_mouse_circ_ctrl_lrmb(xy_t pos, double radius, const int check_bypass);
extern ctrl_knob_state_t proc_mouse_knob_ctrl(rect_t box);
extern int proc_mouse_draggable_ctrl(ctrl_drag_state_t *state, rect_t box);
extern int proc_mouse_xy_ctrl(rect_t box, xy_t *pos, int *lmb, int *rmb);
extern ctrl_button_state_t proc_mouse_circular_ctrl(xy_t *pos, double radius, int dragged);

#define proc_mouse_rect_ctrl(box)	proc_mouse_rect_ctrl_lrmb(box)[0]
#define check_ctrl_id_rect(box)	check_ctrl_id(box, XY0, 0., ctrl_type_rect)
#define check_ctrl_id_circle(pos, radius)	check_ctrl_id(rect(XY0,XY0), pos, radius, ctrl_type_circle)
