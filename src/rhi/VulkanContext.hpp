/**
 * @file VulkanContext.hpp
 * @brief Manages Vulkan instance, physical device, logical device, and graphics/present queues.
 */

#pragma once

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "core/Types.hpp"
#include <vector>
#include <optional>

namespace Engine {

    struct QueueFamilyIndices {
        std::optional<uint32> GraphicsFamily;
        std::optional<uint32> PresentFamily;

        [[nodiscard]] bool IsComplete() const {
            return GraphicsFamily.has_value() && PresentFamily.has_value();
        }
    };

    /**
     * @class VulkanContext
     * @brief Singleton-like context managing GPU device access, instance, and queues.
     */
    class VulkanContext {
    public:
        VulkanContext() = default;
        ~VulkanContext();

        VulkanContext(const VulkanContext&) = delete;
        VulkanContext& operator=(const VulkanContext&) = delete;

        /**
         * @brief Initializes Vulkan instance, physical device, and logical device.
         * @param window GLFW window pointer.
         * @return True if initialized.
         */
        bool Initialize(GLFWwindow* window);

        /**
         * @brief Cleans up Vulkan resources.
         */
        void Shutdown();

        [[nodiscard]] VkInstance GetInstance() const noexcept { return m_Instance; }
        [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const noexcept { return m_PhysicalDevice; }
        [[nodiscard]] VkDevice GetDevice() const noexcept { return m_Device; }
        [[nodiscard]] VkQueue GetGraphicsQueue() const noexcept { return m_GraphicsQueue; }
        [[nodiscard]] VkQueue GetPresentQueue() const noexcept { return m_PresentQueue; }
        [[nodiscard]] VkSurfaceKHR GetSurface() const noexcept { return m_Surface; }
        [[nodiscard]] QueueFamilyIndices GetQueueFamilies() const noexcept { return m_QueueIndices; }
        [[nodiscard]] VkCommandPool GetCommandPool() const noexcept { return m_CommandPool; }

        /**
         * @brief Allocates and begins a one-time command buffer.
         * @return Allocated VkCommandBuffer.
         */
        VkCommandBuffer BeginSingleTimeCommands();

        /**
         * @brief Ends and submits a one-time command buffer, waiting for idle.
         * @param commandBuffer Command buffer to finish.
         */
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

        /**
         * @brief Finds a supported memory type index matching requirements.
         * @param typeFilter Bitmask of suitable memory types.
         * @param properties Required memory property flags.
         * @return Index of matching memory type.
         */
        [[nodiscard]] uint32 FindMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties) const;

    private:
        bool CreateInstance();
        bool SetupDebugMessenger();
        bool CreateSurface(GLFWwindow* window);
        bool PickPhysicalDevice();
        bool CreateLogicalDevice();
        bool CreateCommandPool();

        VkInstance m_Instance{VK_NULL_HANDLE};
        VkDebugUtilsMessengerEXT m_DebugMessenger{VK_NULL_HANDLE};
        VkPhysicalDevice m_PhysicalDevice{VK_NULL_HANDLE};
        VkDevice m_Device{VK_NULL_HANDLE};
        VkSurfaceKHR m_Surface{VK_NULL_HANDLE};

        VkQueue m_GraphicsQueue{VK_NULL_HANDLE};
        VkQueue m_PresentQueue{VK_NULL_HANDLE};
        QueueFamilyIndices m_QueueIndices;
        VkCommandPool m_CommandPool{VK_NULL_HANDLE};

        bool m_bEnableValidationLayers{true};
    };

} // namespace Engine
