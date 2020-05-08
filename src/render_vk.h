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

struct device_functions {
  vkfunc(vkCreateSwapchainKHR);
};

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
  struct device_functions *func;

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
  struct render *ctx;
  struct device_functions func;
  size_t phys_device;
  uint32_t n_queue_props;
  VkQueueFamilyProperties *queue_props;
  uint32_t queue_index_graphics;
  uint32_t queue_index_present;
  VkDevice device;
  VkQueue graphics_queue;
  VkQueue present_queue;
};

#undef vkfunc

#endif

