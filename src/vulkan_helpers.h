//functions

//instance functions
VkInstance create_vk_instance();

//device functions
VkDevice create_logical_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
bool check_device_extension_support(VkPhysicalDevice device);
bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface);
VkPhysicalDevice pick_physical_device(VkInstance instance, VkSurfaceKHR VkSurfaceKHR);
struct queue_family_indices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);

//extension functions
void PrintAvailibleExtensions();
struct extension_info get_required_extensions();

//debug functions
void setup_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT* p_debug_messenger);
void populate_debug_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info);
bool CheckValidationLayerSupport();
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

//swap chain functions
struct swap_chain_info create_swap_chain(VkPhysicalDevice physical_device, VkSurfaceKHR surface, GLFWwindow *window, VkDevice device);
VkImageView *create_image_views(VkImage *images, int image_count, VkFormat format, VkDevice device);
VkSurfaceKHR create_surface(VkInstance instance, GLFWwindow *window);
struct swap_chain_support_details query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface);
VkSurfaceFormatKHR choose_swap_surface_format(VkSurfaceFormatKHR *available_formats, int format_count);
VkPresentModeKHR choose_swap_present_mode(VkPresentModeKHR *availible_modes, int mode_count);
VkExtent2D choose_swap_extent(GLFWwindow *window, VkSurfaceCapabilitiesKHR capabilities);

//graphics pipeline functions
VkPipeline create_graphics_pipeline(VkDevice device, VkExtent2D extent, VkRenderPass render_pass, VkPipelineLayout pipeline_layout);
VkPipelineLayout create_graphics_pipeline_layout(VkDevice device);
VkShaderModule create_shader_module(char *code, long code_size, VkDevice device);

//framebuffers
VkFramebuffer *create_swap_chain_framebuffers(VkDevice device, int image_count, VkRenderPass render_pass, VkImageView *swap_chain_image_views, VkExtent2D extent);

//misc functions

//render pass functions
VkRenderPass create_render_pass(VkFormat format, VkDevice device);

//command stuff
VkCommandPool create_command_pool(VkDevice device, uint32_t queue_index);
VkCommandBuffer *create_command_buffers(VkDevice device, VkCommandPool command_pool, VkRenderPass render_pass, VkPipeline pipeline, VkFramebuffer *framebuffers, VkExtent2D extent, int image_count);

//structs

//a struct for getting the queue family indicies for certain family types
struct queue_family_indices{
	int family_count;
	uint32_t graphics_family;
	bool graphics_family_set;
	uint32_t presentation_family;
	bool presentation_family_set;
};

//a struct made to allow the get_required_extensions function to give both a list of extensions and
//the number of extensions seeing as it may change at runtime and sizeof is done at compile time i think
struct extension_info{
	const char** extensions;
	uint32_t extension_count;
};

//a struct for swap chain details supported needed to create a swap chain
struct swap_chain_support_details{
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR *formats;
	int format_count;
	VkPresentModeKHR *present_modes;
	int present_modes_count;
};

//a struct for swap chain details to pass back from create function
struct swap_chain_info{
	VkSwapchainKHR swap_chain;
	VkImage *images;
	int image_count;
	VkFormat format;
	VkExtent2D extent;
};


//macros

//handy macro for gettin the size of an array in bytes at runtime, this cool
#define ARR_SIZE(x) sizeof(x) / sizeof(x[0])

//WARNING: this has double evaluation so only pass in expressions that don't have side effects as they will happen twice
#define MIN(x, y) ((x < y) ? x : y)

//WARNING: this has double evaluation so only pass in expressions that don't have side effects as they will happen twice
#define MAX(x, y) ((x > y) ? x : y)