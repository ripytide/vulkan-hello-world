//plain old C headers
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//GLFW header
#define GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "vulkan_helpers.h"
#include "basic_helpers.h"

//the layers/extensions wanted on top of the GLFW required extensions
const char *validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
const char *other_extensions[] = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
const char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

//enables validation layers depending of whether it was compiled in debug mode of not
#ifdef NDEBUG
	static const bool enableValidationLayers = false;
#else
	static const bool enableValidationLayers = true;
#endif

GLFWwindow* InitialiseGLFW(uint32_t width, uint32_t height) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	return(glfwCreateWindow(width, height, "Vulkan", NULL, NULL));
}

VkInstance create_vk_instance() {
	if (enableValidationLayers && !CheckValidationLayerSupport()) {
		printf("Error: Validation layers requested but not found!");
	}
	
	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Hello Triangle",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0
	};

	struct extension_info ext = get_required_extensions();

	VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app_info,
		.enabledExtensionCount = ext.extension_count,
		.ppEnabledExtensionNames = ext.extensions,
	};

	
	VkDebugUtilsMessengerCreateInfoEXT messenger_info = {0};
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = ARR_SIZE(validation_layers);
		createInfo.ppEnabledLayerNames = validation_layers;
		populate_debug_create_info(&messenger_info);

		createInfo.pNext = &messenger_info;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = NULL;
	}

	VkInstance instance;
	if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS)
		printf("Error creating vk instance");
	return instance;
}

void populate_debug_create_info(VkDebugUtilsMessengerCreateInfoEXT* create_info) {
	create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info->pNext = NULL;
	create_info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info->pfnUserCallback = debug_callback;
	create_info->pUserData = NULL; // Optional
}



void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func) {
		func(instance, debugMessenger, pAllocator);
	}
}

void PrintAvailibleExtensions() {
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
	VkExtensionProperties *extensions = malloc(sizeof *extensions * extensionCount);

	if (!extensions) {
		printf("NULL pointer extensions");
		free(extensions);
		return;
	}

	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);
	
	printf("You have %d installed Vulkan Extensions\n", extensionCount);
	printf("Currently installed Vulkan Extensions:\n");
	for (unsigned int i = 0; i < extensionCount; i++)
	{
		printf("%s\n", extensions[i].extensionName);
	}

	free(extensions);
}

bool CheckValidationLayerSupport() {
	//this functions takes the layers defined at the top of this file and checks them agaist the availible layers on the system
	//if any one layer defined at the top is not found of the system then false is returned; true returned if all layers found
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);
	VkLayerProperties* AvailibleLayers = malloc(sizeof *AvailibleLayers * layerCount);

	if (!AvailibleLayers) {
		printf("Null pointer AvailibleLayers");
		free(AvailibleLayers);
		return false;
	}

	vkEnumerateInstanceLayerProperties(&layerCount, AvailibleLayers);

	for (unsigned int i = 0; i < ARR_SIZE(validation_layers); i++)
	{
		bool found = false;
		for (unsigned int j = 0; j < layerCount; j++)
		{
			if (strcmp(validation_layers[i], AvailibleLayers[j].layerName) == 0) {
				found = true;
			}
		}
		if (!found) {
			free(AvailibleLayers);
			return false;
		}
	}
	free(AvailibleLayers);
	return true;
}

struct extension_info get_required_extensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	uint32_t total_extension_count = glfwExtensionCount;
	if (enableValidationLayers) {
		total_extension_count += ARR_SIZE(other_extensions);
	}
	
	const char** all_extensions = malloc(sizeof *all_extensions * total_extension_count);

	if (!all_extensions) {
		printf("Null pointer all_extensions");
		struct extension_info blank = {
			.extensions = NULL,
			.extension_count = 0
			};
		free(all_extensions);
		return(blank);
	}

	for (unsigned int i = 0; i < glfwExtensionCount; i++)
	{
		all_extensions[i] = glfwExtensions[i];
	}

	for (unsigned int i = 0; i < ARR_SIZE(other_extensions); i++) {
		all_extensions[i + glfwExtensionCount] = other_extensions[i];
	}

	struct extension_info ext = {
		.extensions = all_extensions,
		.extension_count = total_extension_count,
	};

	return ext;
}

