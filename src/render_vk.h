#ifndef RENDER_VK_H
#define RENDER_VK_H

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>

#if PLATFORM_LINUX
#include <vulkan/vulkan_xcb.h>
#endif

#define vk_instance_func(f) PFN_##f f

vk_instance_func(vkGetInstanceProcAddr);
vk_instance_func(vkCreateInstance);
vk_instance_func(vkDestroyInstance);
vk_instance_func(vkEnumerateInstanceExtensionProperties);
vk_instance_func(vkGetDeviceProcAddr);
vk_instance_func(vkEnumeratePhysicalDevices);
vk_instance_func(vkDestroySurfaceKHR);
vk_instance_func(vkGetPhysicalDeviceQueueFamilyProperties);
vk_instance_func(vkGetPhysicalDeviceSurfaceSupportKHR);

#if PLATFORM_LINUX
vk_instance_func(vkCreateXcbSurfaceKHR);
#endif

#undef vk_instance_func

struct render {
  void *vklib;
  VkInstance instance;
  uint32_t n_devices;
  VkPhysicalDevice *phys_devices;
  VkSurfaceKHR surface;
};

#define vkfunc(F) PFN_vk##F F
struct vk_device_functions {
  int dummy;
};
#undef vkfunc

struct render_pipeline {
  struct render *ctx;
  struct vk_device_functions vk;
  size_t device;
  uint32_t n_queue_props;
  VkQueueFamilyProperties *queue_props;
  uint32_t queue_index_graphics;
  uint32_t queue_index_present;
};

#endif

