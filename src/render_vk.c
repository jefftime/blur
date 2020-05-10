/* Copyright 2020, Jeffery Stager
 *
 * This file is part of Blur
 *
 * Blur is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Blur is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Blur.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "render.h"
#include "error.h"
#include "window.h"
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_platform.h>

#if PLATFORM_LINUX
#include <vulkan/vulkan_xcb.h>
#include <dlfcn.h>
#define LIBNAME "libvulkan.so"
#endif  /* PLATFORM_LINUX */

static int load_vulkan(struct render *r) {
  /*
   * WARNING: This won't work on systems where object pointers and function
   * pointers have different size and/or alignments
   */
  union {
    PFN_vkGetInstanceProcAddr func;
    void *ptr;
  } dlsym_result;

#if PLATFORM_LINUX
  r->vklib = dlopen(LIBNAME, RTLD_NOW);
  if (!r->vklib) return RENDER_ERROR_VULKAN_LOAD;

  /*
   * Object pointers and function pointers are treated differently in
   * ISO C, so we have to type pun the result of dlsym()
   */
  dlsym_result.ptr = dlsym(r->vklib, "vkGetInstanceProcAddr");
#endif

  vkGetInstanceProcAddr = dlsym_result.func;
  return RENDER_ERROR_NONE;
}