void setup_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT* p_debug_messenger) {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT create_info = { 0 };
	populate_debug_create_info(&create_info);
	
	if (CreateDebugUtilsMessengerEXT(instance, &create_info, NULL, p_debug_messenger) != VK_SUCCESS) {
		printf("Error: failed to set up debug messenger!");
	}
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

VkSurfaceKHR create_surface(VkInstance instance, GLFWwindow* window){
	VkSurfaceKHR surface;

	if (glfwCreateWindowSurface(instance, window, NULL, &surface) != VK_SUCCESS)
		printf("Error: failed to create window surface");

	return surface;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {

	static int count = 0;

	printf("Current Count: %d\tSeverity: %d\tType: %d\tMessage: %s\n\n", ++count, messageSeverity, messageType, pCallbackData->pMessage);

	return VK_FALSE;
}


VkPhysicalDevice pick_physical_device(VkInstance instance, VkSurfaceKHR surface){
	VkPhysicalDevice device = VK_NULL_HANDLE;
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(instance, &device_count, NULL);
	if (device_count == 0)
		printf("Error: No physical devices found");
	VkPhysicalDevice *devices = malloc(sizeof *devices * device_count);

	vkEnumeratePhysicalDevices(instance, &device_count, devices);

	for (unsigned int i = 0; i < device_count; i++){
		if (is_device_suitable(devices[i], surface))
			device = devices[i];
	}

	if (device == VK_NULL_HANDLE)
		printf("Error: No suitible device found");

	free(devices);
	return device;
}

bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface){
	VkPhysicalDeviceProperties device_properties;
   VkPhysicalDeviceFeatures device_features;
   vkGetPhysicalDeviceProperties(device, &device_properties);
   vkGetPhysicalDeviceFeatures(device, &device_features);

	struct queue_family_indices indices = find_queue_families(device, surface);
	struct swap_chain_support_details details = query_swap_chain_support(device, surface);


	bool queue_adequate = indices.graphics_family_set && indices.presentation_family_set;
	bool device_extension_support = check_device_extension_support(device);

	bool swap_chain_adaquate = false;
	if (device_extension_support){
		swap_chain_adaquate = details.format_count && details.present_modes_count;
	}

	bool features_adaquate = device_features.geometryShader;
	bool type_adaquate = device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

	bool device_adaquate = queue_adequate && swap_chain_adaquate && device_extension_support && features_adaquate && type_adaquate;

	return device_adaquate;
}

bool check_device_extension_support(VkPhysicalDevice device){
	uint32_t extension_count = 0;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);

	VkExtensionProperties *extensions = malloc(sizeof *extensions * extension_count);
	vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, extensions);

	bool found;
	for (unsigned int i = 0; i < ARR_SIZE(device_extensions); i++) {
		found = false;
		for (unsigned int j = 0; j < extension_count; j++) {
			if (strcmp(device_extensions[i], extensions[j].extensionName) == 0)
				found = true;
		}
		if (!found) {
			return false;
		}
	}
	return true;
}

struct queue_family_indices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface){
	struct queue_family_indices indices = {0};

	indices.family_count = 2;

	uint32_t family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, NULL);

	VkQueueFamilyProperties *families = malloc(sizeof *families * family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, families);

	VkBool32 presentation_support;

	for (unsigned int i = 0; i < family_count; i++){
		if(families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
			indices.graphics_family = i;
			indices.graphics_family_set = true;
		}

		presentation_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentation_support);

		if (presentation_support){
			indices.presentation_family = i;
			indices.presentation_family_set = true;
		}

		if (indices.graphics_family_set && indices.presentation_family_set){
			free(families);
			return indices;
		}
		
	}

	printf("Error: not all queue families found");
	free(families);
	return indices;
}

