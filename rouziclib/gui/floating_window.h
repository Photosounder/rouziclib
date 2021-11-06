// in gui/controls_struct.h:
// flwindow_t, window_manager_entry_t, window_manager_t

extern void flwindow_init_defaults(flwindow_t *w);
extern void flwindow_init_pinned(flwindow_t *w);
extern void draw_dialog_window_fromlayout(flwindow_t *w, int *diag_on, rect_t *parent_area, gui_layout_t *layout, const int id);

extern int window_register(int priority, void *window_func, void *window_data, rect_t parent_area, int *wind_on, int num_args, ...);
extern void window_manager();
extern int window_find_id_by_func(void *window_func, void *window_data);
extern void window_set_parent(void *window_func, void *window_data, void *parent_window_func, void *parent_window_data);
extern void window_move_up(int id, int offset);
extern void window_move_to_top(void *window_func, void *window_data);
