#include "render.h"

/* **************************************** */
/* Public */
/* **************************************** */

int render_compile_shader(
  struct render_shader *rs,
  struct render_device *rd,
  size_t vlen,
  uint32_t *vsrc,
  size_t flen,
  uint32_t *fsrc
) {
  int err = RENDER_ERROR_VULKAN_SHADER_MODULE;
  VkShaderModuleCreateInfo create_info = { 0 };
  VkResult result;

  if (!rs) return RENDER_ERROR_NULL;
  if (!rd) return RENDER_ERROR_NULL;
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = vlen;
  create_info.pCode = vsrc;
  result = rd->vkCreateShaderModule(
    rd->device,
    &create_info,
    NULL,
    &rs->vert_module
  );
  if (result != VK_SUCCESS) goto err_vert;
  create_info.codeSize = flen;
  create_info.pCode = fsrc;
  result = rd->vkCreateShaderModule(
    rd->device,
    &create_info,
    NULL,
    &rs->frag_module
  );
  if (result != VK_SUCCESS) goto err_frag;
  return RENDER_ERROR_NONE;

 err_frag:
  rd->vkDestroyShaderModule(rd->device, rs->vert_module, NULL);
 err_vert:
  return err;
}