VkDevice create_logical_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface){
	//gets queue indices
	struct queue_family_indices indices = find_queue_families(physical_device, surface);

	int family_array[] = {indices.graphics_family, indices.presentation_family};

	VkDeviceQueueCreateInfo *queue_create_infos = malloc(sizeof *queue_create_infos * indices.family_count);

	float queue_priority = 1.0f;

	bool duplicate_found;
	int unique_family_count = 0;

	//search through all previous families to make sure that family hasnt already been added
	//for every unique family queue give it a create info and store it in the create info array
	for (int i = 0; i < indices.family_count; i++){
		duplicate_found = false;
		for (int j = 0; j < i; j++){
			if (family_array[j] == family_array[i])
				duplicate_found = true;
		}

		if (!duplicate_found){
			queue_create_infos[unique_family_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_infos[unique_family_count].queueFamilyIndex = family_array[i];
			queue_create_infos[unique_family_count].queueCount = 1;
			queue_create_infos[unique_family_count].pQueuePriorities = &queue_priority;
			queue_create_infos[unique_family_count].pNext = NULL;
			queue_create_infos[unique_family_count].flags = 0;

			unique_family_count++;
		}
	}

	VkPhysicalDeviceFeatures device_features = {VK_FALSE};

	VkDeviceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pQueueCreateInfos = queue_create_infos,
		.queueCreateInfoCount = unique_family_count,
		.pEnabledFeatures = &device_features,
		.enabledExtensionCount = ARR_SIZE(device_extensions),
		.ppEnabledExtensionNames = device_extensions
	};

	if (enableValidationLayers) {
		create_info.enabledLayerCount = ARR_SIZE(validation_layers);
		create_info.ppEnabledLayerNames = validation_layers;
	} else {
		create_info.enabledLayerCount = 0;
	}

	VkDevice device;

	if (vkCreateDevice(physical_device, &create_info, NULL, &device) != VK_SUCCESS)
		printf("Error: Failed to create logical device");

	return device;
}

struct swap_chain_support_details query_swap_chain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface){
	struct swap_chain_support_details details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);

	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, NULL);
	if (format_count){
		details.formats = malloc(sizeof *details.formats * format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, details.formats);
		details.format_count = format_count;
	}

	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL);
	if (present_mode_count){
		details.present_modes = malloc(sizeof *details.present_modes * present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, details.present_modes);
		details.present_modes_count = present_mode_count;
	}

	return details;
}

VkSurfaceFormatKHR choose_swap_surface_format(VkSurfaceFormatKHR *available_formats, int format_count){
	//try and find a srgb formal
	for (int i = 0; i < format_count; i++){
		if (available_formats[i].format == VK_FORMAT_B8G8R8_SRGB && available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return available_formats[i];
	}
	//if our specific format is not found it is usally alright to just use any format
	return available_formats[0];
}

VkPresentModeKHR choose_swap_present_mode(VkPresentModeKHR *availible_modes, int mode_count){
		//FIFO is the only mode guaranteed to be available
		//TODO implement mode selection for v-sync triple sync etc
		return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D choose_swap_extent(GLFWwindow *window, VkSurfaceCapabilitiesKHR capabilities){
	if (capabilities.currentExtent.width != UINT32_MAX){
		return capabilities.currentExtent;
	} else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actual_extent = {
			.width = (uint32_t)width,
			.height = (uint32_t)height
		};
		
		//clamp the width/height to the min max range given in the capabilities
		actual_extent.width = MAX(capabilities.minImageExtent.width, MIN(capabilities.maxImageExtent.width, actual_extent.width));
		actual_extent.height = MAX(capabilities.minImageExtent.height, MIN(capabilities.maxImageExtent.height, actual_extent.height));

		return actual_extent;
	}
}

struct swap_chain_info create_swap_chain(VkPhysicalDevice physical_device, VkSurfaceKHR surface, GLFWwindow *window, VkDevice device) {
	struct swap_chain_support_details details = query_swap_chain_support(physical_device, surface);

	VkSurfaceFormatKHR surface_format = choose_swap_surface_format(details.formats, details.format_count);
	VkPresentModeKHR present_mode = choose_swap_present_mode(details.present_modes, details.present_modes_count);
	VkExtent2D extent = choose_swap_extent(window, details.capabilities);

	uint32_t min_image_count = details.capabilities.minImageCount + 1;

	if (details.capabilities.maxImageCount > 0 && min_image_count > details.capabilities.maxImageCount){
		min_image_count = details.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = surface;

	create_info.minImageCount = min_image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	struct queue_family_indices indices = find_queue_families(physical_device, surface);
	uint32_t queue_family_indices[] = {indices.graphics_family, indices.presentation_family};

	if (indices.graphics_family != indices.presentation_family){
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices;
	} else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0; //optional
		create_info.pQueueFamilyIndices = NULL; //optional
	}

	// if we want to apply stuff like 90 rotations we can do so here (if supported)
	create_info.preTransform = details.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	create_info.presentMode = present_mode;
	//if some pixels are obscurred then we dont care about them which is better for performance
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	VkSwapchainKHR swap_chain;

	if (vkCreateSwapchainKHR(device, &create_info, NULL, &swap_chain) != VK_SUCCESS){
		printf("Error: Unable to create swap chain");
	}

	uint32_t image_count = 0;
	vkGetSwapchainImagesKHR(device, swap_chain, &image_count, NULL);
	VkImage *images = malloc(sizeof *images * image_count);
	vkGetSwapchainImagesKHR(device, swap_chain, &image_count, images);

	struct swap_chain_info info = {
		.swap_chain = swap_chain,
		.images = images,
		.image_count = image_count,
		.format = surface_format.format,
		.extent = extent
	};

	return info;
}

VkImageView *create_image_views(VkImage *images, int image_count, VkFormat format, VkDevice device){
	VkImageView *image_views = malloc(sizeof *image_views * image_count);

	for (int i = 0; i < image_count; i++){
		VkImageViewCreateInfo create_info = {0};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.image = images[i];
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format = format;
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &create_info, NULL, &image_views[i]) != VK_SUCCESS){
			printf("Error: failed to create image view: %d\n", i);
		}
	}

	return image_views;
}

