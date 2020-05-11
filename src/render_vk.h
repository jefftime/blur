#ifndef RENDER_VK_H
#define RENDER_VK_H

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_platform.h>

#if PLATFORM_LINUX
#include <vulkan/vulkan_xcb.h>
#endif

#define vkfunc(F) PFN_##F F

vkfunc(vkGetInstanceProcAddr);
vkfunc(vkCreateInstance);
vkfunc(vkDestroyInstance);
vkfunc(vkEnumerateInstanceExtensionProperties);
/* Physical device */
vkfunc(vkGetPhysicalDeviceProperties);
vkfunc(vkGetDeviceProcAddr);
vkfunc(vkEnumeratePhysicalDevices);
vkfunc(vkDestroySurfaceKHR);
vkfunc(vkGetPhysicalDeviceQueueFamilyProperties);
vkfunc(vkGetPhysicalDeviceSurfaceSupportKHR);
/* Logical device */
vkfunc(vkCreateDevice);
vkfunc(vkGetDeviceQueue);
vkfunc(vkDestroyDevice);
/* Swapchain */
vkfunc(vkGetPhysicalDeviceSurfaceFormatsKHR);
vkfunc(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
vkfunc(vkCreateSwapchainKHR);
vkfunc(vkDestroySwapchainKHR);
vkfunc(vkGetSwapchainImagesKHR);

#if PLATFORM_LINUX
vkfunc(vkCreateXcbSurfaceKHR);
#endif

/* Device functions */
vkfunc(vkCreateShaderModule);
vkfunc(vkDestroyShaderModule);
vkfunc(vkCreatePipelineLayout);
vkfunc(vkDestroyPipelineLayout);
vkfunc(vkCreateRenderPass);
vkfunc(vkDestroyRenderPass);
vkfunc(vkCreateGraphicsPipelines);
vkfunc(vkDestroyPipeline);
vkfunc(vkCreateImageView);
vkfunc(vkDestroyImageView);
vkfunc(vkCreateFramebuffer);
vkfunc(vkDestroyFramebuffer);
vkfunc(vkCreateCommandPool);
vkfunc(vkDestroyCommandPool);
vkfunc(vkAllocateCommandBuffers);
vkfunc(vkFreeCommandBuffers);
vkfunc(vkCreateBuffer);
vkfunc(vkDestroyBuffer);

struct render {
  void *vklib;
  VkInstance instance;
  VkSurfaceKHR surface;

  /* Device information */
  size_t n_devices;
  uint32_t *graphics_indices;
  uint32_t *present_indices;
  VkPhysicalDevice *pdevices;
  VkDevice *devices;
  VkSurfaceFormatKHR *formats;

  /* Active device */
  size_t active_device_index;
  VkPhysicalDevice *active_pdevice;
  VkDevice *active_device;
  VkSwapchainKHR swapchain;
  size_t n_swapchain_images;
  VkImage *swapchain_images;
  VkExtent2D swap_extent;
};

struct render_pipeline {
  VkDevice device;
  VkSurfaceFormatKHR format;
  VkExtent2D swap_extent;
  VkRenderPass render_pass;
  uint32_t queue_index_graphics;
  VkPipeline pipeline;
  size_t n_swapchain_images;
  VkImage *swapchain_images;
  VkImageView *image_views;
  VkFramebuffer *framebuffers;
  VkCommandPool command_pool;
  VkCommandBuffer *command_buffers;
  VkBuffer vertex_buffer;
  VkBuffer index_buffer;
  VkDeviceMemory vertex_memory;
  VkDeviceMemory index_memory;
};

#undef vkfunc

#endif

