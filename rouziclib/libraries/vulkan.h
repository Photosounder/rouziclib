#ifdef RL_VULKAN

#ifdef PRAGMA_COMMENT
#pragma comment (lib, "vulkan-1.lib")
#endif

extern const char *get_vk_error_string(VkResult err);
extern int vk_init();
extern void vk_suspend();
extern void vk_hide_surface_window();
extern void vk_deinit();
extern void vk_wait_idle();
extern int vk_data_buffer_resize(size_t size);
extern void vk_data_buffer_free();
extern void vk_mark_data_dirty(size_t offset, size_t size);
extern void vk_mark_swapchain_dirty();
extern int vk_drawq_run();

#define VK_ERR_RET(name, ret) if ((ret) != VK_SUCCESS) { fprintf_rl(stderr, "%s failed (err %d: %s)\n", name, (int) (ret), get_vk_error_string(ret)); return ret; }
#define VK_ERR_NORET(name, ret) if ((ret) != VK_SUCCESS) { fprintf_rl(stderr, "%s failed (err %d: %s)\n", name, (int) (ret), get_vk_error_string(ret)); return; }

#endif
