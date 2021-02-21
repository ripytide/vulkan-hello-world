VkInstance create_vk_instance();
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);
void PrintAvailibleExtensions();
bool CheckValidationLayerSupport();
struct extension_info get_required_extensions();
void populate_debug_create_info(VkDebugUtilsMessengerCreateInfoEXT* create_info);
void setup_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT* p_debug_messenger);
VkSurfaceKHR create_surface(VkInstance instance, GLFWwindow *window);
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);
VkPhysicalDevice pick_physical_device(VkInstance instance, VkSurfaceKHR VkSurfaceKHR);
bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface);
bool check_device_extension_support(VkPhysicalDevice device);
struct queue_family_indices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);
VkDevice create_logical_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

//a struct for getting the queue family indicies for certain family types
struct queue_family_indices {
	int family_count;
	uint32_t graphics_family;
	bool graphics_family_set;
	uint32_t presentation_family;
	bool presentation_family_set;
};

//a struct made to allow the get_required_extensions function to give both a list of extensions and
//the number of extensions seeing as it may change at runtime and sizeof is done at compile time i think
struct extension_info {
	const char** extensions;
	uint32_t extension_count;
};