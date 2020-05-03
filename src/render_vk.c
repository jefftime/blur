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

  r->vk.GetInstanceProcAddr = dlsym_result.func;
  return RENDER_ERROR_NONE;
}

static int load_preinstance_functions(struct render *r) {
#define load(F) \
  if (!(r->vk.F = (PFN_vk##F) r->vk.GetInstanceProcAddr(NULL, "vk" #F))) \
    return RENDER_ERROR_VULKAN_PREINST_LOAD

  load(CreateInstance);
  load(EnumerateInstanceExtensionProperties);
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

  result = r->vk.EnumerateInstanceExtensionProperties(
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
  r->vk.EnumerateInstanceExtensionProperties(
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
  result = r->vk.CreateInstance(&create_info, NULL, &r->instance);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_INSTANCE;
  return RENDER_ERROR_NONE;
}

static int load_instance_functions(struct render *r) {
#define load(F) \
  if (!(r->vk.F = (PFN_vk##F) r->vk.GetInstanceProcAddr(r->instance, "vk" #F))) \
    return RENDER_ERROR_VULKAN_INSTANCE_FUNC_LOAD

  load(GetDeviceProcAddr);
  load(DestroyInstance);
  load(EnumeratePhysicalDevices);
  return RENDER_ERROR_NONE;

#undef load
}

static int get_devices(struct render *r) {
  uint32_t n_devices;
  VkResult result;

  result = r->vk.EnumeratePhysicalDevices(r->instance, &n_devices, NULL);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_PHYSICAL_DEVICE;
  if (n_devices == 0) return RENDER_ERROR_VULKAN_NO_DEVICES;
  r->n_devices = n_devices;
  r->phys_devices = malloc(sizeof(VkPhysicalDevice) * n_devices);
  if (!r->phys_devices) return RENDER_ERROR_MEMORY;
  result = r->vk.EnumeratePhysicalDevices(
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
  chkerr(get_devices(r));
  return RENDER_ERROR_NONE;
}

void render_deinit(struct render *r) {
  if (!r) return;
  r->vk.DestroyInstance(r->instance, NULL);
  dlclose(r->vklib);
}

struct render_pipeline *render_create_pipeline(struct render *r) {
  return NULL;
}

int render_configure(struct render *r, uint16_t width, uint16_t height) {
  return 0;
}

void render_update(struct render *r) {
  
}
