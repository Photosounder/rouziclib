#ifdef RL_TRUETYPE

extern vector_font_t *load_truetype_font_as_triangles(const uint8_t *data, size_t size, xyi_t *range, int range_count, int quality);
extern polynomial_grid_t mesh_to_polynomial(vobj_tri_t *mesh, double radius, double max_grid_step, int degree);
extern void font_gen_polynomial_grids(vector_font_t *font, double asc_height_px, double radius);

#endif

extern void draw_polynomial_grid_lrgb(polynomial_grid_t *grid, xy_t pos, double scale, double angle, lrgb_t colour);
extern void draw_polynomial_grid(polynomial_grid_t *grid, xy_t pos, double scale, double angle, col_t colour);
