/* Copyright 2020, Jeffery Stager
 *
 * This file is part of Tortuga
 *
 * Tortuga is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Tortuga is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tortuga.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "render.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

#ifdef PLATFORM_LINUX
#include <vulkan/vulkan_xcb.h>
#include <dlfcn.h>
#define LIBNAME "libvulkan.so"
#endif

static int load_vulkan(struct render_instance *r) {
  /*
   * WARNING: This won't work on systems where object pointers and function
   * pointers have different size and/or alignments
   */
  union {
    PFN_vkGetInstanceProcAddr func;
    void *ptr;
  } dlsym_result;

#ifdef PLATFORM_LINUX
  r->vk_handle = dlopen(LIBNAME, RTLD_NOW);
  if (!r->vk_handle) return RENDER_ERROR_VULKAN_LOAD;

  /*
   * Object pointers and function pointers are treated differently in
   * ISO C, so we have to type pun the result of dlsym()
   */
  dlsym_result.ptr = dlsym(r->vk_handle, "vkGetInstanceProcAddr");
#endif

  vkGetInstanceProcAddr = dlsym_result.func;
  return RENDER_ERROR_NONE;
  return RENDER_ERROR_NONE;
}

static int load_preinstance_functions(struct render_instance *r) {
#define vkfunc(F) \
  if (!(F = (PFN_##F) vkGetInstanceProcAddr(NULL, #F))) \
    return RENDER_ERROR_VULKAN_LOAD_INSTANCE_FUNCTION

  vkfunc(vkCreateInstance);
  vkfunc(vkEnumerateInstanceExtensionProperties);
  return RENDER_ERROR_NONE;

#undef vkfunc
}

static int create_instance(struct render_instance *r, size_t n_exts, char **exts) {
  char *layers[] = { "VK_LAYER_LUNARG_standard_validation" };
  VkApplicationInfo app_info = { 0 };
  VkInstanceCreateInfo create_info = { 0 };
  VkResult result;

  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Tortuga Test";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "Tortuga";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = (uint32_t) n_exts;
  create_info.ppEnabledExtensionNames = (const char *const *) exts;
#if NDEBUG
  create_info.enabledLayerCount = 0;
  create_info.ppEnabledLayerNames = VK_NULL_HANDLE;
#else
  create_info.enabledLayerCount = 1;
  create_info.ppEnabledLayerNames = (const char *const *) layers;
#endif
  result = vkCreateInstance(&create_info, NULL, &r->instance);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_CREATE_INSTANCE;
  return RENDER_ERROR_NONE;
}

static int load_instance_functions(struct render_instance *r) {
#define vkfunc(F) \
  if (!(F = (PFN_##F) vkGetInstanceProcAddr(r->instance, #F))) \
    return RENDER_ERROR_VULKAN_INSTANCE_FUNCTIONS

  vkfunc(vkDestroyInstance);
  vkfunc(vkEnumerateInstanceExtensionProperties);
  vkfunc(vkCreateXcbSurfaceKHR);
  vkfunc(vkDestroySurfaceKHR);
  vkfunc(vkEnumeratePhysicalDevices);
  vkfunc(vkGetDeviceProcAddr);
  vkfunc(vkCreateDevice);
  vkfunc(vkDestroyDevice);
  vkfunc(vkGetPhysicalDeviceQueueFamilyProperties);
  vkfunc(vkGetPhysicalDeviceSurfaceSupportKHR);
  vkfunc(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
  vkfunc(vkGetPhysicalDeviceSurfaceFormatsKHR);
  vkfunc(vkGetPhysicalDeviceSurfacePresentModesKHR);
  vkfunc(vkCreateSwapchainKHR);
  vkfunc(vkDestroySwapchainKHR);
  vkfunc(vkGetSwapchainImagesKHR);
  vkfunc(vkGetPhysicalDeviceMemoryProperties);
  vkfunc(vkGetPhysicalDeviceProperties);
  vkfunc(vkGetPhysicalDeviceFeatures);
  return RENDER_ERROR_NONE;

#undef vkfunc
}

static int create_surface(struct render_instance *r) {
  VkXcbSurfaceCreateInfoKHR create_info = { 0 };
  VkResult result;

  create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  create_info.connection = r->window->os.cn;
  create_info.window = r->window->os.wn;
  result = vkCreateXcbSurfaceKHR(r->instance, &create_info, NULL, &r->surface);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_SURFACE;
  return RENDER_ERROR_NONE;
}

static int get_devices(struct render_instance *r) {
  uint32_t n_devices;
  VkResult result;

  result = vkEnumeratePhysicalDevices(r->instance, &n_devices, NULL);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_PHYSICAL_DEVICES;
  if (n_devices == 0) return RENDER_ERROR_VULKAN_PHYSICAL_DEVICES;
  r->pdevices = malloc(sizeof(VkPhysicalDevice) * n_devices);
  if (!r->pdevices) return RENDER_ERROR_MEMORY;
  result = vkEnumeratePhysicalDevices(r->instance, &n_devices, r->pdevices);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_PHYSICAL_DEVICES;
  r->n_pdevices = n_devices;
  return RENDER_ERROR_NONE;
}

/* **************************************** */
/* Public */
/* **************************************** */

int render_instance_init(struct render_instance *r, struct window *w) {
  int err;
  size_t n_exts;
  char *exts[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_XCB_SURFACE_EXTENSION_NAME
  };

  if (!r) return RENDER_ERROR_NULL;
  memset(r, 0, sizeof(struct render_instance));
  n_exts = sizeof(exts) / sizeof(exts[0]);
  r->window = w;
  chkerrg(err = load_vulkan(r), err_load_vulkan);
  chkerrg(err = load_preinstance_functions(r), err_preinstance_functions);
  chkerrg(err = create_instance(r, n_exts, exts), err_instance);
  chkerrg(err = load_instance_functions(r), err_instance_functions);
  chkerrg(err = create_surface(r), err_surface);
  chkerrg(err = get_devices(r), err_devices);
  return RENDER_ERROR_NONE;

 err_devices:
  vkDestroySurfaceKHR(r->instance, r->surface, NULL);
 err_surface:
 err_instance_functions:
  vkDestroyInstance(r->instance, NULL);
 err_instance:
 err_preinstance_functions:
#ifdef PLATFORM_LINUX
  dlclose(r->vk_handle);
#endif
 err_load_vulkan:
  memset(r, 0, sizeof(struct render_instance));
  return err;
}

void render_instance_deinit(struct render_instance *r) {
  free(r->pdevices);
  vkDestroySurfaceKHR(r->instance, r->surface, NULL);
  vkDestroyInstance(r->instance, NULL);
#ifdef PLATFORM_LINUX
  dlclose(r->vk_handle);
#endif
  memset(r, 0, sizeof(struct render_instance));
}
