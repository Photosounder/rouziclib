extern xy_t offset_scale2(xy_t pos, xy_t offset, xy_t sm2);
extern xy_t offset_scale(xy_t pos, xy_t offset, double sm);
extern rect_t offset_scale2_rect(rect_t r, xy_t offset, xy_t sm2);
extern rect_t offset_scale_rect(rect_t r, xy_t offset, double sm);
extern xy_t offset_scale_inv(xy_t pos, xy_t offset, double sm);
extern rect_t offset_scale_inv_rect(rect_t r, xy_t offset, double sm);
extern xy_t fit_unscaled_rect(rect_t a, rect_t f, double *sm);
extern void area_to_area_transform(rect_t a, rect_t b, xy_t *tmul, xy_t *tadd);
extern void rect_code_tool_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, double drawing_thickness, xy_t offset, double sm);

#define rect_code_tool(offset, sm)	rect_code_tool_fullarg(fb, zc, mouse, drawing_thickness, offset, sm)
