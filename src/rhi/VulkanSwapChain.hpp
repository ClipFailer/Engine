/**
 * @file VulkanSwapChain.hpp
 * @brief Window presentation swapchain, depth buffer, render pass, and framebuffers.
 */

#pragma once

#include "VulkanContext.hpp"
#include <vector>

namespace Engine {

    /**
     * @class VulkanSwapChain
     * @brief Manages presentation images, depth buffer, render passes, and synchronization fences.
     */
    class VulkanSwapChain {
    public:
        VulkanSwapChain(VulkanContext& context, GLFWwindow* window);
        ~VulkanSwapChain();

        VulkanSwapChain(const VulkanSwapChain&) = delete;
        VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

        bool Initialize();
        void Shutdown();
        void Recreate();

        [[nodiscard]] VkSwapchainKHR GetSwapChain() const noexcept { return m_SwapChain; }
        [[nodiscard]] VkFormat GetImageFormat() const noexcept { return m_ImageFormat; }
        [[nodiscard]] VkExtent2D GetExtent() const noexcept { return m_Extent; }
        [[nodiscard]] VkRenderPass GetRenderPass() const noexcept { return m_RenderPass; }
        [[nodiscard]] const std::vector<VkFramebuffer>& GetFramebuffers() const noexcept { return m_Framebuffers; }
        [[nodiscard]] uint32 GetImageCount() const noexcept { return static_cast<uint32>(m_Images.size()); }

        /**
         * @brief Acquires next available swapchain image index.
         * @param imageIndex Output index.
         * @param semaphore Signal semaphore.
         * @return VkResult status.
         */
        VkResult AcquireNextImage(uint32* imageIndex, VkSemaphore semaphore);

        /**
         * @brief Presents rendered frame to display.
         * @param imageIndex Image index to present.
         * @param waitSemaphore Semaphore to wait on before presenting.
         * @return VkResult status.
         */
        VkResult Present(uint32 imageIndex, VkSemaphore waitSemaphore);

    private:
        bool CreateSwapChain();
        bool CreateImageViews();
        bool CreateRenderPass();
        bool CreateDepthResources();
        bool CreateFramebuffers();

        VulkanContext& m_Context;
        GLFWwindow* m_Window;

        VkSwapchainKHR m_SwapChain{VK_NULL_HANDLE};
        VkFormat m_ImageFormat{VK_FORMAT_B8G8R8A8_UNORM};
        VkExtent2D m_Extent{1280, 720};

        std::vector<VkImage> m_Images;
        std::vector<VkImageView> m_ImageViews;
        std::vector<VkFramebuffer> m_Framebuffers;
        VkRenderPass m_RenderPass{VK_NULL_HANDLE};

        VkImage m_DepthImage{VK_NULL_HANDLE};
        VkDeviceMemory m_DepthImageMemory{VK_NULL_HANDLE};
        VkImageView m_DepthImageView{VK_NULL_HANDLE};
    };

} // namespace Engine
