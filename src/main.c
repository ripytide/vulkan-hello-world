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
void mainLoop(GLFWwindow* window, VkDevice device, VkQueue graphics_queue, VkQueue presentation_queue, VkSwapchainKHR swap_chain, VkCommandBuffer *command_buffers, VkSemaphore image_availible_semaphore, VkSemaphore render_finished_semaphore);
void CleanUp(GLFWwindow *window, VkInstance instance, VkDevice device, VkDebugUtilsMessengerEXT debug_messenger, VkSurfaceKHR surface, VkSwapchainKHR swap_chain, VkImageView *image_views, int image_count, VkPipelineLayout pipeline_layout, VkRenderPass render_pass, VkPipeline graphics_pipeline, VkFramebuffer *framebuffers, VkCommandPool command_pool, VkSemaphore image_availible_semaphore, VkSemaphore render_finished_semaphore);

//enables validation layers depending of whether it was compiled in debug mode of not
#ifdef NDEBUG
	static const bool enableValidationLayers = false;
#else
	static const bool enableValidationLayers = true;
#endif

int main() {
	//declare important variables used frequently
	//for vulkan setup and config
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice physical_device;
	VkDevice device;
	VkSwapchainKHR swap_chain;
	VkImage *images;
	int image_count;
	VkImageView *image_views;
	VkFormat format;
	VkExtent2D extent;

	struct queue_family_indices queue_family_indicies;
	VkQueue graphics_queue;
	VkQueue presentation_queue;


	//define them
	window = InitialiseGLFW(400, 400);

	instance = create_vk_instance();

	setup_debug_messenger(instance, &debug_messenger);

	surface = create_surface(instance, window);

	physical_device = pick_physical_device(instance, surface);

	device = create_logical_device(physical_device, surface);

	queue_family_indicies = find_queue_families(physical_device, surface);
	vkGetDeviceQueue(device, queue_family_indicies.graphics_family, 0, &graphics_queue);
	vkGetDeviceQueue(device, queue_family_indicies.presentation_family, 0, &presentation_queue);

	struct swap_chain_info swap_chain_info = create_swap_chain(physical_device, surface, window, device);
	swap_chain = swap_chain_info.swap_chain;
	images = swap_chain_info.images;
	image_count = swap_chain_info.image_count;
	printf("Using %d images in the swapchain\n\n", image_count);
	format = swap_chain_info.format;
	extent = swap_chain_info.extent;

	image_views = create_image_views(images, image_count, format, device);

	//the graphics pipeline setup
	//declarations
	VkRenderPass render_pass;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
	VkFramebuffer* framebuffers;

	//definitions
	render_pass = create_render_pass(format, device);
	pipeline_layout = create_graphics_pipeline_layout(device);
	graphics_pipeline = create_graphics_pipeline(device, extent, render_pass, pipeline_layout);
	framebuffers = create_swap_chain_framebuffers(device, image_count, render_pass, image_views, extent);

	//control stuff
	//declarations
	VkCommandPool command_pool;
	VkCommandBuffer *command_buffers;
	VkSemaphore image_availible_semaphore;
	VkSemaphore render_finished_semaphore;

	//definitions
	command_pool = create_command_pool(device, queue_family_indicies.graphics_family);
	command_buffers = create_command_buffers(device, command_pool, render_pass, graphics_pipeline, framebuffers, extent, image_count);
	image_availible_semaphore = create_semaphore(device);
	render_finished_semaphore = create_semaphore(device);

	//printf("Do you have the required layers installed: %s", CheckValidationLayerSupport() ? "YES\n" : "NO\n");

	//the mainloop
	mainLoop(window, device, graphics_queue, presentation_queue, swap_chain, command_buffers, image_availible_semaphore, render_finished_semaphore);
	

	//the clean up after main loop ends
	CleanUp(window, instance, device, debug_messenger, surface, swap_chain, image_views, image_count, pipeline_layout, render_pass, graphics_pipeline, framebuffers, command_pool, image_availible_semaphore, render_finished_semaphore);

	return 0;
}


void mainLoop(GLFWwindow* window, VkDevice device, VkQueue graphics_queue, VkQueue presentation_queue, VkSwapchainKHR swap_chain, VkCommandBuffer *command_buffers, VkSemaphore image_availible_semaphore, VkSemaphore render_finished_semaphore) {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		draw_frame(device, graphics_queue, presentation_queue, swap_chain, command_buffers, image_availible_semaphore, render_finished_semaphore);
	}

	vkDeviceWaitIdle(device);
}

void CleanUp(GLFWwindow *window, VkInstance instance, VkDevice device, VkDebugUtilsMessengerEXT debug_messenger, VkSurfaceKHR surface, VkSwapchainKHR swap_chain, VkImageView *image_views, int image_count, VkPipelineLayout pipeline_layout, VkRenderPass render_pass, VkPipeline graphics_pipeline, VkFramebuffer *framebuffers, VkCommandPool command_pool, VkSemaphore image_availible_semaphore, VkSemaphore render_finished_semaphore) {

	vkDestroySemaphore(device, image_availible_semaphore, NULL);
	vkDestroySemaphore(device, render_finished_semaphore, NULL);

	vkDestroyCommandPool(device, command_pool, NULL);

	for (int i = 0; i < image_count; i++){
		vkDestroyFramebuffer(device, framebuffers[i], NULL);
	}

		vkDestroyPipeline(device, graphics_pipeline, NULL);
	vkDestroyPipelineLayout(device, pipeline_layout, NULL);
	vkDestroyRenderPass(device, render_pass, NULL);
	//order here is extremly important
	for (int j = 0; j < image_count; j++){
		vkDestroyImageView(device, image_views[j], NULL);
	}

	vkDestroySwapchainKHR(device, swap_chain, NULL);

	vkDestroyDevice(device, NULL);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debug_messenger, NULL);
	}

	vkDestroySurfaceKHR(instance, surface, NULL);

	vkDestroyInstance(instance, NULL);

	glfwDestroyWindow(window);

	glfwTerminate();
}