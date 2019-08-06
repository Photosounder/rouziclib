typedef struct
{
	char *path;
	char **lines;
	int linecount;
} pref_file_t;

extern pref_file_t pref_def;

extern void pref_file_load(pref_file_t *pf);
extern int pref_file_save(pref_file_t *pf);
extern void free_pref_file(pref_file_t *pf);
extern pref_file_t pref_set_file_by_path(const char *path);
extern pref_file_t pref_set_file_by_appdata_path(const char *folder);
extern int pref_find_loc(pref_file_t *pf, const char *loc);
extern double pref_get_double(pref_file_t *pf, char *loc, double def_value, const char *suffix);
extern void pref_set_double(pref_file_t *pf, char *loc, double new_value, const char *suffix);
