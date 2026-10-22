extern const char *wahe_rl_parse_inputs(const char *line, int *received_input);
extern void wahe_rl_input_init(void);
extern void wahe_rl_input_deinit(void);
extern void wahe_rl_frame_begin(framebuffer_t *target);
extern void wahe_rl_frame_end(void);
extern int wahe_rl_get_scancode_by_key_name(const char *name);
extern void wahe_rl_module_init(xyi_t fb_dim, const int mode);
extern char *wahe_rl_file_dialog(const char *filter, int save);
