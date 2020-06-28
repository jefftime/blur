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
#include "shaders/default_vert.h"
#include "shaders/default_frag.h"
#include <stdlib.h>

/* XXX: Globals for now, will be passed in later */
VkVertexInputBindingDescription bindings[] = {
  { 0, sizeof(float) * 6, VK_VERTEX_INPUT_RATE_VERTEX },
};
VkVertexInputAttributeDescription attrs[] = {
  { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
  { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 }
};

static int create_pipeline_layout(
  struct render_pass *rp,
  VkPipelineLayout *out_layout
) {
  VkPipelineLayoutCreateInfo create_info = { 0 };
  VkResult result;

  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  create_info.setLayoutCount = 0;
  create_info.pushConstantRangeCount = 0;
  result = rp->device->vkCreatePipelineLayout(
    rp->device->device,
    &create_info,
    NULL,
    out_layout
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_PIPELINE_LAYOUT;
  return RENDER_ERROR_NONE;
}

static int create_render_pass(struct render_pass *rp) {
  VkSubpassDependency dependency = { 0 };
  VkAttachmentDescription attachment = { 0 };
  VkAttachmentReference reference = {
    0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  };
  VkSubpassDescription subpass = { 0 };
  VkRenderPassCreateInfo create_info = { 0 };
  VkResult result;

  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  attachment.format = rp->device->surface_format.format;
  attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &reference;
  create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  create_info.attachmentCount = 1;
  create_info.pAttachments = &attachment;
  create_info.subpassCount = 1;
  create_info.pSubpasses = &subpass;
  create_info.dependencyCount = 1;
  create_info.pDependencies = &dependency;
  result = rp->device->vkCreateRenderPass(
    rp->device->device,
    &create_info,
    NULL,
    &rp->render_pass
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_RENDER_PASS;
  return RENDER_ERROR_NONE;
}

static int create_shader(
  struct render_pass *rp,
  size_t len,
  uint32_t *src,
  VkShaderModule *out_module
) {
  VkShaderModuleCreateInfo create_info = { 0 };
  VkResult result;

  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = len;
  create_info.pCode = src;
  result = rp->device->vkCreateShaderModule(
    rp->device->device,
    &create_info,
    NULL,
    out_module
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_SHADER_MODULE;
  return RENDER_ERROR_NONE;
}

static int create_pipeline(
  struct render_pass *rp,
  size_t n_bindings,
  VkVertexInputBindingDescription *bindings,
  size_t n_attrs,
  VkVertexInputAttributeDescription *attrs
) {
  int err = RENDER_ERROR_VULKAN_GRAPHICS_PIPELINE;
  size_t vlen, flen;
  VkShaderModule vmodule = { 0 }, fmodule = { 0 };
  VkPipelineShaderStageCreateInfo shader_info[] = { { 0 }, { 0 } };
  VkPipelineVertexInputStateCreateInfo vertex_info = { 0 };
  VkPipelineInputAssemblyStateCreateInfo assembly_info = { 0 };
  VkViewport viewport = { 0 };
  VkRect2D scissor = { 0 };
  VkPipelineViewportStateCreateInfo viewport_info = { 0 };
  VkPipelineRasterizationStateCreateInfo raster_info = { 0 };
  VkPipelineMultisampleStateCreateInfo multisample_info = { 0 };
  VkPipelineDepthStencilStateCreateInfo depth_info = { 0 };
  VkPipelineColorBlendAttachmentState color_attachment = { 0 };
  VkPipelineColorBlendStateCreateInfo color_info = { 0 };
  VkPipelineDynamicStateCreateInfo dynamic_info = { 0 };
  VkPipelineLayout layout = { 0 };
  VkGraphicsPipelineCreateInfo graphics_pipeline = { 0 };
  VkResult result;

  vlen =
    sizeof(uint32_t) * sizeof(default_vert_src) / sizeof(default_vert_src[0]);
  flen =
    sizeof(uint32_t) * sizeof(default_frag_src) / sizeof(default_frag_src[0]);

  chkerrg(err = create_pipeline_layout(rp, &layout), err_pipeline_layout);
  chkerrg(err = create_render_pass(rp), err_render_pass);

  chkerrg(
    err = create_shader(rp, vlen, (uint32_t *) default_vert_src, &vmodule),
    err_vmodule
  );
  chkerrg(
    err = create_shader(rp, flen, (uint32_t *) default_frag_src, &fmodule),
    err_fmodule
  );
  shader_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shader_info[0].module = vmodule;
  shader_info[0].pName = "main";
  shader_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shader_info[1].module = fmodule;
  shader_info[1].pName = "main";

  vertex_info.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_info.vertexBindingDescriptionCount = (uint32_t) n_bindings;
  vertex_info.pVertexBindingDescriptions = bindings;
  vertex_info.vertexAttributeDescriptionCount = (uint32_t) n_attrs;
  vertex_info.pVertexAttributeDescriptions = attrs;

  assembly_info.sType =
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  assembly_info.primitiveRestartEnable = VK_FALSE;

  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float) rp->device->swap_extent.width;
  viewport.height = (float) rp->device->swap_extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent = rp->device->swap_extent;

  viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info.viewportCount = 1;
  viewport_info.pViewports = &viewport;
  viewport_info.scissorCount = 1;
  viewport_info.pScissors = &scissor;

  raster_info.sType =
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  raster_info.depthClampEnable = VK_FALSE;
  raster_info.rasterizerDiscardEnable = VK_FALSE;
  raster_info.polygonMode = VK_POLYGON_MODE_FILL;
  raster_info.cullMode = VK_CULL_MODE_NONE;
  raster_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  raster_info.depthBiasEnable = VK_FALSE;
  raster_info.lineWidth = 1.0;

  multisample_info.sType =
    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisample_info.sampleShadingEnable = VK_FALSE;
  multisample_info.minSampleShading = 1.0f;
  multisample_info.pSampleMask = NULL;
  multisample_info.alphaToCoverageEnable = VK_FALSE;
  multisample_info.alphaToOneEnable = VK_FALSE;

  depth_info.sType =
    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_info.depthTestEnable = VK_FALSE;
  depth_info.depthWriteEnable = VK_FALSE;
  depth_info.depthBoundsTestEnable = VK_FALSE;
  depth_info.stencilTestEnable = VK_FALSE;

  color_attachment.blendEnable = VK_FALSE;
  color_attachment.colorWriteMask = (
    VK_COLOR_COMPONENT_R_BIT
    | VK_COLOR_COMPONENT_G_BIT
    | VK_COLOR_COMPONENT_B_BIT
    | VK_COLOR_COMPONENT_A_BIT
  );

  color_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_info.logicOpEnable = VK_FALSE;
  color_info.logicOp = VK_LOGIC_OP_COPY;
  color_info.attachmentCount = 1;
  color_info.pAttachments = &color_attachment;
  color_info.blendConstants[0] = 0.0f;
  color_info.blendConstants[1] = 0.0f;
  color_info.blendConstants[2] = 0.0f;
  color_info.blendConstants[3] = 0.0f;

  dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_info.dynamicStateCount = 0;

  graphics_pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  graphics_pipeline.stageCount = sizeof(shader_info) / sizeof(shader_info[0]);
  graphics_pipeline.pStages = shader_info;
  graphics_pipeline.pVertexInputState = &vertex_info;
  graphics_pipeline.pInputAssemblyState = &assembly_info;
  graphics_pipeline.pViewportState = &viewport_info;
  graphics_pipeline.pRasterizationState = &raster_info;
  graphics_pipeline.pMultisampleState = &multisample_info;
  graphics_pipeline.pDepthStencilState = &depth_info;
  graphics_pipeline.pColorBlendState = &color_info;
  graphics_pipeline.pDynamicState = &dynamic_info;
  graphics_pipeline.layout = layout;
  graphics_pipeline.renderPass = rp->render_pass;
  graphics_pipeline.subpass = 0;
  graphics_pipeline.basePipelineHandle = VK_NULL_HANDLE;
  graphics_pipeline.basePipelineIndex = -1;
  result = rp->device->vkCreateGraphicsPipelines(
    rp->device->device,
    VK_NULL_HANDLE,
    1,
    &graphics_pipeline,
    NULL,
    &rp->pipeline
  );
  if (result != VK_SUCCESS) goto err_graphics_pipeline;
  rp->device->vkDestroyShaderModule(rp->device->device, vmodule, NULL);
  rp->device->vkDestroyShaderModule(rp->device->device, fmodule, NULL);
  rp->device->vkDestroyPipelineLayout(rp->device->device, layout, NULL);

  return RENDER_ERROR_NONE;

 err_graphics_pipeline:
  rp->device->vkDestroyShaderModule(rp->device->device, fmodule, NULL);
 err_fmodule:
  rp->device->vkDestroyShaderModule(rp->device->device, vmodule, NULL);
 err_vmodule:
  rp->device->vkDestroyRenderPass(rp->device->device, rp->render_pass, NULL);
 err_render_pass:
  rp->device->vkDestroyPipelineLayout(rp->device->device, layout, NULL);
 err_pipeline_layout:
  return err;
}

static int create_image_views(
  struct render_pass *rp,
  size_t n_images,
  VkImage *images,
  VkImageView *out_image_views
) {
  size_t i;
  VkImageViewCreateInfo create_info = { 0 };
  VkResult result;

  for (i = 0; i < n_images; ++i) {
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = images[i];
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = rp->device->surface_format.format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;
    result = rp->device->vkCreateImageView(
      rp->device->device,
      &create_info,
      NULL,
      out_image_views + i
    );
    if (result != VK_SUCCESS) goto err_loop_image_views;

    continue;

  err_loop_image_views:
    while (i--) {
      rp->device->vkDestroyImageView(
        rp->device->device,
        rp->image_views[i],
        NULL
      );
    }
    return RENDER_ERROR_VULKAN_IMAGE_VIEW;
  }
  return RENDER_ERROR_NONE;
}

static int create_framebuffers(
  struct render_pass *rp,
  size_t n_framebuffers,
  VkImageView *images,
  VkFramebuffer *out_framebuffers
) {
  size_t i;
  VkFramebufferCreateInfo create_info = { 0 };
  VkResult result;

  for (i = 0; i < n_framebuffers; ++i) {
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.renderPass = rp->render_pass;
    create_info.attachmentCount = 1;
    create_info.pAttachments = images + i;
    create_info.width = rp->device->swap_extent.width;
    create_info.height = rp->device->swap_extent.height;
    create_info.layers = 1;
    result = rp->device->vkCreateFramebuffer(
      rp->device->device,
      &create_info,
      NULL,
      out_framebuffers + i
    );
    if (result != VK_SUCCESS) goto err_loop_framebuffer;

    continue;

  err_loop_framebuffer:
    while (i--) {
      rp->device->vkDestroyFramebuffer(
        rp->device->device,
        out_framebuffers[i],
        NULL
      );
    }
    return RENDER_ERROR_VULKAN_FRAMEBUFFER;
  }

  return RENDER_ERROR_NONE;
}

static int create_command_pool(struct render_pass *rp) {
  VkCommandPoolCreateInfo create_info = { 0 };
  VkResult result;

  create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  create_info.queueFamilyIndex = (uint32_t) rp->device->graphics_index;
  result = rp->device->vkCreateCommandPool(
    rp->device->device,
    &create_info,
    NULL,
    &rp->command_pool
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_COMMAND_POOL;
  return RENDER_ERROR_NONE;
}

static int create_command_buffers(struct render_pass *rp) {
  int err = RENDER_ERROR_VULKAN_COMMAND_BUFFER;
  VkCommandBufferAllocateInfo alloc_info = { 0 };
  VkResult result;

  rp->command_buffers = malloc(
    sizeof(VkCommandBuffer) * rp->device->n_swapchain_images
  );
  if (!rp->command_buffers) goto err_mem_command_buffer;
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = rp->command_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = (uint32_t) rp->device->n_swapchain_images;
  result = rp->device->vkAllocateCommandBuffers(
    rp->device->device,
    &alloc_info,
    rp->command_buffers
  );
  if (result != VK_SUCCESS) goto err_command_buffer;
  return RENDER_ERROR_NONE;

 err_command_buffer:
  free(rp->command_buffers);
 err_mem_command_buffer:
  return err;
}

static int create_vertex_data(struct render_pass *rp) {
  int err = RENDER_ERROR_VULKAN_VERTEX_DATA;
  size_t size_verts = sizeof(float) * 6 * 4;
  size_t size_indices = sizeof(uint16_t) * 3 * 2;
  float vertices[] = {
    -0.5f, -0.5f, 0.0f, 1.0, 0.0f, 0.0f,
    -0.5f,  0.5f, 0.0f, 0.0, 1.0f, 0.0f,
     0.5f,  0.5f, 0.0f, 1.0, 1.0f, 1.0f,
     0.5f, -0.5f, 0.0f, 0.0, 1.0f, 0.0f
  };
  uint16_t indices[] = { 0, 1, 2, 2, 3, 0 };

  chkerrg(
    err = create_buffer(
      rp,
      &rp->vertex_buffer,
      size_verts,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    ),
    err_vertices
  );
  chkerrg(
    err = create_buffer(
      rp,
      &rp->index_buffer,
      size_indices,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    ),
    err_indices
  );
  chkerrg(
    err = alloc_buffer(rp, rp->vertex_buffer, &rp->vertex_memory),
    err_vertex_memory
  );
  chkerrg(
    err = alloc_buffer(rp, rp->index_buffer, &rp->index_memory),
    err_index_memory
  );
  chkerrg(
    err = write_data(rp, rp->vertex_memory, size_verts, vertices),
    err_write_vertices
  );
  chkerrg(
    err = write_data(rp, rp->index_memory, size_indices, indices),
    err_write_indices
  );
  return RENDER_ERROR_NONE;


 err_write_indices:
 err_write_vertices:
  rp->device->vkFreeMemory(rp->device->device, rp->index_memory, NULL);
 err_index_memory:
  rp->device->vkFreeMemory(rp->device->device, rp->vertex_memory, NULL);
 err_vertex_memory:
  rp->device->vkDestroyBuffer(rp->device->device, rp->vertex_buffer, NULL);
 err_indices:
  rp->device->vkDestroyBuffer(rp->device->device, rp->vertex_buffer, NULL);
 err_vertices:
  return err;
}

static int write_buffers(struct render_pass *rp) {
  size_t i;
  VkCommandBufferBeginInfo begin_info = { 0 };
  VkResult result;

  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  /* begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; */
  for (i = 0; i < rp->device->n_swapchain_images; ++i) {
    VkRenderPassBeginInfo render_info = { 0 };
    VkClearValue clear_value = { { { 0 } } };

    result = rp->device->vkBeginCommandBuffer(
      rp->command_buffers[i],
      &begin_info
    );
    if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_COMMAND_BUFFER_BEGIN;
    clear_value.color.float32[3] = 1.0f;
    render_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_info.renderPass = rp->render_pass;
    render_info.framebuffer = rp->framebuffers[i];
    render_info.renderArea.offset.x = 0;
    render_info.renderArea.offset.y = 0;
    render_info.renderArea.extent = rp->device->swap_extent;
    render_info.clearValueCount = 1;
    render_info.pClearValues = &clear_value;
    rp->device->vkCmdBeginRenderPass(
      rp->command_buffers[i],
      &render_info,
      VK_SUBPASS_CONTENTS_INLINE
    );
    {
      VkDeviceSize offsets[] = { 0 };

      rp->device->vkCmdBindPipeline(
        rp->command_buffers[i],
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        rp->pipeline
      );
      rp->device->vkCmdBindVertexBuffers(
        rp->command_buffers[i],
        0,
        1,
        &rp->vertex_buffer,
        offsets
      );
      rp->device->vkCmdBindIndexBuffer(
        rp->command_buffers[i],
        rp->index_buffer,
        0,
        VK_INDEX_TYPE_UINT16
      );
      rp->device->vkCmdDrawIndexed(rp->command_buffers[i], 6, 1, 0, 0, 0);
    }
    rp->device->vkCmdEndRenderPass(rp->command_buffers[i]);
    result = rp->device->vkEndCommandBuffer(rp->command_buffers[i]);
    if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_COMMAND_BUFFER_END;
  }
  return RENDER_ERROR_NONE;
}

static int teardown_swapchain(struct render_pass *rp) {
  size_t i;

  for (i = 0; i < rp->device->n_swapchain_images; ++i) {
    rp->device->vkDestroyImageView(
      rp->device->device,
      rp->image_views[i],
      NULL
    );
    rp->device->vkDestroyFramebuffer(
      rp->device->device,
      rp->framebuffers[i],
      NULL
    );
  }
  rp->device->vkFreeCommandBuffers(
    rp->device->device,
    rp->command_pool,
    (uint32_t) rp->device->n_swapchain_images,
    rp->command_buffers
  );
  rp->device->vkDestroyRenderPass(rp->device->device, rp->render_pass, NULL);
  rp->device->vkDestroyPipeline(rp->device->device, rp->pipeline, NULL);
  return RENDER_ERROR_NONE;
}

static int recreate_swapchain(struct render_pass *rp) {
  int err = RENDER_ERROR_VULKAN_SWAPCHAIN_RECREATE;

  render_device_recreate_swapchain(rp->device);
  teardown_swapchain(rp);
  chkerrg(
    err = create_pipeline(
      rp,
      sizeof(bindings) / sizeof(bindings[0]),
      bindings,
      sizeof(attrs) / sizeof(attrs[0]),
      attrs
    ),
    err_pipeline
  );

  chkerrg(
    err = create_image_views(
      rp,
      rp->device->n_swapchain_images,
      rp->device->swapchain_images,
      rp->image_views
    ),
    err_image_views
  );
  chkerrg(
    err = create_framebuffers(
      rp,
      rp->device->n_swapchain_images,
      rp->image_views,
      rp->framebuffers
    ),
    err_framebuffers
  );
  chkerrg(err = create_command_buffers(rp), err_command_buffers);
  chkerrg(err = write_buffers(rp), err_write_buffers);
  return RENDER_ERROR_NONE;

 err_write_buffers:
  rp->device->vkFreeCommandBuffers(
    rp->device->device,
    rp->command_pool,
    (uint32_t) rp->device->n_swapchain_images,
    rp->command_buffers
  );
 err_command_buffers:
  {
    size_t i = rp->device->n_swapchain_images;

    while (i--) rp->device->vkDestroyFramebuffer(
      rp->device->device,
      rp->framebuffers[i],
      NULL
    );
  }
 err_framebuffers:
  {
    size_t i = rp->device->n_swapchain_images;

    while (i--) rp->device->vkDestroyImageView(
      rp->device->device,
      rp->image_views[i],
      NULL
    );
  }
 err_image_views:
 err_pipeline:
  return err;
}

/* **************************************** */
/* Public */
/* **************************************** */

int render_pass_init(
  struct render_pass *rp,
  struct render_device *rd
) {
  int err = RENDER_ERROR_VULKAN_DEVICE;

  if (!rp) return RENDER_ERROR_NULL;
  if (!rd) return RENDER_ERROR_NULL;
  rp->device = rd;

  /* Pipeline */
  chkerrg(
    err = create_pipeline(
      rp,
      sizeof(bindings) / sizeof(bindings[0]),
      bindings,
      sizeof(attrs) / sizeof(attrs[0]),
      attrs
    ),
    err_pipeline
  );

  /* Framebuffers */
  rp->framebuffers = malloc(
    sizeof(VkFramebuffer) * rp->device->n_swapchain_images
  );
  if (!rp->framebuffers) goto err_framebuffer_memory;
  rp->image_views = malloc(
    sizeof(VkImageView) * rp->device->n_swapchain_images
  );
  if (!rp->image_views) goto err_image_view_memory;
  chkerrg(
    err = create_image_views(
      rp,
      rp->device->n_swapchain_images,
      rp->device->swapchain_images,
      rp->image_views
    ),
    err_image_views
  );
  chkerrg(
    err = create_framebuffers(
      rp,
      rp->device->n_swapchain_images,
      rp->image_views,
      rp->framebuffers
    ),
    err_framebuffers
  );

  /* Rest of init */
  chkerrg(err = create_command_pool(rp), err_command_pool);
  chkerrg(err = create_command_buffers(rp), err_command_buffers);
  chkerrg(err = create_vertex_data(rp), err_vertex_data);
  chkerrg(err = write_buffers(rp), err_write_buffers);
  return RENDER_ERROR_NONE;

 err_write_buffers:
  rp->device->vkFreeMemory(rp->device->device, rp->index_memory, NULL);
  rp->device->vkFreeMemory(rp->device->device, rp->vertex_memory, NULL);
  rp->device->vkDestroyBuffer(rp->device->device, rp->vertex_buffer, NULL);
  rp->device->vkDestroyBuffer(rp->device->device, rp->vertex_buffer, NULL);
 err_vertex_data:
  rp->device->vkFreeCommandBuffers(
    rp->device->device,
    rp->command_pool,
    (uint32_t) rp->device->n_swapchain_images,
    rp->command_buffers
  );
 err_command_buffers:
  rp->device->vkDestroyCommandPool(rp->device->device, rp->command_pool, NULL);
 err_command_pool:
  {
    size_t i = rp->device->n_swapchain_images;

    while (i--) rp->device->vkDestroyFramebuffer(
      rp->device->device,
      rp->framebuffers[i],
      NULL
    );
  }
 err_framebuffers:
  {
    size_t i = rp->device->n_swapchain_images;

    while (i--) rp->device->vkDestroyImageView(
      rp->device->device,
      rp->image_views[i],
      NULL
    );
  }
 err_image_views:
  free(rp->image_views);
 err_image_view_memory:
  free(rp->framebuffers);
 err_framebuffer_memory:
  rp->device->vkDestroyRenderPass(rp->device->device, rp->render_pass, NULL);
  rp->device->vkDestroyPipeline(rp->device->device, rp->pipeline, NULL);
 err_pipeline:
  return err;
}

void render_pass_deinit(struct render_pass *rp) {
  teardown_swapchain(rp);
  rp->device->vkFreeMemory(rp->device->device, rp->index_memory, NULL);
  rp->device->vkFreeMemory(rp->device->device, rp->vertex_memory, NULL);
  rp->device->vkDestroyBuffer(rp->device->device, rp->index_buffer, NULL);
  rp->device->vkDestroyBuffer(rp->device->device, rp->vertex_buffer, NULL);
  rp->device->vkDestroyCommandPool(rp->device->device, rp->command_pool, NULL);
  free(rp->image_views);
  free(rp->framebuffers);
}

void render_pass_update(struct render_pass *rp) {
  uint32_t image_index;
  VkSubmitInfo submit_info = { 0 };
  VkPipelineStageFlags wait_stages[] = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };
  VkPresentInfoKHR present_info = { 0 };
  VkResult result;

  result = rp->device->vkAcquireNextImageKHR(
    rp->device->device,
    rp->device->swapchain,
    (uint64_t) 2e9L,
    rp->device->image_semaphore,
    VK_NULL_HANDLE,
    &image_index
  );
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreate_swapchain(rp);
    return;
  }
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &rp->device->image_semaphore;
  submit_info.pWaitDstStageMask = wait_stages;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = rp->command_buffers + image_index;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &rp->device->render_semaphore;
  result = rp->device->vkQueueSubmit(
    rp->device->graphics_queue,
    1,
    &submit_info,
    VK_NULL_HANDLE
  );
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    recreate_swapchain(rp);
  }
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &rp->device->render_semaphore;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &rp->device->swapchain;
  present_info.pImageIndices = &image_index;
  rp->device->vkQueuePresentKHR(rp->device->present_queue, &present_info);
  rp->device->vkQueueWaitIdle(rp->device->present_queue);
}

