/**
 * @file VulkanBuffer.cpp
 * @brief Implementation of VulkanBuffer memory allocation and updates.
 */

#include "VulkanBuffer.hpp"
#include <cstring>
#include <stdexcept>

namespace Engine {

    VulkanBuffer::VulkanBuffer(VulkanContext& context,
                               VkDeviceSize size,
                               VkBufferUsageFlags usage,
                               VkMemoryPropertyFlags properties)
        : m_Context(context), m_Size(size) {

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_Context.GetDevice(), &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan buffer");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Context.GetDevice(), m_Buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = m_Context.FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_Context.GetDevice(), &allocInfo, nullptr, &m_Memory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate Vulkan buffer memory");
        }

        vkBindBufferMemory(m_Context.GetDevice(), m_Buffer, m_Memory, 0);
    }

    VulkanBuffer::~VulkanBuffer() {
        Unmap();
        if (m_Buffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(m_Context.GetDevice(), m_Buffer, nullptr);
            m_Buffer = VK_NULL_HANDLE;
        }
        if (m_Memory != VK_NULL_HANDLE) {
            vkFreeMemory(m_Context.GetDevice(), m_Memory, nullptr);
            m_Memory = VK_NULL_HANDLE;
        }
    }

    void* VulkanBuffer::Map(VkDeviceSize size, VkDeviceSize offset) {
        if (!m_Mapped) {
            vkMapMemory(m_Context.GetDevice(), m_Memory, offset, size, 0, &m_Mapped);
        }
        return m_Mapped;
    }

    void VulkanBuffer::Unmap() {
        if (m_Mapped) {
            vkUnmapMemory(m_Context.GetDevice(), m_Memory);
            m_Mapped = nullptr;
        }
    }

    void VulkanBuffer::UploadData(const void* data, VkDeviceSize size) {
        void* mapped = Map(size);
        std::memcpy(mapped, data, static_cast<size_t>(size));
        Unmap();
    }

} // namespace Engine
