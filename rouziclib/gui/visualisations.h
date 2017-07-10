extern void draw_graph_data_bar_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, rect_t area, float *data, float *label_data, int count, const char *format, double min_data_height, double max_data_height, int norm, double bar_width, col_t col);

#define draw_graph_data_bar(area, data, label_data, count, format, min_data_height, max_data_height, norm, bar_width, col)	draw_graph_data_bar_fullarg(fb, zc, mouse, font, drawing_thickness, area, data, label_data, count, format, min_data_height, max_data_height, norm, bar_width, col)
