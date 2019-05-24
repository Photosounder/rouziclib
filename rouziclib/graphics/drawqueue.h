#include "drawqueue_enums.h"

extern void drawq_reinit(framebuffer_t *fb);
extern void drawq_alloc(framebuffer_t *fb);
extern void drawq_free(framebuffer_t *fb);
extern void drawq_add_layer(uint32_t *dataint, const uint32_t layer_type);
extern void drawq_run(framebuffer_t *fb);
extern int32_t drawq_entry_size(const int32_t type);
extern void *drawq_add_to_main_queue(framebuffer_t fbs, const int dqtype);
extern void drawq_add_sector_id_nopending(framebuffer_t fbs, int32_t sector_id);
extern void drawq_add_sector_id(framebuffer_t fb, int32_t sector_id);
extern void drawq_add_sectors_for_already_set_sectors(framebuffer_t fb);
extern void drawq_compile_lists(framebuffer_t *fb);
extern void drawq_test1(framebuffer_t fb);
extern int drawq_get_bounding_box(framebuffer_t fb, rect_t box, xy_t rad, recti_t *bbi);
extern void drawq_remove_prev_entry_for_sector(framebuffer_t fb, int32_t sector_id, int bracket_search, xyi_t pix_coord);

extern void drawq_bracket_open(framebuffer_t fb);
extern void drawq_bracket_close(framebuffer_t fb, enum dq_blend blending_mode);