VkRenderPass create_render_pass(VkFormat format, VkDevice device){
	VkSubpassDependency dependency = {0};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


	VkAttachmentDescription color_attachment = {0};
	color_attachment.format = format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {0};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkRenderPassCreateInfo render_pass_create_info = {0};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = 1;
	render_pass_create_info.pAttachments = &color_attachment;
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass;
	render_pass_create_info.dependencyCount = 1;
	render_pass_create_info.pDependencies = &dependency;

	VkRenderPass render_pass;
	if (vkCreateRenderPass(device, &render_pass_create_info, NULL, &render_pass) != VK_SUCCESS) {
		printf("Error: failed to create render pass");
	}
	return render_pass;
}

VkPipelineLayout create_graphics_pipeline_layout(VkDevice device){
	VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pushConstantRangeCount = 0;

	VkPipelineLayout pipeline_layout;
	if (vkCreatePipelineLayout(device, &pipeline_layout_info, NULL, &pipeline_layout) != VK_SUCCESS) {
		printf("Error: failed to create pipeline layout");
	}
	return pipeline_layout;
}

VkPipeline create_graphics_pipeline(VkDevice device, VkExtent2D extent, VkRenderPass render_pass, VkPipelineLayout pipeline_layout){
	//no need to null terminate as we will be explicit about length later
	char *vert_shader_code = read_file("shaders/vert.spv", false);
	long vert_shader_length = get_length("shaders/vert.spv");
	char *frag_shader_code = read_file("shaders/frag.spv", false);
	long frag_shader_length = get_length("shaders/frag.spv");

	VkShaderModule vert_shader_module = create_shader_module(vert_shader_code, vert_shader_length, device);
	VkShaderModule frag_shader_module = create_shader_module(frag_shader_code, frag_shader_length, device);

	VkPipelineShaderStageCreateInfo vert_shader_stage_info = {0};
	vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module = vert_shader_module;
	vert_shader_stage_info.pName = "main";
	//use for eificient constant definition at pipeline creation time
	vert_shader_stage_info.pSpecializationInfo = NULL;

	VkPipelineShaderStageCreateInfo frag_shader_stage_info = {0};
	frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module = frag_shader_module;
	frag_shader_stage_info.pName = "main";
	//use for eificient constant definition at pipeline creation time
	frag_shader_stage_info.pSpecializationInfo = NULL;

	VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};
	
	VkPipelineVertexInputStateCreateInfo vertex_input_info = {0};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.vertexAttributeDescriptionCount = 0;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {0};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) extent.width;
	viewport.height = (float) extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {0};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = extent;

	VkPipelineViewportStateCreateInfo viewport_state = {0};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {0};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {0};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo color_blending = {0};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;

	VkGraphicsPipelineCreateInfo pipeline_info = {0};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = NULL;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDynamicState = NULL;

	pipeline_info.layout = pipeline_layout;
	pipeline_info.renderPass = render_pass;
	pipeline_info.subpass = 0;

	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.basePipelineIndex = -1;

	VkPipeline graphics_pipeline;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &graphics_pipeline) != VK_SUCCESS){
		printf("Error: failed to create graphics pipeline");
	}

	vkDestroyShaderModule(device, vert_shader_module, NULL);
	vkDestroyShaderModule(device, frag_shader_module, NULL);

	return graphics_pipeline;
}

VkShaderModule create_shader_module(char *code, long code_size, VkDevice device){
	VkShaderModuleCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = code_size;

	//TODO learn the reason for this cast, may be a source of bugs
	create_info.pCode = (uint32_t*) code;

	VkShaderModule shader_module;
	if (vkCreateShaderModule(device, &create_info, NULL, &shader_module) != VK_SUCCESS){
		printf("Error: failed to create shader module");
	}

	free(code);

	return shader_module;
}

