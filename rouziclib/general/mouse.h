// in general/mouse_struct.h:
// mouse_ctrl_id_t, mousebut_t, mouse_t

extern mouse_t init_mouse();
extern void mouse_pre_event_proc(mouse_t *mouse);
extern void mouse_post_event_proc(mouse_t *mouse, zoom_t *zc);
