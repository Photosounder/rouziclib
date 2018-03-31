// in gui/zoom_struct.h:
// zoom_t

extern zoom_t init_zoom(framebuffer_t *fb, mouse_t *mouse, double drawing_thickness);
extern double to_screen_coord_x(zoom_t zc, double x);
extern double to_screen_coord_y(zoom_t zc, double y);
extern xy_t to_screen_coord_xy(zoom_t zc, xy_t p);
extern double to_world_coord_x(zoom_t zc, double x);
extern double to_world_coord_y(zoom_t zc, double y);
extern xy_t to_world_coord_xy(zoom_t zc, xy_t p);
extern rect_t to_screen_coord_rect(zoom_t zc, rect_t r);
extern rect_t to_world_coord_rect(zoom_t zc, rect_t r);
extern void zoom_key_released(zoom_t *zc, int8_t *flag_zoom_key, int source);
extern void zoom_wheel(zoom_t *zc, int8_t flag_zoom_key, int y);
extern void calc_screen_limits(zoom_t *zc);
extern void toggle_guizoom(zoom_t *zc, int on);

#define sc_x(p) to_screen_coord_x(zc, p)
#define sc_y(p) to_screen_coord_y(zc, p)
#define sc_xy(p) to_screen_coord_xy(zc, p)
#define sc_rect(r) to_screen_coord_rect(zc, r)
#define wc_x(p) to_world_coord_x(zc, p)
#define wc_y(p) to_world_coord_y(zc, p)
#define wc_xy(p) to_world_coord_xy(zc, p)
#define wc_rect(r) to_world_coord_rect(zc, r)
