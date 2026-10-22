/**
 * @file VulkanSwapChain.cpp
 * @brief Implementation of VulkanSwapChain creation, resizing, and presentation.
 */

#include "VulkanSwapChain.hpp"
#include "core/Log.hpp"
#include <algorithm>

namespace Engine {

    VulkanSwapChain::VulkanSwapChain(VulkanContext& context, GLFWwindow* window)
        : m_Context(context), m_Window(window) {}

    VulkanSwapChain::~VulkanSwapChain() {
        Shutdown();
    }

    bool VulkanSwapChain::Initialize() {
        if (!CreateSwapChain()) return false;
        if (!CreateImageViews()) return false;
        if (!CreateRenderPass()) return false;
        if (!CreateDepthResources()) return false;
        if (!CreateFramebuffers()) return false;

        LOG_INFO("VulkanSwapChain initialized ({}x{})", m_Extent.width, m_Extent.height);
        return true;
    }

    void VulkanSwapChain::Shutdown() {
        VkDevice device = m_Context.GetDevice();
        if (device == VK_NULL_HANDLE) return;

        vkDeviceWaitIdle(device);

        for (auto fb : m_Framebuffers) {
            vkDestroyFramebuffer(device, fb, nullptr);
        }
        m_Framebuffers.clear();

        if (m_DepthImageView != VK_NULL_HANDLE) {
            vkDestroyImageView(device, m_DepthImageView, nullptr);
            m_DepthImageView = VK_NULL_HANDLE;
        }
        if (m_DepthImage != VK_NULL_HANDLE) {
            vkDestroyImage(device, m_DepthImage, nullptr);
            m_DepthImage = VK_NULL_HANDLE;
        }
        if (m_DepthImageMemory != VK_NULL_HANDLE) {
            vkFreeMemory(device, m_DepthImageMemory, nullptr);
            m_DepthImageMemory = VK_NULL_HANDLE;
        }

        if (m_RenderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device, m_RenderPass, nullptr);
            m_RenderPass = VK_NULL_HANDLE;
        }

        for (auto iv : m_ImageViews) {
            vkDestroyImageView(device, iv, nullptr);
        }
        m_ImageViews.clear();

        if (m_SwapChain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device, m_SwapChain, nullptr);
            m_SwapChain = VK_NULL_HANDLE;
        }
    }

    void VulkanSwapChain::Recreate() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_Window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(m_Window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_Context.GetDevice());
        Shutdown();
        Initialize();
    }

    bool VulkanSwapChain::CreateSwapChain() {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Context.GetPhysicalDevice(), m_Context.GetSurface(), &capabilities);

        int width, height;
        glfwGetFramebufferSize(m_Window, &width, &height);
        m_Extent.width = std::clamp(static_cast<uint32>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        m_Extent.height = std::clamp(static_cast<uint32>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        uint32 imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Context.GetSurface();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
        createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        createInfo.imageExtent = m_Extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = m_Context.GetQueueFamilies();
        uint32 queueFamilyIndices[] = {indices.GraphicsFamily.value(), indices.PresentFamily.value()};

        if (indices.GraphicsFamily != indices.PresentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // VSync enabled
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(m_Context.GetDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) {
            return false;
        }

        m_ImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
        vkGetSwapchainImagesKHR(m_Context.GetDevice(), m_SwapChain, &imageCount, nullptr);
        m_Images.resize(imageCount);
        vkGetSwapchainImagesKHR(m_Context.GetDevice(), m_SwapChain, &imageCount, m_Images.data());

        return true;
    }

    bool VulkanSwapChain::CreateImageViews() {
        m_ImageViews.resize(m_Images.size());

        for (size_t i = 0; i < m_Images.size(); ++i) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_Images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_ImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_Context.GetDevice(), &createInfo, nullptr, &m_ImageViews[i]) != VK_SUCCESS) {
                return false;
            }
        }
        return true;
    }

    bool VulkanSwapChain::CreateRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_ImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        return vkCreateRenderPass(m_Context.GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) == VK_SUCCESS;
    }

    bool VulkanSwapChain::CreateDepthResources() {
        return true; // Main presentation window swapchain doesn't need depth; offscreen viewport has its own dedicated 32-bit depth target
    }

    bool VulkanSwapChain::CreateFramebuffers() {
        m_Framebuffers.resize(m_ImageViews.size());

        for (size_t i = 0; i < m_ImageViews.size(); ++i) {
            VkImageView attachments[] = {m_ImageViews[i]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_RenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = m_Extent.width;
            framebufferInfo.height = m_Extent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_Context.GetDevice(), &framebufferInfo, nullptr, &m_Framebuffers[i]) != VK_SUCCESS) {
                return false;
            }
        }
        return true;
    }

    VkResult VulkanSwapChain::AcquireNextImage(uint32* imageIndex, VkSemaphore semaphore) {
        return vkAcquireNextImageKHR(m_Context.GetDevice(), m_SwapChain, UINT64_MAX, semaphore, VK_NULL_HANDLE, imageIndex);
    }

    VkResult VulkanSwapChain::Present(uint32 imageIndex, VkSemaphore waitSemaphore) {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &waitSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_SwapChain;
        presentInfo.pImageIndices = &imageIndex;

        return vkQueuePresentKHR(m_Context.GetPresentQueue(), &presentInfo);
    }

} // namespace Engine
