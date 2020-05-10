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
#include "shaders.h"
#include <stdlib.h>

static int create_shader(
  struct render_pipeline *rp,
  size_t len,
  uint32_t *shader,
  VkShaderModule *out_module
) {
  VkShaderModuleCreateInfo create_info = { 0 };
  VkResult result;

  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = len * sizeof(uint32_t);
  create_info.pCode = shader;
  result = vkCreateShaderModule(
    rp->device,
    &create_info,
    NULL,
    out_module
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_SHADER_MODULE;
  return RENDER_ERROR_NONE;
}

static int create_pipeline_layout(
  struct render_pipeline *rp,
  VkPipelineLayout *out_layout
) {
  VkPipelineLayoutCreateInfo create_info = { 0 };
  VkResult result;

  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  result = vkCreatePipelineLayout(
    rp->device,
    &create_info,
    NULL,
    out_layout
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_PIPELINE_LAYOUT;
  return RENDER_ERROR_NONE;
}

static int create_render_pass(struct render_pipeline *rp) {
  VkResult result;
  VkSubpassDependency dependency = {
    VK_SUBPASS_EXTERNAL,
    0,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    0,
    (
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
      | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    ),
    0
  };
  VkAttachmentDescription attachment = { 0 };
  VkAttachmentReference attachment_ref = {
    0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  };
  VkSubpassDescription subpass = { 0 };
  VkRenderPassCreateInfo create_info = { 0 };

  attachment.format = rp->format.format;
  attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &attachment_ref;
  create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  create_info.attachmentCount = 1;
  create_info.pAttachments = &attachment;
  create_info.subpassCount = 1;
  create_info.pSubpasses = &subpass;
  create_info.dependencyCount = 1;
  create_info.pDependencies = &dependency;
  result = vkCreateRenderPass(
    rp->device,
    &create_info,
    NULL,
    &rp->render_pass
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_RENDER_PASS;
  return RENDER_ERROR_NONE;
}

static int create_pipeline(
  struct render_pipeline *rp,
  size_t n_bindings,
  VkVertexInputBindingDescription *bindings,
  size_t n_attrs,
  VkVertexInputAttributeDescription *attrs
) {
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

  vlen = sizeof(vert_shader) / sizeof(vert_shader[0]);
  flen = sizeof(frag_shader) / sizeof(frag_shader[0]);

  chkerr(create_shader(rp, vlen, (uint32_t *) vert_shader, &vmodule));
  chkerr(create_shader(rp, flen, (uint32_t *) frag_shader, &fmodule));
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
  viewport.width = (float) rp->swap_extent.width;
  viewport.height = (float) rp->swap_extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent.width = rp->swap_extent.width;
  scissor.extent.height = rp->swap_extent.height;

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
  raster_info.cullMode = VK_CULL_MODE_BACK_BIT;
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
  depth_info.depthTestEnable = VK_TRUE;
  depth_info.depthWriteEnable = VK_TRUE;
  depth_info.depthBoundsTestEnable = VK_FALSE;
  depth_info.stencilTestEnable = VK_FALSE;

  color_attachment.blendEnable = VK_FALSE;
  color_attachment.colorWriteMask = (
    VK_COLOR_COMPONENT_R_BIT
    | VK_COLOR_COMPONENT_G_BIT
    | VK_COLOR_COMPONENT_B_BIT
  );

  color_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_info.logicOpEnable = VK_FALSE;
  color_info.attachmentCount = 1;
  color_info.pAttachments = &color_attachment;

  dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_info.dynamicStateCount = 0;

  chkerr(create_pipeline_layout(rp, &layout));
  chkerr(create_render_pass(rp));

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

  result = vkCreateGraphicsPipelines(
    rp->device,
    VK_NULL_HANDLE,
    1,
    &graphics_pipeline,
    NULL,
    &rp->pipeline
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_CREATE_PIPELINE;
  vkDestroyPipelineLayout(rp->device, layout, NULL);
  vkDestroyShaderModule(rp->device, vmodule, NULL);
  vkDestroyShaderModule(rp->device, fmodule, NULL);

  return RENDER_ERROR_NONE;
}

static int create_framebuffers(struct render_pipeline *rp) {
  size_t i;

  rp->image_views = malloc(sizeof(VkImageView) * rp->n_swapchain_images);
  if (!rp->image_views) return RENDER_ERROR_MEMORY;
  rp->framebuffers = malloc(sizeof(VkFramebuffer) * rp->n_swapchain_images);
  if (!rp->framebuffers) {
    free(rp->image_views);
    return RENDER_ERROR_MEMORY;
  }
  for (i = 0; i < rp->n_swapchain_images; ++i) {
    VkImageViewCreateInfo image_view_info = { 0 };
    VkFramebufferCreateInfo framebuffer_info = { 0 };
    VkResult result;

    image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_info.image = rp->swapchain_images[i];
    image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format = rp->format.format;
    image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_info.subresourceRange.baseMipLevel = 0;
    image_view_info.subresourceRange.levelCount = 1;
    image_view_info.subresourceRange.baseArrayLayer = 0;
    image_view_info.subresourceRange.layerCount = 1;
    result = vkCreateImageView(
      rp->device,
      &image_view_info,
      NULL,
      rp->image_views + i
    );
    if (result != VK_SUCCESS) {
      while (i--) {
        vkDestroyImageView(rp->device, rp->image_views[i], NULL);
        vkDestroyFramebuffer(rp->device, rp->framebuffers[i], NULL);
      }
      free(rp->image_views);
      free(rp->framebuffers);
      return RENDER_ERROR_VULKAN_IMAGE_VIEW;
    }
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = rp->render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = rp->image_views + i;
    framebuffer_info.width = rp->swap_extent.width;
    framebuffer_info.height = rp->swap_extent.height;
    framebuffer_info.layers = 1;
    result = vkCreateFramebuffer(
      rp->device,
      &framebuffer_info,
      NULL,
      rp->framebuffers + i
    );
    if (result != VK_SUCCESS) {
      /* Don't forget to destroy the image view we created this iteration */
      vkDestroyImageView(rp->device, rp->image_views[i], NULL);
      while (i--) {
        vkDestroyImageView(rp->device, rp->image_views[i], NULL);
        vkDestroyFramebuffer(rp->device, rp->framebuffers[i], NULL);
      }
      free(rp->image_views);
      free(rp->framebuffers);
      return RENDER_ERROR_VULKAN_FRAMEBUFFER;
    }
  }
  return RENDER_ERROR_NONE;
}

/* **************************************** */
/* Public */
/* **************************************** */

int render_init_pipeline(
  struct render_pipeline *rp,
  struct render *r,
  uint16_t width,
  uint16_t height
) {
  VkVertexInputBindingDescription bindings[] = {
    { 0, sizeof(float) * 6, VK_VERTEX_INPUT_RATE_VERTEX }
  };
  VkVertexInputAttributeDescription attrs[] = {
    { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
    { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 }
  };

  if (!rp) return RENDER_ERROR_NULL;
  if (!r) return RENDER_ERROR_NULL;
  if (!r->active_device) return RENDER_ERROR_VULKAN_NO_DEVICE;
  rp->device = *r->active_device;
  rp->format = r->formats[r->active_device_index];
  rp->swap_extent = r->swap_extent;
  rp->n_swapchain_images = r->n_swapchain_images;
  rp->swapchain_images = r->swapchain_images;
  chkerr(
    create_pipeline(
      rp,
      sizeof(bindings) / sizeof(bindings[0]),
      bindings,
      sizeof(attrs) / sizeof(attrs[0]),
      attrs
    )
  );
  chkerrf(create_framebuffers(rp), render_deinit_pipeline(rp));
  /* chkerrf(create_command_pool(r), render_deinit_pipeline(rp)); */
  /* chkerrf(create_command_buffers(r), render_deinit_pipeline(rp)); */
  return RENDER_ERROR_NONE;
}

void render_deinit_pipeline(struct render_pipeline *rp) {
  size_t i;

  for (i = 0; i < rp->n_swapchain_images; ++i) {
    vkDestroyImageView(rp->device, rp->image_views[i], NULL);
    vkDestroyFramebuffer(rp->device, rp->framebuffers[i], NULL);
  }
  free(rp->image_views);
  free(rp->framebuffers);
  vkDestroyPipeline(rp->device, rp->pipeline, NULL);
  vkDestroyRenderPass(rp->device, rp->render_pass, NULL);
}
