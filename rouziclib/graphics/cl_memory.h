// in graphics/graphics_struct.h:
// cl_data_alloc_t

#ifdef RL_OPENCL

extern void cl_copy_buffer_to_device(framebuffer_t fb, void *buffer, size_t offset, size_t size);
extern void cl_copy_raster_to_device(framebuffer_t fb, raster_t r, size_t offset);
extern void data_cl_alloc(framebuffer_t *fb, int mb);
extern void data_cl_realloc(framebuffer_t *fb, size_t buffer_size);
extern void cl_data_table_remove_entry(framebuffer_t *fb, int i);
extern void cl_data_table_prune_unused(framebuffer_t *fb);
extern void cl_data_table_remove_entry_by_host_ptr(framebuffer_t *fb, void *host_ptr);
extern uint64_t cl_add_data_table_entry(framebuffer_t *fb, size_t table_index, size_t prev_end, void *buffer, size_t size, int *table_index_p);
extern void cl_data_find_max_free_space(framebuffer_t *fb);
extern int cl_data_check_enough_room(framebuffer_t *fb, size_t align_size, size_t buffer_size);

#endif

extern uint64_t cl_add_buffer_to_data_table(framebuffer_t *fb, void *buffer, size_t buffer_size, size_t align_size, int *table_index);
extern uint64_t cl_add_raster_to_data_table(framebuffer_t *fb, raster_t *r);
