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
#include "vector.h"
#include <stdlib.h>
#include <string.h>

struct uniforms {
  struct vec3 color;
};

/* TODO: Globals for now, will be passed in later */
VkVertexInputBindingDescription bindings[] = {
  { 0, sizeof(float) * 6, VK_VERTEX_INPUT_RATE_VERTEX }
};
VkVertexInputAttributeDescription attrs[] = {
  { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
  { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 }
};

/* TODO: Refactor to pass in descriptor set layouts */
static int create_pipeline_layout(
  struct render_pass *rp,
  VkPipelineLayout *out_layout
) {
  VkPipelineLayoutCreateInfo create_info = { 0 };
  VkDescriptorSetLayoutCreateInfo layout_create_info = { 0 };
  VkResult result;

  layout_create_info.bindingCount = 1;
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  create_info.setLayoutCount = (uint32_t) rp->n_desc_layouts;
  create_info.pSetLayouts = rp->desc_layouts;
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

  /* TODO: Move these out and pass them in as parameters */
  chkerrg(
    err = create_pipeline_layout(rp, &layout),
    err_pipeline_layout
  );
  chkerrg(err = create_render_pass(rp), err_render_pass);

  /* TODO: Move these out and pass them in as parameters */
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

/* TODO: Pass in descriptor layouts instead of relying on rp to have them */
static int create_descriptor_sets(struct render_pass *rp) {
  size_t i;
  VkDescriptorSetAllocateInfo alloc_info = { 0 };
  VkDescriptorSetLayout *layouts;
  VkResult result;

  layouts =
    malloc(sizeof(VkDescriptorSetLayout) * rp->device->n_swapchain_images);
  if (!layouts) goto err_layouts_memory;
  for (i = 0; i < rp->device->n_swapchain_images; ++i) {
    layouts[i] = rp->desc_layouts[0];
  }
  rp->desc_sets =
    malloc(sizeof(VkDescriptorSet) * rp->device->n_swapchain_images);
  if (!rp->desc_sets) goto err_desc_memory;
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = rp->desc_pool;
  alloc_info.descriptorSetCount = (uint32_t) rp->device->n_swapchain_images;
  alloc_info.pSetLayouts = layouts;
  result = rp->device->vkAllocateDescriptorSets(
    rp->device->device,
    &alloc_info,
    rp->desc_sets
  );
  if (result != VK_SUCCESS) goto err_desc_sets;
  free(layouts);
  return RENDER_ERROR_NONE;

 err_desc_sets:
  free(rp->desc_sets);
 err_desc_memory:
  free(layouts);
 err_layouts_memory:
  return RENDER_ERROR_VULKAN_DESCRIPTOR_SET;
}

static int write_descriptor_sets(struct render_pass *rp) {
  size_t i;
  VkDescriptorBufferInfo buffer_info = { 0 };
  VkWriteDescriptorSet write_info = { 0 };

  for (i = 0; i < rp->device->n_swapchain_images; ++i) {
    buffer_info.buffer = rp->uniforms[i].buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(struct uniforms);
    write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_info.dstSet = rp->desc_sets[i];
    write_info.dstBinding = 0;
    write_info.dstArrayElement = 0;
    write_info.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_info.descriptorCount = 1;
    write_info.pBufferInfo = &buffer_info;
    rp->device->vkUpdateDescriptorSets(
      rp->device->device,
      1,
      &write_info,
      0,
      NULL
    );
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

static int create_descriptor_pool(struct render_pass *rp) {
  VkDescriptorPoolSize size = { 0 };
  VkDescriptorPoolCreateInfo create_info = { 0 };
  VkResult result;

  size.descriptorCount = rp->device->n_swapchain_images;
  size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  create_info.maxSets = rp->device->n_swapchain_images;
  create_info.poolSizeCount = 1;
  create_info.pPoolSizes = &size;
  result = rp->device->vkCreateDescriptorPool(
    rp->device->device,
    &create_info,
    NULL,
    &rp->desc_pool
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_DESCRIPTOR_POOL;

  return RENDER_ERROR_NONE;
}

static int create_command_buffers(struct render_pass *rp) {
  int err = RENDER_ERROR_VULKAN_COMMAND_BUFFER;
  VkCommandBufferAllocateInfo alloc_info = { 0 };
  VkResult result;

  rp->command_buffers = malloc(
    sizeof(VkCommandBuffer) * rp->device->n_swapchain_images
  );
  if (!rp->command_buffers) goto err_command_buffer_memory;
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
 err_command_buffer_memory:
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
  struct render_buffer test_buffer;

  chkerrg(
    err = render_memory_create_buffer(
      &rp->device->memory,
      16,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      size_verts,
      &rp->vertices
    ),
    err_vertices
  );
  chkerrg(
    err = render_memory_create_buffer(
      &rp->device->memory,
      16,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      size_indices,
      &rp->indices
    ),
    err_indices
  );
  chkerrg(
    err = render_buffer_write(&rp->vertices, size_verts, vertices),
    err_write_vertices
  );
  chkerrg(
    err = render_buffer_write(&rp->indices, size_indices, indices),
    err_write_indices
  );

  /* TEST */
  {
    chkerrg(
      err = render_memory_create_buffer(
        &rp->uniform_memory,
        16,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        12,
        &test_buffer
      ),
      err_indices
    );
    render_buffer_destroy(&test_buffer);
  }
  return RENDER_ERROR_NONE;


 err_write_indices:
 err_write_vertices:
  render_buffer_destroy(&rp->indices);
 err_indices:
  render_buffer_destroy(&rp->vertices);
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
        &rp->vertices.buffer,
        offsets
      );
      rp->device->vkCmdBindIndexBuffer(
        rp->command_buffers[i],
        rp->indices.buffer,
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

static int teardown_pass(struct render_pass *rp) {
  size_t i;

  for (i = 0; i < rp->device->n_swapchain_images; ++i) {
    render_buffer_destroy(&rp->uniforms[i]);
  }
  render_memory_reset(&rp->uniform_memory);

  for (i = 0; i < rp->n_desc_layouts; ++i) {
    rp->device->vkDestroyDescriptorSetLayout(
      rp->device->device,
      rp->desc_layouts[i],
      NULL
    );
  }
  free(rp->desc_layouts);

  rp->device->vkDestroyDescriptorPool(rp->device->device, rp->desc_pool, NULL);

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

  free(rp->command_buffers);
  rp->device->vkDestroyRenderPass(rp->device->device, rp->render_pass, NULL);
  rp->device->vkDestroyPipeline(rp->device->device, rp->pipeline, NULL);

  return RENDER_ERROR_NONE;
}

static int create_descriptor_layouts(
  struct render_pass *rp,
  size_t n_descriptors,
  VkDescriptorSetLayout *out_descriptors
) {
  size_t i;
  VkDescriptorSetLayoutBinding binding = { 0 };
  VkDescriptorSetLayoutCreateInfo create_info = { 0 };
  VkResult result;

  binding.binding = 0;
  binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  binding.descriptorCount = 1;
  binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  create_info.bindingCount = 1;
  create_info.pBindings = &binding;
  for (i = 0; i < n_descriptors; ++i) {
    result = rp->device->vkCreateDescriptorSetLayout(
      rp->device->device,
      &create_info,
      NULL,
      &out_descriptors[i]
    );
    if (result != VK_SUCCESS) goto err_descriptors;

    continue;

  err_descriptors:
    while (i--) rp->device->vkDestroyDescriptorSetLayout(
      rp->device->device,
      out_descriptors[i],
      NULL
    );
    return RENDER_ERROR_VULKAN_DESCRIPTOR_SET;
  }
  return RENDER_ERROR_NONE;
}

static int create_uniform_buffers(struct render_pass *rp) {
  size_t i;

  for (i = 0; i < rp->device->n_swapchain_images; ++i) {
    int err;

    err = render_memory_create_buffer(
      &rp->uniform_memory,
      16,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      sizeof(struct uniforms),
      &rp->uniforms[i]
    );
    if (err) goto err_loop;

    continue;

  err_loop:
    while (i--) render_buffer_destroy(&rp->uniforms[i]);
    free(rp->uniforms);
    return RENDER_ERROR_VULKAN_BUFFER;
  }

  return RENDER_ERROR_NONE;
}

static int create_pass(struct render_pass *rp) {
  int err = RENDER_ERROR_VULKAN_SWAPCHAIN_RECREATE;
  size_t i;

  chkerrg(err = create_descriptor_pool(rp), err_descriptor_pool);
  /* TODO: Don't hard code number of descriptor layouts */
  rp->desc_layouts = malloc(sizeof(VkDescriptorSetLayout));
  if (!rp->desc_layouts) goto err_desc_layouts_memory;
  rp->n_desc_layouts = 1;
  chkerrg(
    err = create_descriptor_layouts(rp, 1, rp->desc_layouts),
    err_descriptors
  );

  chkerrg(
    err = create_uniform_buffers(rp),
    err_uniform_buffers
  );
  for (i = 0; i < rp->device->n_swapchain_images; ++i) {
    struct uniforms data = { 0 };

    render_buffer_write(
      &rp->uniforms[i],
      sizeof(struct uniforms),
      (void *) &data
    );
  }

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

  chkerrg(
    err = create_descriptor_sets(rp),
    err_descriptor_sets
  );
  chkerrg(
    err = write_descriptor_sets(rp),
    err_write_descriptor_sets
  );

  chkerrg(err = create_command_buffers(rp), err_command_buffers);
  chkerrg(err = write_buffers(rp), err_write_buffers);

  return RENDER_ERROR_NONE;

 err_write_buffers:
  free(rp->command_buffers);
  rp->device->vkFreeCommandBuffers(
    rp->device->device,
    rp->command_pool,
    (uint32_t) rp->device->n_swapchain_images,
    rp->command_buffers
  );
 err_command_buffers:
 err_write_descriptor_sets:
 err_descriptor_sets:
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
  {
    size_t i;

    for (i = 0; i < rp->device->n_swapchain_images; ++i) {
      render_buffer_destroy(&rp->uniforms[i]);
    }
    free(rp->uniforms);
  }
 err_uniform_buffers:
  {
    size_t i;

    for (i = 0; i < rp->n_desc_layouts; ++i) {
      rp->device->vkDestroyDescriptorSetLayout(
        rp->device->device,
        rp->desc_layouts[i],
        NULL
      );
    }
  }
 err_descriptors:
  free(rp->desc_layouts);
 err_desc_layouts_memory:
  rp->device->vkDestroyDescriptorPool(rp->device->device, rp->desc_pool, NULL);
 err_descriptor_pool:
  return err;
}

static int recreate_pass(struct render_pass *rp) {
  int err = RENDER_ERROR_VULKAN_SWAPCHAIN_RECREATE;

  render_device_recreate_swapchain(rp->device);
  if ((err = render_device_recreate_swapchain(rp->device))) return err;
  teardown_pass(rp);
  if ((err = create_pass(rp))) return err;
  return RENDER_ERROR_NONE;
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

  memset(rp, 0, sizeof(struct render_pass));
  rp->device = rd;

  chkerrg(
    err = render_memory_init(
      &rp->uniform_memory,
      rd,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      MB_TO_BYTES(1)
    ),
    err_uniform_memory
  );

  rp->uniforms =
    malloc(sizeof(struct render_buffer) * rd->n_swapchain_images);

  chkerrg(err = create_command_pool(rp), err_command_pool);

  chkerrg(err = create_vertex_data(rp), err_vertex_data);

  rp->framebuffers = malloc(
    sizeof(VkFramebuffer) * rp->device->n_swapchain_images
  );
  if (!rp->framebuffers) goto err_framebuffer_memory;

  rp->image_views = malloc(
    sizeof(VkImageView) * rp->device->n_swapchain_images
  );
  if (!rp->image_views) goto err_image_view_memory;

  chkerrg(err = create_pass(rp), err_pass);

  return RENDER_ERROR_NONE;

 err_pass:
  free(rp->image_views);
 err_image_view_memory:
  free(rp->framebuffers);
 err_framebuffer_memory:
  render_buffer_destroy(&rp->vertices);
  render_buffer_destroy(&rp->indices);
 err_vertex_data:
  rp->device->vkDestroyRenderPass(rp->device->device, rp->render_pass, NULL);
  rp->device->vkDestroyPipeline(rp->device->device, rp->pipeline, NULL);
  rp->device->vkDestroyCommandPool(rp->device->device, rp->command_pool, NULL);
 err_command_pool:
  render_memory_deinit(&rp->uniform_memory);
 err_uniform_memory:
  return err;
}

void render_pass_deinit(struct render_pass *rp) {
  teardown_pass(rp);
  render_memory_deinit(&rp->uniform_memory);
  free(rp->uniforms);
  /* TODO: remove vertices and indices */
  render_buffer_destroy(&rp->vertices);
  render_buffer_destroy(&rp->indices);
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
    recreate_pass(rp);
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
    recreate_pass(rp);
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

