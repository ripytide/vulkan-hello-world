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

//function declarations
GLFWwindow* InitialiseGLFW();
void mainLoop(GLFWwindow* window);
void CleanUp(GLFWwindow* window, VkInstance instance, VkDevice device, VkDebugUtilsMessengerEXT debug_mesenger, VkSurfaceKHR surface);

//enables validation layers depending of whether it was compiled in debug mode of not
#ifdef NDEBUG
	static const bool enableValidationLayers = false;
#else
	static const bool enableValidationLayers = true;
#endif

//height and width parameters for the application window
const uint32_t width = 10;
const uint32_t height = 10;
	
int main() {
	//important variables used frequently
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkPhysicalDevice physical_device;
	VkDevice device;
	VkSurfaceKHR surface;
	struct queue_family_indices indices;
	VkQueue graphics_queue;
	VkQueue presentation_queue;
	//define them
	window = InitialiseGLFW();
	instance = create_vk_instance();
	setup_debug_messenger(instance, &debug_messenger);
	surface = create_surface(instance, window);
	physical_device = pick_physical_device(instance, surface);
	device = create_logical_device(physical_device, surface);
	indices = find_queue_families(physical_device, surface);
	vkGetDeviceQueue(device, indices.graphics_family, 0, &graphics_queue);
	vkGetDeviceQueue(device, indices.presentation_family, 0, &presentation_queue);

	//printf("Do you have the required layers installed: %s", CheckValidationLayerSupport() ? "YES\n" : "NO\n");

	mainLoop(window);
	
	CleanUp(window, instance, device, debug_messenger, surface);

	return 0;
}

GLFWwindow* InitialiseGLFW() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	return(glfwCreateWindow(width, height, "Vulkan", NULL, NULL));
}

void mainLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
}

void CleanUp(GLFWwindow* window, VkInstance instance, VkDevice device, VkDebugUtilsMessengerEXT debug_messenger, VkSurfaceKHR surface) {
	vkDestroyDevice(device, NULL);

	vkDestroySurfaceKHR(instance, surface, NULL);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debug_messenger, NULL);
	}

	vkDestroyInstance(instance, NULL);

	glfwDestroyWindow(window);

	glfwTerminate();
}