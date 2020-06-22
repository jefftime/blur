#include "render.h"
#include "error.h"

int render_data_init(struct render_data *rd, struct render_pipeline *rp) {
  /* int err = RENDER_ERROR_VULKAN_VERTEX_DATA; */
  /* size_t size_verts = sizeof(float) * 6 * 4; */
  /* size_t size_indices = sizeof(uint16_t) * 3 * 2; */
  /* float vertices[] = { */
  /*   -0.5f, -0.5f, 0.0f, 1.0, 0.0f, 0.0f, */
  /*   -0.5f,  0.5f, 0.0f, 0.0, 1.0f, 0.0f, */
  /*    0.5f,  0.5f, 0.0f, 1.0, 1.0f, 1.0f, */
  /*    0.5f, -0.5f, 0.0f, 0.0, 1.0f, 0.0f */
  /* }; */
  /* uint16_t indices[] = { 0, 1, 2, 2, 3, 0 }; */

  /* if (!rd) return RENDER_ERROR_NULL; */
  /* if (!rp) return RENDER_ERROR_NULL; */
  /* rd->pipeline = rp; */
  /* cherrg( */
  /*   err = create_buffer( */
  /*     rp, */
  /*     &rp->vertex_buffer, */
  /*     size_verts, */
  /*     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT */
  /*   ), */
  /*   err_vertices */
  /* ); */
  /* chkerrg( */
  /*   err = create_buffer( */
  /*     rp, */
  /*     &rp->index_buffer, */
  /*     size_indices, */
  /*     VK_BUFFER_USAGE_INDEX_BUFFER_BIT */
  /*   ), */
  /*   err_indices */
  /* ); */

  return RENDER_ERROR_NONE;

 err_indices:
  rd->pipeline->device->vkDestroyBuffer(
    rd->pipeline->device->device,
    rd->vertex_buffer,
    NULL
  );
 err_vertices:
  return err;
}

void render_data_deinit(struct render_data *rd) {
}
