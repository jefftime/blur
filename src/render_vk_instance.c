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

static int load_vulkan(void **vk_handle) {
  /* WARNING: This won't work on systems where object pointers and function
   * pointers have different size and/or alignments */
  union {
    void *ptr;
    PFN_vkGetInstanceProcAddr func;
  } dlsym_result;

#ifdef PLATFORM_LINUX
  *vk_handle = dlopen(LIBNAME, RTLD_NOW);
  if (!*vk_handle) return RENDER_ERROR_VULKAN_LOAD;

  /* Object pointers and function pointers are treated differently in
   * ISO C, so we have to type pun the result of dlsym() */
  dlsym_result.ptr = dlsym(*vk_handle, "vkGetInstanceProcAddr");
#endif

  vkGetInstanceProcAddr = dlsym_result.func;
  return RENDER_ERROR_NONE;
}

static int load_preinstance_functions() {
#define vkfunc(F) \
  if (!(F = (PFN_##F) vkGetInstanceProcAddr(NULL, #F))) \
    return RENDER_ERROR_VULKAN_LOAD_INSTANCE_FUNCTION

  vkfunc(vkCreateInstance);
  vkfunc(vkEnumerateInstanceExtensionProperties);
  return RENDER_ERROR_NONE;

#undef vkfunc
}

static int create_instance(
  char *app_name,
  char *engine_name,
  size_t n_exts,
  char **exts,
  VkInstance *out_instance
) {
  char *layers[] = { "VK_LAYER_LUNARG_standard_validation" };
  VkApplicationInfo app_info = { 0 };
  VkInstanceCreateInfo create_info = { 0 };
  VkResult result;

  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = app_name;
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = engine_name;
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
  result = vkCreateInstance(&create_info, NULL, out_instance);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_CREATE_INSTANCE;
  return RENDER_ERROR_NONE;
}

static int load_instance_functions(VkInstance instance) {
#define vkfunc(F) \
  if (!(F = (PFN_##F) vkGetInstanceProcAddr(instance, #F))) \
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

static int create_surface(
  VkInstance instance,
  struct window *window,
  VkSurfaceKHR *out_surface
) {
  VkXcbSurfaceCreateInfoKHR create_info = { 0 };
  VkResult result;

  create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  create_info.connection = window->os.cn;
  create_info.window = window->os.wn;
  result = vkCreateXcbSurfaceKHR(instance, &create_info, NULL, out_surface);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_SURFACE;
  return RENDER_ERROR_NONE;
}

static int get_devices(
  VkInstance instance,
  uint32_t *out_n_pdevices,
  VkPhysicalDevice **out_pdevices
) {
  uint32_t n_pdevices;
  VkResult result;
  VkPhysicalDevice *pdevices;

  result = vkEnumeratePhysicalDevices(instance, &n_pdevices, NULL);
  if (result != VK_SUCCESS || n_pdevices == 0)
    return RENDER_ERROR_VULKAN_PHYSICAL_DEVICES;
  pdevices = malloc(sizeof(VkPhysicalDevice) * n_pdevices);
  if (!pdevices) return RENDER_ERROR_MEMORY;
  result = vkEnumeratePhysicalDevices(instance, &n_pdevices, pdevices);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_PHYSICAL_DEVICES;
  *out_n_pdevices = n_pdevices;
  *out_pdevices = pdevices;
  return RENDER_ERROR_NONE;
}

/* **************************************** */
/* Public */
/* **************************************** */

int render_instance_init(struct render_instance *r, struct window *window) {
  char *exts[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_XCB_SURFACE_EXTENSION_NAME
  };
  int err;
  uint32_t n_pdevices;
  size_t n_exts;
  void *vk_handle;
  VkInstance instance;
  VkSurfaceKHR surface;
  VkPhysicalDevice *pdevices;

  if (!r) return RENDER_ERROR_NULL;
  memset(r, 0, sizeof(struct render_instance));
  n_exts = sizeof(exts) / sizeof(exts[0]);
  chkerrg(err = load_vulkan(&vk_handle), err_load_vulkan);
  chkerrg(err = load_preinstance_functions(), err_preinstance_functions);
  chkerrg(
    err = create_instance("Tortuga", "Tortuga", n_exts, exts, &instance),
    err_instance
  );
  chkerrg(err = load_instance_functions(instance), err_instance_functions);
  chkerrg(err = create_surface(instance, window, &surface), err_surface);
  chkerrg(err = get_devices(instance, &n_pdevices, &pdevices), err_devices);
  r->vk_handle = vk_handle;
  r->instance = instance;
  r->surface = surface;
  r->n_pdevices = n_pdevices;
  r->pdevices = pdevices;
  r->window = window;
  return RENDER_ERROR_NONE;

 err_devices:
  vkDestroySurfaceKHR(instance, surface, NULL);
 err_surface:
 err_instance_functions:
  vkDestroyInstance(instance, NULL);
 err_instance:
 err_preinstance_functions:
#ifdef PLATFORM_LINUX
  dlclose(vk_handle);
#endif
 err_load_vulkan:
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
