extern double find_line_for_thresh(vector_font_t *font, uint8_t *string, word_stats_t ws, const int mode, double thresh, int iw_start, int *iw_end);
extern int find_line_count_for_thresh(vector_font_t *font, uint8_t *string, word_stats_t ws, const int mode, double thresh, double *maxwidth);
extern int search_thresh(vector_font_t *font, search_thresh_t *st, double maxwidth, int nlines, double prec);
extern double find_string_width_for_nlines(vector_font_t *font, uint8_t *string, word_stats_t ws, int *nlines, const int mode, double *lower_bound);
extern double find_best_string_width(vector_font_t *font, uint8_t *string, word_stats_t ws, const int mode, xy_t boxdim, int *nlines, double *scale_ratio);
extern void draw_string_maxwidth(vector_font_t *font, uint8_t *string, word_stats_t ws, xy_t box0, xy_t box1, double scale, lrgb_t colour, double intensity, const int mode, double maxwidth, int nlines);
extern void draw_string_bestfit(vector_font_t *font, uint8_t *string, xy_t box0, xy_t box1, const double border, const double scale, lrgb_t colour, double intensity, const int mode);
