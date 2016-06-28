typedef struct
{
	int word_count, *word_start, *word_end;
	double *word_length, max_word_length, full_length, aver_word_length;
} word_stats_t;		// statistics for words in a string

typedef struct
{
	double thresh, min, max;
	int n, min_n, max_n;	// nline counts
} search_thresh_t;

extern double letter_width(vector_font_t *font, double pos, uint32_t c, double scale, const int mode);
extern double calc_strlen_len(vector_font_t *font, uint8_t *string, double scale, const int mode, int32_t len);
extern double calc_strlen(vector_font_t *font, uint8_t *string, double scale, const int mode);
extern word_stats_t make_word_stats(vector_font_t *font, uint8_t *string, const int mode);
extern word_stats_t make_word_stats(vector_font_t *font, uint8_t *string, const int mode);
extern void free_word_stats(word_stats_t ws);
extern double get_word_length(vector_font_t *font, uint8_t *string, word_stats_t ws, const int mode, int iw_start, int iw);
