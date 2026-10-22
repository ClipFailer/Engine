/**
 * @file VulkanBuffer.hpp
 * @brief Encapsulation of Vulkan GPU/staging buffers with memory mapping.
 */

#pragma once

#include "VulkanContext.hpp"

namespace Engine {

    /**
     * @class VulkanBuffer
     * @brief Manages VkBuffer allocation, device memory binding, and CPU/GPU memory transfers.
     */
    class VulkanBuffer {
    public:
        VulkanBuffer(VulkanContext& context,
                     VkDeviceSize size,
                     VkBufferUsageFlags usage,
                     VkMemoryPropertyFlags properties);
        ~VulkanBuffer();

        VulkanBuffer(const VulkanBuffer&) = delete;
        VulkanBuffer& operator=(const VulkanBuffer&) = delete;

        /**
         * @brief Maps buffer memory into host virtual address space.
         * @param size Size to map (or VK_WHOLE_SIZE).
         * @param offset Offset into memory.
         * @return Host memory pointer.
         */
        void* Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        /**
         * @brief Unmaps mapped memory.
         */
        void Unmap();

        /**
         * @brief Uploads data directly into mapped buffer memory.
         * @param data Pointer to host data.
         * @param size Byte size of data.
         */
        void UploadData(const void* data, VkDeviceSize size);

        [[nodiscard]] VkBuffer GetBuffer() const noexcept { return m_Buffer; }
        [[nodiscard]] VkDeviceMemory GetMemory() const noexcept { return m_Memory; }
        [[nodiscard]] VkDeviceSize GetSize() const noexcept { return m_Size; }

    private:
        VulkanContext& m_Context;
        VkBuffer m_Buffer{VK_NULL_HANDLE};
        VkDeviceMemory m_Memory{VK_NULL_HANDLE};
        VkDeviceSize m_Size{0};
        void* m_Mapped{nullptr};
    };

} // namespace Engine
