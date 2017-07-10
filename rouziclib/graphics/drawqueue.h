#include "drawqueue_enums.h"

extern void drawq_reinit(raster_t *fb);
extern void drawq_alloc(raster_t *fb, int size);
extern void drawq_free(raster_t *fb);
extern void drawq_add_layer(uint32_t *dataint, const uint32_t layer_type);
extern void drawq_run(raster_t *fb);
extern int32_t drawq_entry_size(const int32_t type);
extern void *drawq_add_to_main_queue(raster_t fb, const int dqtype);
extern void drawq_add_sector_id(raster_t fb, int32_t sector_id);
extern void drawq_compile_lists(raster_t *fb);
extern void drawq_bracket_open(raster_t fb);
extern void drawq_bracket_close(raster_t fb, int32_t blending_mode);
extern void drawq_test1(raster_t fb);
