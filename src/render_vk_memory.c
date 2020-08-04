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
#include <string.h>

static int get_heap_index(
  struct render_device *rd,
  uint32_t memory_type_bit,
  VkMemoryPropertyFlags flags
) {
  int i;
  VkPhysicalDeviceMemoryProperties props;

  props = rd->memory_properties;
  for (i = 0; i < props.memoryTypeCount; ++i) {
    if (memory_type_bit & (1U << i)) {
      if (props.memoryTypes[i].propertyFlags & flags) return i;
    }
  }
  return -1;
}

/* **************************************** */
/* Public */
/* **************************************** */

int render_memory_init(
  struct render_memory *rm,
  struct render_device *device,
  VkBufferUsageFlags usage,
  size_t size
) {
  int index;
  int err = RENDER_ERROR_MEMORY;
  VkBufferCreateInfo create_info = { 0 };
  VkMemoryAllocateInfo alloc_info = { 0 };
  VkMemoryRequirements reqs = { 0 };
  VkResult result;

  if (!rm) return RENDER_ERROR_NULL;
  if (!device) return RENDER_ERROR_NULL;
  memset(rm, 0, sizeof(struct render_memory));
  rm->device = device;
  rm->size = size;
  create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  create_info.size = size;
  create_info.usage = usage;
  result = device->vkCreateBuffer(
    device->device,
    &create_info,
    NULL,
    &rm->buffer
  );
  if (result != VK_SUCCESS) {
    err = RENDER_ERROR_VULKAN_BUFFER;
    goto err_buffer;
  }
  device->vkGetBufferMemoryRequirements(device->device, rm->buffer, &reqs);
  index = get_heap_index(
    device,
    reqs.memoryTypeBits,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
  );
  if (index < 0) {
    err = RENDER_ERROR_VULKAN_MEMORY;
    goto err_index;
  }
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = reqs.size;
  alloc_info.memoryTypeIndex = (uint32_t) index;
  result = device->vkAllocateMemory(
    device->device,
    &alloc_info,
    NULL,
    &rm->memory
  );
  chkerrf(
    result != VK_SUCCESS, {
      err = RENDER_ERROR_VULKAN_MEMORY;
      goto err_memory;
    }
  );
  return RENDER_ERROR_NONE;

 err_memory:
 err_index:
  device->vkDestroyBuffer(device->device, rm->buffer, NULL);
 err_buffer:
  return err;
}

void render_memory_deinit(struct render_memory *rm) {
  if (!rm) return;
  rm->device->vkDestroyBuffer(rm->device->device, rm->buffer, NULL);
  rm->device->vkFreeMemory(rm->device->device, rm->memory, NULL);
}

void render_memory_reset(struct render_memory *rm) {
  rm->offset = 0;
}

int render_memory_create_buffer(
  struct render_memory *rm,
  size_t align,
  VkBufferUsageFlags usage,
  size_t size,
  struct render_buffer *out_buffer
) {
  size_t next_offset;
  VkBufferCreateInfo create_info = { 0 };
  VkMemoryRequirements reqs = { 0 };
  VkResult result;

  if (!out_buffer) return RENDER_ERROR_NULL;
  /* TODO: Check free memory before trying to create a buffer */
  create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  create_info.size = size;
  create_info.usage = usage;
  result = rm->device->vkCreateBuffer(
    rm->device->device,
    &create_info,
    NULL,
    &out_buffer->buffer
  );
  chkerrg(result != VK_SUCCESS, err_buffer);
  rm->device->vkGetBufferMemoryRequirements(
    rm->device->device,
    out_buffer->buffer,
    &reqs
  );
  align = (align < reqs.alignment) ? reqs.alignment : align;
  next_offset =
    (rm->offset % align)        /* Are we unaligned? */
    ? rm->offset + (align - (rm->offset % align))
    : rm->offset;
  result = rm->device->vkBindBufferMemory(
    rm->device->device,
    out_buffer->buffer,
    rm->memory,
    next_offset
  );
  chkerrg(result != VK_SUCCESS, err_bind);
  rm->offset = next_offset + size;
  out_buffer->offset = next_offset;
  out_buffer->size = size;
  out_buffer->memory = rm;
  return RENDER_ERROR_NONE;

 err_bind:
  rm->device->vkDestroyBuffer(rm->device->device, out_buffer->buffer, NULL);
 err_buffer:
  return RENDER_ERROR_VULKAN_BUFFER;
}

void render_buffer_destroy(struct render_buffer *rb) {
  if (!rb) return;
  rb->memory->device->vkDestroyBuffer(
    rb->memory->device->device,
    rb->buffer,
    NULL
  );
  /* TODO: update parent memory that this buffer has been freed */
}

int render_buffer_write(
  struct render_buffer *rb,
  size_t size,
  void *data
) {
  unsigned char *dst;
  size_t align, begin, len;
  VkMappedMemoryRange range = { 0 };
  VkResult result;

  if (!rb) return RENDER_ERROR_NULL;
  /* Vulkan spec states there are restrictions on memory mapping,
   * so we need to ensure begin and len are aligned properly */
  align = rb->memory->device->properties.limits.nonCoherentAtomSize;
  begin = rb->offset - (rb->offset % align);
  len = size + (align - (size % align));
  range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  range.memory = rb->memory->memory;
  range.offset = begin;
  range.size = len;
  result = rb->memory->device->vkMapMemory(
    rb->memory->device->device,
    range.memory,
    range.offset,
    range.size,
    0,
    (void **) &dst
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_MEMORY_MAP;
  memcpy(dst + (rb->offset - begin), data, size);
  rb->memory->device->vkFlushMappedMemoryRanges(
    rb->memory->device->device,
    1,
    &range
  );
  rb->memory->device->vkInvalidateMappedMemoryRanges(
    rb->memory->device->device,
    1,
    &range
  );
  rb->memory->device->vkUnmapMemory(
    rb->memory->device->device,
    rb->memory->memory
  );
  return RENDER_ERROR_NONE;
}

