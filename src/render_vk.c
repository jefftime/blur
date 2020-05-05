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
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

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
    return RENDER_ERROR_VULKAN_UNSUPPORTED_EXTENSION;

  success:
    continue;
  }
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
  load(vkGetPhysicalDeviceQueueFamilyProperties);
  load(vkGetPhysicalDeviceSurfaceSupportKHR);
  load(vkDestroySurfaceKHR);
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

static int get_devices(struct render *r) {
  uint32_t n_devices;
  VkResult result;

  result = vkEnumeratePhysicalDevices(r->instance, &n_devices, NULL);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_PHYSICAL_DEVICE;
  if (n_devices == 0) return RENDER_ERROR_VULKAN_NO_DEVICES;
  r->n_devices = n_devices;
  r->phys_devices = malloc(sizeof(VkPhysicalDevice) * n_devices);
  if (!r->phys_devices) return RENDER_ERROR_MEMORY;
  result = vkEnumeratePhysicalDevices(
    r->instance,
    &n_devices,
    r->phys_devices
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_PHYSICAL_DEVICE;
  return RENDER_ERROR_NONE;
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
  chkerr(get_devices(r));
  return RENDER_ERROR_NONE;
}

void render_deinit(struct render *r) {
  if (!r) return;
  vkDestroySurfaceKHR(r->instance, r->surface, NULL);
  vkDestroyInstance(r->instance, NULL);
  dlclose(r->vklib);
}

struct render_pipeline *render_create_pipeline(struct render *r) {
  return NULL;
}

void render_update(struct render *r) {
  
}
