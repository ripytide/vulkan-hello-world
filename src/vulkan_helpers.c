//plain old C headers
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//GLFW header
#define GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "vulkan_helpers.h"

//the layers/extensions wanted on top of the GLFW required extensions
const char* validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
const char* other_extensions[] = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };

//handy macro for gettin the size of an array in bytes at runtime
#define ARR_SIZE(x) sizeof(x) / sizeof(x[0])

//a struct made to allow the get_required_extensions function to give both a list of extensions and
//the number of extensions seeing as it may change at runtime and sizeof is done at compile time i think
struct extension_info {
	const char** extensions;
	uint32_t extension_count;
};

//enables validation layers depending of whether it was compiled in debug mode of not
#ifdef NDEBUG
	static const bool enableValidationLayers = false;
#else
	static const bool enableValidationLayers = true;
#endif

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

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = ARR_SIZE(validation_layers);
		createInfo.ppEnabledLayerNames = validation_layers;

		VkDebugUtilsMessengerCreateInfoEXT messenger_info = { 0 };
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
	create_info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info->pfnUserCallback = debug_callback;
	create_info->pUserData = NULL; // Optional
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
			return false;
		}
	}
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

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {

	static int count = 0;

	printf("Current Count: %d\tSeverity: %d\tType: %d\tMessage: %s\n", ++count, messageSeverity, messageType, pCallbackData->pMessage);

	return VK_FALSE;
}


VkPhysicalDevice pick_physical_device(VkInstance instance){
	VkPhysicalDevice device = VK_NULL_HANDLE;
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(instance, &device_count, NULL);
	if (device_count == 0)
		printf("Error: No physical devices found");
	VkPhysicalDevice *devices = malloc(sizeof *devices * device_count);

	vkEnumeratePhysicalDevices(instance, &device_count, devices);

	for (unsigned int i = 0; i < device_count; i++){
		if (is_device_suitable(devices[i]))
			device = devices[i];
	}

	if (device == VK_NULL_HANDLE)
		printf("Error: No suitible device found");

	return device;
}

bool is_device_suitable(VkPhysicalDevice device){
	VkPhysicalDeviceProperties device_properties;
   VkPhysicalDeviceFeatures device_features;
   vkGetPhysicalDeviceProperties(device, &device_properties);
   vkGetPhysicalDeviceFeatures(device, &device_features);

	struct queue_family_indices indices = find_queue_families(device);

	return device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && device_features.geometryShader && indices.graphics_family_set;
}

struct queue_family_indices find_queue_families(VkPhysicalDevice device){
	struct queue_family_indices indices =
	{
		.graphics_family_set = 0,
		.graphics_family = false
	};

	uint32_t family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, NULL);

	VkQueueFamilyProperties *families = malloc(sizeof *families * family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, families);

	for (unsigned int i = 0; i < family_count; i++){
		if(families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
			indices.graphics_family = i;
			indices.graphics_family_set = true;
			return indices;
		}
	}

	return indices;
}