VkFramebuffer *create_swap_chain_framebuffers(VkDevice device, int image_count, VkRenderPass render_pass, VkImageView *swap_chain_image_views, VkExtent2D extent){
	VkFramebuffer *framebuffers = malloc(sizeof *framebuffers * image_count);

	for (int i = 0; i < image_count; i++){

		VkImageView attachments[] = {swap_chain_image_views[i]};

		VkFramebufferCreateInfo framebuffer_create_info = {0};
		framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.renderPass = render_pass;
		framebuffer_create_info.attachmentCount = 1;
		framebuffer_create_info.pAttachments = attachments;
		framebuffer_create_info.width = extent.width;
		framebuffer_create_info.height = extent.height;
		framebuffer_create_info.layers = 1;

		if (vkCreateFramebuffer(device, &framebuffer_create_info, NULL, &framebuffers[i])){
			printf("Error: failed to create framebuffer: %d", i);
		}
	}

	return framebuffers;
}

VkCommandPool create_command_pool(VkDevice device, uint32_t queue_index){
	VkCommandPoolCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.queueFamilyIndex = queue_index;
	create_info.flags = 0;

	VkCommandPool command_pool;
	if (vkCreateCommandPool(device, &create_info, NULL, &command_pool) != VK_SUCCESS){
		printf("Error: failed to create command pool");
	}
	return command_pool;
}

VkCommandBuffer *create_command_buffers(VkDevice device, VkCommandPool command_pool, VkRenderPass render_pass, VkPipeline pipeline, VkFramebuffer *framebuffers, VkExtent2D extent, int image_count){
	VkCommandBuffer *command_buffers = malloc(sizeof *command_buffers * image_count);

	VkCommandBufferAllocateInfo alloc_info = {0};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t)image_count;

	if (vkAllocateCommandBuffers(device, &alloc_info, command_buffers) != VK_SUCCESS){
		printf("Error: failed to allocate command buffers");
	}

	for (int i = 0; i < image_count; i++){
		VkCommandBufferBeginInfo begin_info = {0};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0;
		begin_info.pInheritanceInfo = NULL;

		if (vkBeginCommandBuffer(command_buffers[i], &begin_info) != VK_SUCCESS){
			printf("Error: failed to being recording command buffer: %d", i);
		}

		VkRenderPassBeginInfo render_pass_begin_info = {0};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = render_pass;
		render_pass_begin_info.framebuffer = framebuffers[i];
		render_pass_begin_info.renderArea.offset.x = 0;
		render_pass_begin_info.renderArea.offset.y = 0;
		render_pass_begin_info.renderArea.extent = extent;

		VkClearValue clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
		render_pass_begin_info.clearValueCount = 1;
		render_pass_begin_info.pClearValues = &clear_color;

		vkCmdBeginRenderPass(command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		vkCmdDraw(command_buffers[i], 3, 1, 0, 0); //holy balls this is it

		vkCmdEndRenderPass(command_buffers[i]);

		if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS){
			printf("Error: failed to record command buffer: %d", i);
		}
	}

	return command_buffers;
}

VkSemaphore create_semaphore(VkDevice device){
	VkSemaphoreCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkSemaphore semaphore;
	if (vkCreateSemaphore(device, &create_info, NULL, &semaphore) != VK_SUCCESS){
		printf("Error: failed to create semaphore");
	}

	return semaphore;
}

void draw_frame(VkDevice device, VkQueue graphics_queue, VkQueue presentation_queue, VkSwapchainKHR swap_chain, VkCommandBuffer *command_buffers, VkSemaphore image_availible_semaphore, VkSemaphore render_finished_semaphore){
	uint32_t image_index;
	vkAcquireNextImageKHR(device, swap_chain, UINT32_MAX, image_availible_semaphore, VK_NULL_HANDLE, &image_index);

	VkSubmitInfo submit_info = {0};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = {image_availible_semaphore};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffers[image_index];

	VkSemaphore signal_semaphores[] = {render_finished_semaphore};
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	if (vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS){
		printf("Error: failed to submit draw command buffer");
	}

	VkPresentInfoKHR present_info = {0};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;
	VkSwapchainKHR swap_chains[] = {swap_chain};
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;
	present_info.pImageIndices = &image_index;

	present_info.pResults = NULL;

	vkQueuePresentKHR(presentation_queue, &present_info);

	printf("frame completed");
}