static int load_preinstance_functions(struct render *r) {
#define load(F) \
  if (!(F = (PFN_##F) vkGetInstanceProcAddr(NULL, #F))) \
    return RENDER_ERROR_VULKAN_PREINST_LOAD

  load(vkCreateInstance);
  load(vkEnumerateInstanceExtensionProperties);
  return RENDER_ERROR_NONE;

#undef load
}

static int check_instance_extensions(
  struct render *r,
  size_t n_exts,
  char **exts
) {
  size_t i, j;
  uint32_t n_supported_exts;
  VkExtensionProperties *supported_exts;
  VkResult result;

  result = vkEnumerateInstanceExtensionProperties(
    NULL,
    &n_supported_exts,
    NULL
  );
  if (result != VK_SUCCESS) {
    return RENDER_ERROR_VULKAN_ENUMERATE_SUPPORTED_EXTENSIONS;
  }
  if (!n_supported_exts) return 0;
  supported_exts = malloc(n_supported_exts * sizeof(VkExtensionProperties));
  if (!supported_exts) return RENDER_ERROR_MEMORY;
  vkEnumerateInstanceExtensionProperties(
    NULL,
    &n_supported_exts,
    supported_exts
  );
  /* Check supported_exts */
  for (i = 0; i < n_exts; ++i) {
    for (j = 0; j < n_supported_exts; ++j) {
      if (!strcmp(supported_exts[j].extensionName, exts[i])) goto success;
    }
    free(supported_exts);
    return RENDER_ERROR_VULKAN_UNSUPPORTED_EXTENSION;

  success:
    continue;
  }
  free(supported_exts);
  return RENDER_ERROR_NONE;
}

static int create_instance(struct render *r, size_t n_exts, char **exts) {
  VkInstanceCreateInfo create_info = { 0 };
  VkResult result;

  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.enabledExtensionCount = (uint32_t) n_exts;
  create_info.ppEnabledExtensionNames = (const char *const *) exts;
  result = vkCreateInstance(&create_info, NULL, &r->instance);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_INSTANCE;
  return RENDER_ERROR_NONE;
}

static int load_instance_functions(struct render *r) {
#define load(F) \
  if (!(F = (PFN_##F) vkGetInstanceProcAddr(r->instance, #F))) \
    return RENDER_ERROR_VULKAN_INSTANCE_FUNC_LOAD

  load(vkGetDeviceProcAddr);
  load(vkDestroyInstance);
  load(vkEnumeratePhysicalDevices);
  load(vkGetPhysicalDeviceProperties);
  load(vkGetPhysicalDeviceQueueFamilyProperties);
  load(vkGetPhysicalDeviceSurfaceSupportKHR);
  load(vkDestroySurfaceKHR);
  load(vkCreateDevice);
  load(vkGetDeviceQueue);
  load(vkDestroyDevice);
  load(vkGetPhysicalDeviceSurfaceFormatsKHR);
  load(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
  load(vkCreateSwapchainKHR);
  load(vkDestroySwapchainKHR);
  load(vkGetSwapchainImagesKHR);
#if PLATFORM_LINUX
  load(vkCreateXcbSurfaceKHR);
#endif  /* PLATFORM_LINUX */
  return RENDER_ERROR_NONE;

#undef load
}

static int create_surface(struct render *r, struct window *w) {
  VkXcbSurfaceCreateInfoKHR create_info = { 0 };
  VkResult result;

  create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  create_info.connection = w->os.cn;
  create_info.window = w->os.wn;
  result = vkCreateXcbSurfaceKHR(
    r->instance,
    &create_info,
    NULL,
    &r->surface
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_SURFACE;
  return RENDER_ERROR_NONE;
}

static int get_devices(
  struct render *r,
  VkPhysicalDevice **pdevices,
  VkPhysicalDeviceProperties **phys_dev_props
) {
  uint32_t n_devices;
  size_t i;
  VkResult result;

  result = vkEnumeratePhysicalDevices(r->instance, &n_devices, NULL);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_PHYSICAL_DEVICE;
  if (n_devices == 0) return RENDER_ERROR_VULKAN_NO_DEVICES;
  r->n_devices = n_devices;
  *pdevices = malloc(sizeof(VkPhysicalDevice) * n_devices);
  if (!*pdevices) return RENDER_ERROR_MEMORY;
  result = vkEnumeratePhysicalDevices(
    r->instance,
    &n_devices,
    *pdevices
  );
  if (result != VK_SUCCESS) {
    free(*pdevices);
    return RENDER_ERROR_VULKAN_PHYSICAL_DEVICE;
  }
  *phys_dev_props = malloc(
    sizeof(VkPhysicalDeviceProperties) * r->n_devices
  );
  if (!*phys_dev_props) {
    free(*pdevices);
    return RENDER_ERROR_MEMORY;
  }
  for (i = 0; i < r->n_devices; ++i) {
    VkPhysicalDeviceProperties *props;

    props = *phys_dev_props;
    vkGetPhysicalDeviceProperties(
      (*pdevices)[i],
      props + i
    );
  }
  return RENDER_ERROR_NONE;
}

static int get_queue_indices(
  struct render *r,
  VkPhysicalDevice device,
  uint32_t *out_graphics,
  uint32_t *out_present
) {
  int graphics_isset = 0, present_isset = 0;
  uint32_t i, n_props;
  VkQueueFamilyProperties *props;
  VkResult result;

  vkGetPhysicalDeviceQueueFamilyProperties(device, &n_props, NULL);
  if (n_props == 0) return RENDER_ERROR_VULKAN_QUEUE_INDICES;
  props = malloc(sizeof(VkPhysicalDeviceProperties) * n_props);
  if (!props) return RENDER_ERROR_MEMORY;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &n_props, props);
  for (i = 0; i < n_props; ++i) {
    uint32_t present_support = 0;

    if (
      props[i].queueCount > 0
      && props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT
    ) {
      graphics_isset = 1;
      *out_graphics = i;
    }
    result = vkGetPhysicalDeviceSurfaceSupportKHR(
      device,
      i,
      r->surface,
      &present_support
    );
    if (result != VK_SUCCESS) {
      free(props);
      return RENDER_ERROR_VULKAN_QUEUE_INDICES;
    }
    if (props[i].queueCount > 0 && present_support) {
      present_isset = 1;
      *out_present = i;
    }
  }
  free(props);
  if (graphics_isset && present_isset) return RENDER_ERROR_NONE;
  return RENDER_ERROR_VULKAN_QUEUE_INDICES;
}

static int get_queue_information(
  struct render *r,
  VkPhysicalDevice *devices
) {
  size_t i;

  r->graphics_indices = malloc(sizeof(uint32_t) * r->n_devices);
  if (!r->graphics_indices) return RENDER_ERROR_MEMORY;
  r->present_indices = malloc(sizeof(uint32_t) * r->n_devices);
  if (!r->present_indices) {
    free(r->graphics_indices);
    return RENDER_ERROR_MEMORY;
  }

  for (i = 0; i < r->n_devices; ++i) {
    chkerr(get_queue_indices(
             r,
             devices[i],
             r->graphics_indices + i,
             r->present_indices + i
           ));
  }
  return RENDER_ERROR_NONE;
}

static int create_logical_devices(
  struct render *r,
  VkPhysicalDevice *phys_devices
) {
  char *extensions[] = { "VK_KHR_swapchain" };
  size_t i;

  r->devices = malloc(sizeof(VkDevice) * r->n_devices);
  for (i = 0; i < r->n_devices; ++i) {
    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_infos[] = { { 0 }, { 0 } };
    VkDeviceCreateInfo create_info = { 0 };
    VkResult result;

    /* TODO: support separate graphics and present queues since
     * currently this is out of spec */
    queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[0].queueFamilyIndex = r->graphics_indices[i];
    queue_create_infos[0].queueCount = 1;
    queue_create_infos[0].pQueuePriorities = &queue_priority;
    queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[1].queueFamilyIndex = r->present_indices[i];
    queue_create_infos[1].queueCount = 1;
    queue_create_infos[1].pQueuePriorities = &queue_priority;
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = queue_create_infos;
    create_info.enabledExtensionCount = 1;
    create_info.ppEnabledExtensionNames = (const char *const *) extensions;
    result = vkCreateDevice(
      phys_devices[i],
      &create_info,
      NULL,
      r->devices + i
    );
    if (result != VK_SUCCESS) {
      while (i--) vkDestroyDevice(r->devices[i], NULL);
      free(r->devices);
      return RENDER_ERROR_VULKAN_CREATE_DEVICE;
    }
  }
  return RENDER_ERROR_NONE;
}

static int load_device_functions(struct render *r, size_t device_id) {
#define load(F) \
  if (!(F = (PFN_##F) vkGetDeviceProcAddr(r->devices[device_id], #F))) \
    return RENDER_ERROR_VULKAN_DEVICE_FUNC_LOAD

  load(vkCreateShaderModule);
  load(vkDestroyShaderModule);
  load(vkCreatePipelineLayout);
  load(vkDestroyPipelineLayout);
  load(vkCreateRenderPass);
  load(vkDestroyRenderPass);
  load(vkCreateGraphicsPipelines);
  load(vkDestroyPipeline);
  load(vkCreateImageView);
  load(vkDestroyImageView);
  load(vkCreateFramebuffer);
  load(vkDestroyFramebuffer);
  return RENDER_ERROR_NONE;

#undef load
}

static int get_surface_format(
  struct render *r,
  VkPhysicalDevice *phys_devices
) {
  uint32_t n_formats;
  size_t i;

  r->formats = malloc(sizeof(VkSurfaceFormatKHR) * r->n_devices);
  if (!r->formats) return RENDER_ERROR_MEMORY;
  for (i = 0; i < r->n_devices; ++i) {
    VkSurfaceFormatKHR *formats;
    VkResult result;

    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
      phys_devices[i],
      r->surface,
      &n_formats,
      NULL
    );
    if (result != VK_SUCCESS) {
      free(r->formats);
      return RENDER_ERROR_VULKAN_SURFACE_FORMAT;
    }
    formats = malloc(sizeof(VkSurfaceFormatKHR) * n_formats);
    if (!formats) {
      free(r->formats);
      return RENDER_ERROR_MEMORY;
    }
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
      phys_devices[i],
      r->surface,
      &n_formats,
      formats
    );
    if (result != VK_SUCCESS) {
      free(r->formats);
      free(formats);
      return RENDER_ERROR_VULKAN_SURFACE_FORMAT;
    }
    r->formats[i] = formats[0];
    free(formats);
  }
  return RENDER_ERROR_NONE;
}

static int get_swapchain_images(struct render *r) {
  uint32_t n_images;
  VkResult result;

  result = vkGetSwapchainImagesKHR(
    *r->active_device,
    r->swapchain,
    &n_images,
    NULL
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_SWAPCHAIN_IMAGES;
  if (n_images == 0) return RENDER_ERROR_VULKAN_SWAPCHAIN_IMAGES;
  r->swapchain_images = malloc(sizeof(VkImage) * n_images);
  if (!r->swapchain_images) return RENDER_ERROR_MEMORY;
  result = vkGetSwapchainImagesKHR(
    *r->active_device,
    r->swapchain,
    &n_images,
    r->swapchain_images
  );
  if (result != VK_SUCCESS) {
    free(r->swapchain_images);
    return RENDER_ERROR_VULKAN_SWAPCHAIN_IMAGES;
  }
  r->n_swapchain_images = n_images;
  return RENDER_ERROR_NONE;
}

static int setup_swapchain(struct render *r) {
  int rc;
  VkSurfaceCapabilitiesKHR capabilities;
  VkSwapchainCreateInfoKHR create_info = { 0 };
  VkResult result;

  result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    *r->active_pdevice,
    r->surface,
    &capabilities
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_SURFACE_CAPABILITIES;
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = r->surface;
  create_info.minImageCount = 2;
  create_info.imageFormat = r->formats[r->active_device_index].format;
  create_info.imageColorSpace = r->formats[r->active_device_index].colorSpace;
  create_info.imageExtent = capabilities.currentExtent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  create_info.preTransform = capabilities.currentTransform;
  create_info.compositeAlpha = capabilities.supportedCompositeAlpha;
  create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  create_info.clipped = VK_TRUE;
  result = vkCreateSwapchainKHR(
    *r->active_device,
    &create_info,
    NULL,
    &r->swapchain
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_SWAPCHAIN;
  r->swap_extent = capabilities.currentExtent;
  if ((rc = get_swapchain_images(r))) {
    vkDestroySwapchainKHR(*r->active_device, r->swapchain, NULL);
    return rc;
  }
  return RENDER_ERROR_NONE;
}

static void teardown_active_device(struct render *r) {
  vkDestroySwapchainKHR(*r->active_device, r->swapchain, NULL);
  free(r->swapchain_images);
}

/* **************************************** */
/* Public */
/* **************************************** */

int render_init(struct render *r, struct window *w) {
  char *inst_exts[] = {
    "VK_KHR_surface",
#if PLATFORM_LINUX
    "VK_KHR_xcb_surface"
#endif  /* PLATFORM_LINUX */
  };

  size_t n_inst_exts;
  VkPhysicalDevice *pdevices = NULL;
  VkPhysicalDeviceProperties *pdevice_props = NULL;

  if (!r) return RENDER_ERROR_NULL;
  if (!w) return RENDER_ERROR_NULL;
  memset(r, 0, sizeof(struct render));
  n_inst_exts = sizeof(inst_exts) / sizeof(inst_exts[0]);
  chkerr(load_vulkan(r));
  chkerr(load_preinstance_functions(r));
  chkerr(check_instance_extensions(r, n_inst_exts, inst_exts));
  chkerr(create_instance(r, n_inst_exts, inst_exts));
  chkerr(load_instance_functions(r));
  chkerr(create_surface(r, w));
  chkerr(get_devices(r, &pdevices, &pdevice_props));
  chkerr(get_queue_information(r, pdevices));
  chkerr(create_logical_devices(r, pdevices));
  chkerr(get_surface_format(r, pdevices));
  r->pdevices = pdevices;
  free(pdevice_props);
  return RENDER_ERROR_NONE;
}

void render_deinit(struct render *r) {
  size_t i;

  if (!r) return;
  if (r->active_device) teardown_active_device(r);
  free(r->formats);
  for (i = 0; i < r->n_devices; ++i) vkDestroyDevice(r->devices[i], NULL);
  free(r->devices);
  free(r->pdevices);
  free(r->graphics_indices);
  free(r->present_indices);
  vkDestroySurfaceKHR(r->instance, r->surface, NULL);
  vkDestroyInstance(r->instance, NULL);
  dlclose(r->vklib);
}

int render_set_active_device(struct render *r, size_t device_id) {
  if (!r) return RENDER_ERROR_NULL;
  if (r->active_device) teardown_active_device(r);
  r->active_device_index = device_id;
  r->active_pdevice = &r->pdevices[device_id];
  r->active_device = &r->devices[device_id];
  chkerr(load_device_functions(r, device_id));
  chkerr(setup_swapchain(r));
  return RENDER_ERROR_NONE;
}

void render_update(struct render *r) {
  
}
