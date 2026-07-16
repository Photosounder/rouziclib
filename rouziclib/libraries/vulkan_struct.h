#ifdef RL_VULKAN

#if defined(_WIN32) && !defined(VK_USE_PLATFORM_WIN32_KHR)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#ifdef RL_SDL
#if RL_SDL == 3
#include <SDL3/SDL_vulkan.h>
#else
#include <SDL2/SDL_vulkan.h>
#endif
#endif

#include <vulkan/vulkan.h>

#define RL_VK_FRAMES_IN_FLIGHT 2

typedef struct
{
	VkBuffer buffer;
	VkDeviceMemory memory;
	void *mapped;
	VkDeviceSize capacity;
} vk_buffer_t;

typedef struct
{
	VkCommandBuffer command_buffer;
	VkFence fence;
	VkSemaphore image_available;
	VkSemaphore render_finished;
	VkQueryPool timestamp_pool;
	VkImage output_image;
	VkDeviceMemory output_memory;
	VkImageView output_view;
	VkDescriptorSet descriptor_set;
	vk_buffer_t staging;
	vk_buffer_t drawq;
	vk_buffer_t sector_pos;
	vk_buffer_t entry_list;
	int output_layout_initialised;
	int timestamp_results_pending;
	size_t timing_index;
} vk_frame_t;

typedef struct
{
	VkInstance instance;
	VkSurfaceKHR surface;
	#ifdef _WIN32
	// Keep Vulkan WSI separate from the OpenGL top-level window
	HWND surface_window;
	#endif
	VkPhysicalDevice gpu;
	VkDevice device;
	VkQueue queue;
	uint32_t queue_family;
	VkCommandPool command_pool;

	VkSwapchainKHR swapchain;
	VkFormat swapchain_format;
	VkExtent2D swapchain_extent;
	VkImage *swapchain_images;
	uint32_t swapchain_image_count;
	uint32_t timestamp_valid_bits;
	float timestamp_period;

	VkDescriptorSetLayout descriptor_layout;
	VkDescriptorPool descriptor_pool;
	VkPipelineLayout pipeline_layout;
	VkPipeline compute_pipeline;

	vk_buffer_t resource_data;
	vk_frame_t frame[RL_VK_FRAMES_IN_FLIGHT];
	uint32_t frame_index;
	size_t dirty_start;
	size_t dirty_end;
	int swapchain_dirty;
	int initialised;
} vk_info_t;
#endif
