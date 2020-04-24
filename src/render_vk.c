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

#define vkfunc(F) PFN_vk##F F

struct vk_functions {
  vkfunc(GetInstanceProcAddr);
  vkfunc(CreateInstance);
  vkfunc(EnumerateInstanceExtensionProperties);
};

struct render {
  void *vklib;
  struct vk_functions vk;
  VkInstance instance;
};

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
    return RENDER_ERROR_VULKAN_PREINST_LOAD;

  load(CreateInstance);
  load(EnumerateInstanceExtensionProperties);
  return RENDER_ERROR_NONE;

#undef load
}

static int get_instance_extensions(
  struct render *r,
  uint32_t *out_n_extensions,
  VkExtensionProperties **out_extensions
) {
  uint32_t n_extensions;
  VkExtensionProperties *extensions;
  VkResult result;

  result = r->vk.EnumerateInstanceExtensionProperties(
    NULL,
    &n_extensions,
    NULL
  );
  if (result != VK_SUCCESS) {
    return RENDER_ERROR_VULKAN_ENUMERATE_INSTANCE_EXTENSIONS;
  }
  if (!n_extensions) return 0;
  extensions = malloc(n_extensions * sizeof(VkExtensionProperties));
  if (!extensions) return RENDER_ERROR_MEMORY;
  r->vk.EnumerateInstanceExtensionProperties(
    NULL,
    &n_extensions,
    extensions
  );
  *out_n_extensions = n_extensions;
  *out_extensions = extensions;
  return RENDER_ERROR_NONE;
}

/* **************************************** */
/* Public */
/* **************************************** */

struct render *render_new(struct window *w, int *out_err) {
#define chk(expr) if ((expr)) goto err

  int rc;
  uint32_t n_instance_extensions;
  VkExtensionProperties *instance_extensions;
  struct render *out;

  out = malloc(sizeof(struct render));
  if (!out) return NULL;
  memset(out, 0, sizeof(struct render));
  chk(rc = load_vulkan(out));
  chk(rc = load_preinstance_functions(out));
  chk(rc = get_instance_extensions(
        out,
        &n_instance_extensions,
        &instance_extensions
      ));
  /* chk(rc = check_extensions(n_instance_extensions, instance_extensions)); */
  /* chk(rc = create_instance(out)); */
  return out;

 err:
  render_del(out);
  if (out_err) *out_err = rc;
  return NULL;

#undef chk
}

void render_del(struct render *r) {
  if (!r) return;
  dlclose(r->vklib);
  free(r);
}

int render_configure(struct render *r, uint16_t width, uint16_t height) {
  return 0;
}

void render_update(struct render *r) {
  
}
