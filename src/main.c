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
void CleanUp(GLFWwindow* window, VkInstance instance, VkDebugUtilsMessengerEXT debug_mesenger);

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

	//define them
	window = InitialiseGLFW();
	instance = create_vk_instance();
	setup_debug_messenger(instance, &debug_messenger);

	//printf("Do you have the required layers installed: %s", CheckValidationLayerSupport() ? "YES\n" : "NO\n");

	mainLoop(window);
	
	CleanUp(window, instance, debug_messenger);

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

void CleanUp(GLFWwindow* window, VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger) {
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debug_messenger, NULL);
	}

	vkDestroyInstance(instance, NULL);

	glfwDestroyWindow(window);

	glfwTerminate();
}