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

static get_heap_index(
  struct render_pipeline *rp,
  uint32_t memory_type_bit,
  VkMemoryPropertyFlags flags
) {
  int i;
  VkPhysicalDeviceMemoryProperties props;

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
/* Internal */
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
  result = vkCreateBuffer(rp->device, &create_info, NULL, out_buf);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_BUFFER;
  return RENDER_ERROR_NONE;
}

int alloc_buffer(
  struct render_pipeline *rp,
  VkBuffer buf,
  VkDeviceMemory *out_mem
) {
  int index;
  VkMemoryRequirements reqs;
  VkMemoryAllocateInfo alloc_info = { 0 };
  VkResult result;

  vkGetBufferMemoryRequirements(rp->device, buf, &reqs);
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = reqs.size;
  index = get_heap_index(
    rp,
    reqs.memoryTypeBits,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
  );
  if (index < 0) return RENDER_ERROR_VULKAN_MEMORY;
  alloc_info.memoryTypeIndex = (uint32_t) index;
  result = vkAllocateMemory(rp->device, &alloc_info, NULL, out_mem);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_MEMORY;
  result = vkBindBufferMemory(rp->device, buf, *out_mem, 0);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_MEMORY;
  return RENDER_ERROR_NONE;
}

int write_data(
  struct render_pipeline *rp,
  VkDeviceMemory mem,
  void *data,
  size_t size
) {
  void *dst;
  VkMappedMemoryRange range = { 0 };
  VkResult result;

  range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  range.memory = mem;
  range.offset = 0;
  range.size = size;
  result = vkMapMemory(
    rp->device,
    range.memory,
    range.offset,
    range.size,
    0,
    &dst
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_MEMORY_MAP;
  memcpy(dst, data, range.size);
  result = vkFlushMappedMemoryRanges(rp->device, 1, &range);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_MEMORY_MAP;
  result = vkInvalidateMappedMemoryRanges(rp->device, 1, &range);
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_MEMORY_MAP;
  vkUnmapMemory(rp->device, mem);
  return RENDER_ERROR_NONE;
}

