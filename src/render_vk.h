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

#ifndef RENDER_VK_H
#define RENDER_VK_H

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_platform.h>

#ifdef PLATFORM_LINUX
#include <xcb/xcb.h>
#include <vulkan/vulkan_xcb.h>
#endif

#define vkfunc(F) PFN_##F F
#define MB_TO_BYTES(n) (n * 1024 * 1024)

/* Instance */
vkfunc(vkGetInstanceProcAddr);
vkfunc(vkCreateInstance);
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

enum {
  RENDER_ZONE_SIZE = MB_TO_BYTES(256)
};

struct render_instance {
  struct window *window;
  void *vk_handle;
  VkInstance instance;
  VkSurfaceKHR surface;
  size_t n_pdevices;
  VkPhysicalDevice *pdevices;
};

struct render_memory {
  struct render_device *device;
  size_t size;
  size_t offset;
  VkBuffer buffer;
  VkDeviceMemory memory;
};

struct render_buffer {
  struct render_memory *memory;
  size_t offset;
  size_t size;
  VkBuffer buffer;
};

struct render_device {
  struct render_instance *instance;
  uint32_t device_id;
  uint32_t graphics_index;
  uint32_t present_index;
  VkDevice device;
  VkSurfaceFormatKHR surface_format;
  VkQueue graphics_queue;
  VkQueue present_queue;
  VkExtent2D swap_extent;
  VkSwapchainKHR swapchain;
  uint32_t n_swapchain_images;
  VkImage *swapchain_images;
  VkSemaphore image_semaphore;
  VkSemaphore render_semaphore;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory_properties;
  struct render_memory memory;

  /* Device functions */
  vkfunc(vkGetDeviceQueue);
  vkfunc(vkCreateSemaphore);
  vkfunc(vkDestroySemaphore);
  vkfunc(vkCreatePipelineLayout);
  vkfunc(vkDestroyPipelineLayout);
  vkfunc(vkCreateShaderModule);
  vkfunc(vkDestroyShaderModule);
  vkfunc(vkCreateRenderPass);
  vkfunc(vkDestroyRenderPass);
  vkfunc(vkCreateGraphicsPipelines);
  vkfunc(vkDestroyPipeline);
  vkfunc(vkCreateFramebuffer);
  vkfunc(vkDestroyFramebuffer);
  vkfunc(vkCreateImageView);
  vkfunc(vkDestroyImageView);
  vkfunc(vkCreateCommandPool);
  vkfunc(vkDestroyCommandPool);
  vkfunc(vkAllocateCommandBuffers);
  vkfunc(vkFreeCommandBuffers);
  vkfunc(vkBeginCommandBuffer);
  vkfunc(vkEndCommandBuffer);
  vkfunc(vkCmdBeginRenderPass);
  vkfunc(vkCmdEndRenderPass);
  vkfunc(vkCmdBindPipeline);
  vkfunc(vkCmdBindVertexBuffers);
  vkfunc(vkCmdBindIndexBuffer);
  vkfunc(vkCmdDrawIndexed);
  /* Descriptors */
  vkfunc(vkCreateDescriptorPool);
  vkfunc(vkDestroyDescriptorPool);
  vkfunc(vkCreateDescriptorSetLayout);
  vkfunc(vkDestroyDescriptorSetLayout);
  vkfunc(vkAllocateDescriptorSets);
  vkfunc(vkFreeDescriptorSets);
  vkfunc(vkUpdateDescriptorSets);
  vkfunc(vkCmdBindDescriptorSets);
  /* Memory */
  vkfunc(vkCreateBuffer);
  vkfunc(vkDestroyBuffer);
  vkfunc(vkGetBufferMemoryRequirements);
  vkfunc(vkAllocateMemory);
  vkfunc(vkFreeMemory);
  vkfunc(vkBindBufferMemory);
  vkfunc(vkMapMemory);
  vkfunc(vkFlushMappedMemoryRanges);
  vkfunc(vkInvalidateMappedMemoryRanges);
  vkfunc(vkUnmapMemory);
  /* Present */
  vkfunc(vkAcquireNextImageKHR);
  vkfunc(vkQueueSubmit);
  vkfunc(vkQueuePresentKHR);
  vkfunc(vkQueueWaitIdle);
};

struct render_pass {
  struct render_device *device;
  size_t n_desc_layouts;
  VkDescriptorPool desc_pool;
  VkDescriptorSetLayout *desc_layouts;
  VkDescriptorSet *desc_sets;
  VkRenderPass render_pass;
  VkPipeline pipeline;
  VkPipelineLayout pipeline_layout;
  VkImageView *image_views;
  VkFramebuffer *framebuffers;
  VkCommandPool command_pool;
  VkCommandBuffer *command_buffers;
  struct render_memory uniform_memory;
  struct render_buffer vertices;
  struct render_buffer indices;
  struct render_buffer *uniforms;
};

struct render_shader {
  VkShaderModule vert_module;
  VkShaderModule frag_module;
};

#undef vkfunc

/* **************************************** */
/* render_vk_device.c */
int render_device_recreate_swapchain(struct render_device *rd);
/* **************************************** */

/* **************************************** */
/* render_vk_memory.c */
int render_memory_init(
  struct render_memory *rm,
  struct render_device *rd,
  VkBufferUsageFlags usage,
  size_t size
);
void render_memory_deinit(struct render_memory *rm);
void render_memory_reset(struct render_memory *memory);
int render_memory_create_buffer(
  struct render_memory *rm,
  size_t align,
  VkBufferUsageFlags usage,
  size_t size,
  struct render_buffer *out_buffer
);
void render_buffer_destroy(struct render_buffer *rb);
int render_buffer_write(
  struct render_buffer *rb,
  size_t size,
  void *data
);
/* **************************************** */

#endif
