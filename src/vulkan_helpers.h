VkInstance create_vk_instance();
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);
void PrintAvailibleExtensions();
bool CheckValidationLayerSupport();
struct extension_info get_required_extensions();
void populate_debug_create_info(VkDebugUtilsMessengerCreateInfoEXT* create_info);
void setup_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT* p_debug_messenger);
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);
VkPhysicalDevice pick_physical_device(VkInstance instance);
bool is_device_suitable(VkPhysicalDevice device);
struct queue_family_indices find_queue_families(VkPhysicalDevice device);
VkDevice create_logical_device(VkPhysicalDevice physical_device);

struct queue_family_indices {
	uint32_t graphics_family;
	bool graphics_family_set;
};