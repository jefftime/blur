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

#include "render_vk_memory.h"
#include <string.h>

static int get_heap_index(
  struct render_pipeline *rp,
  uint32_t memory_type_bit,
  VkMemoryPropertyFlags flags
) {
  int i;
  VkPhysicalDeviceMemoryProperties props;

  vkGetPhysicalDeviceMemoryProperties(
    rp->device->instance->pdevices[rp->device->device_id],
    &props
  );
  for (i = 0; i < props.memoryTypeCount; ++i) {
    if (memory_type_bit & (1U << i)) {
      if (props.memoryTypes[i].propertyFlags & flags) {
        return i;
      }
    }
  }
  return -1;
}

/* **************************************** */
/* Public */
/* **************************************** */

int create_buffer(
  struct render_pipeline *rp,
  VkBuffer *out_buf,
  size_t size,
  VkBufferUsageFlags flags
) {
  VkBufferCreateInfo create_info = { 0 };
  VkResult result;

  create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  create_info.size = size;
  create_info.usage = flags;
  create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  result = rp->device->vkCreateBuffer(
    rp->device->device,
    &create_info,
    NULL,
    out_buf
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_BUFFER;
  return RENDER_ERROR_NONE;
}

int alloc_buffer(
  struct render_pipeline *rp,
  VkBuffer buf,
  VkDeviceMemory *out_mem
) {
  int index, err = RENDER_ERROR_VULKAN_MEMORY;
  VkMemoryRequirements reqs;
  VkMemoryAllocateInfo alloc_info = { 0 };
  VkResult result;

  rp->device->vkGetBufferMemoryRequirements(rp->device->device, buf, &reqs);
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = reqs.size;
  index = get_heap_index(
    rp,
    reqs.memoryTypeBits,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
  );
  if (index < 0) goto err_index;
  alloc_info.memoryTypeIndex = (uint32_t) index;
  result = rp->device->vkAllocateMemory(
    rp->device->device,
    &alloc_info,
    NULL,
    out_mem
  );
  if (result != VK_SUCCESS) goto err_alloc;
  result = rp->device->vkBindBufferMemory(rp->device->device, buf, *out_mem, 0);
  if (result != VK_SUCCESS) goto err_bind;
  return RENDER_ERROR_NONE;

 err_bind:
  rp->device->vkFreeMemory(rp->device->device, *out_mem, NULL);
 err_alloc:
 err_index:
  return err;
}

int write_data(
  struct render_pipeline *rp,
  VkDeviceMemory mem,
  size_t size,
  void *data
) {
  void *dst;
  VkMappedMemoryRange range = { 0 };
  VkResult result;

  range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  range.memory = mem;
  range.offset = 0;
  /* range.size = size; */
  range.size =
    (
      size
      + rp->device->properties.limits.nonCoherentAtomSize
    )
    / rp->device->properties.limits.nonCoherentAtomSize
    * rp->device->properties.limits.nonCoherentAtomSize;
  result = rp->device->vkMapMemory(
    rp->device->device,
    range.memory,
    range.offset,
    range.size,
    0,
    &dst
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_MEMORY_MAP;
  memcpy(dst, data, size);
  rp->device->vkFlushMappedMemoryRanges(rp->device->device, 1, &range);
  rp->device->vkInvalidateMappedMemoryRanges(
    rp->device->device,
    1,
    &range
  );
  rp->device->vkUnmapMemory(rp->device->device, mem);
  return RENDER_ERROR_NONE;
}

