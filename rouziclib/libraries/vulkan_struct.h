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

#define RL_VK_FRAMES_IN_FLIGHT 1
#define RL_VK_DIRTY_RANGES 32

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
	int timeout_reported;
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
	VkImageView *swapchain_views;	// only when the compute shader writes the swapchain images directly
	VkSemaphore *swapchain_render_finished;
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
	size_t dirty_start[RL_VK_DIRTY_RANGES];	// the parts of the resource arena to upload with the next frame, in order and apart
	size_t dirty_end[RL_VK_DIRTY_RANGES];
	int dirty_count;
	int unified_memory;		// integrated GPU, whose memory the CPU can write directly
	int swapchain_storage;		// the compute shader writes the swapchain images through swapchain_views
	int swapchain_dirty;
	int upscale_nearest;		// with a pixel scale, keep the swapchain at the window's size and blit the framebuffer up into it with
					// nearest-neighbour, instead of a swapchain of the framebuffer's size that the compositor scales up smoothly
	int initialised;

	// Presentation timing (VK_GOOGLE_display_timing, Android), so that the caller can start each frame as late as possible and keep
	// as few frames waiting for the display as possible, see vk_present_timing()
	int display_timing;
	PFN_vkGetPastPresentationTimingGOOGLE get_past_presentation_timing;
	PFN_vkGetRefreshCycleDurationGOOGLE get_refresh_cycle_duration;
	uint32_t present_id;		// the id of the latest frame presented, counting from 1
	uint32_t shown_id;		// the latest frame known to have been shown, 0 before the first, and when (CLOCK_MONOTONIC)
	uint64_t shown_ns;
	uint64_t refresh_ns;		// the display's refresh period, 0 when unknown
	int refresh_check;		// presents since the refresh period was last asked for
	uint64_t queued_ns[64];		// when each of the latest frames was queued, by id modulo 64
	uint64_t lead_ns;		// how long before being shown a frame must be queued at least (the second longest of the latest 32)
	uint64_t lead_recent[32];	// the latest frames' leads, by count modulo 32
	uint32_t lead_count;
	uint64_t desired_present_ns;	// when the next frame presented should be shown at the earliest, 0 for as soon as possible
} vk_info_t;
#endif
