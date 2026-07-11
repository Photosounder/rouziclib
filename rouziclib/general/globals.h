extern _Thread_local framebuffer_t *fb;
extern _Thread_local zoom_t zc;
extern _Thread_local mouse_t mouse;
extern vector_font_t *font;
extern double drawing_thickness;
extern _Thread_local audiosys_t audiosys;
extern _Thread_local window_manager_t wind_man;

typedef struct
{
	framebuffer_t *fb;
	zoom_t zc;
	mouse_t mouse;
	textedit_t *cur_textedit;
	textedit_t *prev_textedit;
	textedit_t *next_textedit;
	int initialized;
} rl_gui_context_t;

extern void rl_gui_context_init(rl_gui_context_t *context);
extern void rl_gui_context_destroy(rl_gui_context_t *context);
extern rl_gui_context_t *rl_gui_context_bind(rl_gui_context_t *context);
extern rl_gui_context_t *rl_gui_context_get_bound(void);
