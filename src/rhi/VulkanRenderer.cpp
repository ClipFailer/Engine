/**
 * @file VulkanRenderer.cpp
 * @brief Implementation of Vulkan forward rendering, offscreen pass, and mesh drawing.
 */

#include "VulkanRenderer.hpp"
#include "scene/StaticMeshComponent.hpp"
#include "core/Log.hpp"
#include <backends/imgui_impl_vulkan.h>
#include <fstream>
#include <array>

namespace Engine {

    static std::vector<char> ReadBinaryFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open shader file: {}", filename);
            return {};
        }
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        return buffer;
    }

    VulkanRenderer::VulkanRenderer(VulkanContext& context) : m_Context(context) {}

    VulkanRenderer::~VulkanRenderer() {
        Shutdown();
    }

    bool VulkanRenderer::Initialize() {
        if (!CreateDescriptorSetLayout()) return false;
        if (!CreateUniformBuffers()) return false;
        if (!CreateDescriptorPool()) return false;
        if (!CreateDescriptorSets()) return false;
        if (!CreateOffscreenResources()) return false;
        if (!CreatePipeline()) return false;

        UploadPrimitiveMeshes();

        LOG_INFO("VulkanRenderer initialized successfully");
        return true;
    }

    void VulkanRenderer::Shutdown() {
        VkDevice device = m_Context.GetDevice();
        if (device == VK_NULL_HANDLE) return;

        vkDeviceWaitIdle(device);

        m_PrimitiveMeshes.clear();
        m_CameraUBO.reset();

        if (m_Pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(device, m_Pipeline, nullptr);
            m_Pipeline = VK_NULL_HANDLE;
        }
        if (m_PipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
            m_PipelineLayout = VK_NULL_HANDLE;
        }
        if (m_DescriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
            m_DescriptorPool = VK_NULL_HANDLE;
        }
        if (m_DescriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
            m_DescriptorSetLayout = VK_NULL_HANDLE;
        }

        CleanupOffscreenResources();
    }

    void VulkanRenderer::ResizeViewport(uint32 width, uint32 height) {
        if (width == 0 || height == 0 || (width == m_ViewportWidth && height == m_ViewportHeight)) {
            return;
        }

        m_ViewportWidth = width;
        m_ViewportHeight = height;

        vkDeviceWaitIdle(m_Context.GetDevice());
        CleanupOffscreenResources();
        CreateOffscreenResources();
    }

    bool VulkanRenderer::CreateOffscreenResources() {
        VkDevice device = m_Context.GetDevice();

        // 1. Render Pass
        if (m_OffscreenRenderPass == VK_NULL_HANDLE) {
            VkAttachmentDescription attachments[2]{};

            // Color attachment
            attachments[0].format = VK_FORMAT_R8G8B8A8_UNORM;
            attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            // Depth attachment
            attachments[1].format = VK_FORMAT_D32_SFLOAT;
            attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference colorRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
            VkAttachmentReference depthRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorRef;
            subpass.pDepthStencilAttachment = &depthRef;

            VkSubpassDependency dependencies[2]{};
            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            VkRenderPassCreateInfo rpInfo{};
            rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            rpInfo.attachmentCount = 2;
            rpInfo.pAttachments = attachments;
            rpInfo.subpassCount = 1;
            rpInfo.pSubpasses = &subpass;
            rpInfo.dependencyCount = 2;
            rpInfo.pDependencies = dependencies;

            if (vkCreateRenderPass(device, &rpInfo, nullptr, &m_OffscreenRenderPass) != VK_SUCCESS) {
                return false;
            }
        }

        // 2. Color Image & View
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_ViewportWidth;
        imageInfo.extent.height = m_ViewportHeight;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vkCreateImage(device, &imageInfo, nullptr, &m_ColorImage);

        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(device, m_ColorImage, &memReqs);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReqs.size;
        allocInfo.memoryTypeIndex = m_Context.FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vkAllocateMemory(device, &allocInfo, nullptr, &m_ColorMemory);
        vkBindImageMemory(device, m_ColorImage, m_ColorMemory, 0);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_ColorImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(device, &viewInfo, nullptr, &m_ColorView);

        // Sampler
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        vkCreateSampler(device, &samplerInfo, nullptr, &m_ColorSampler);

        // 3. Depth Image & View
        imageInfo.format = VK_FORMAT_D32_SFLOAT;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        vkCreateImage(device, &imageInfo, nullptr, &m_DepthImage);

        vkGetImageMemoryRequirements(device, m_DepthImage, &memReqs);
        allocInfo.allocationSize = memReqs.size;
        allocInfo.memoryTypeIndex = m_Context.FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        vkAllocateMemory(device, &allocInfo, nullptr, &m_DepthMemory);
        vkBindImageMemory(device, m_DepthImage, m_DepthMemory, 0);

        viewInfo.image = m_DepthImage;
        viewInfo.format = VK_FORMAT_D32_SFLOAT;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        vkCreateImageView(device, &viewInfo, nullptr, &m_DepthView);

        // 4. Framebuffer
        VkImageView attachments[2] = {m_ColorView, m_DepthView};
        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = m_OffscreenRenderPass;
        fbInfo.attachmentCount = 2;
        fbInfo.pAttachments = attachments;
        fbInfo.width = m_ViewportWidth;
        fbInfo.height = m_ViewportHeight;
        fbInfo.layers = 1;

        vkCreateFramebuffer(device, &fbInfo, nullptr, &m_OffscreenFramebuffer);

        // Register texture for ImGui
        m_ViewportTextureId = ImGui_ImplVulkan_AddTexture(m_ColorSampler, m_ColorView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        return true;
    }

    void VulkanRenderer::CleanupOffscreenResources() {
        VkDevice device = m_Context.GetDevice();
        if (device == VK_NULL_HANDLE) return;

        if (m_ViewportTextureId != VK_NULL_HANDLE) {
            ImGui_ImplVulkan_RemoveTexture(m_ViewportTextureId);
            m_ViewportTextureId = VK_NULL_HANDLE;
        }

        if (m_OffscreenFramebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device, m_OffscreenFramebuffer, nullptr);
            m_OffscreenFramebuffer = VK_NULL_HANDLE;
        }
        if (m_ColorSampler != VK_NULL_HANDLE) {
            vkDestroySampler(device, m_ColorSampler, nullptr);
            m_ColorSampler = VK_NULL_HANDLE;
        }
        if (m_ColorView != VK_NULL_HANDLE) {
            vkDestroyImageView(device, m_ColorView, nullptr);
            m_ColorView = VK_NULL_HANDLE;
        }
        if (m_ColorImage != VK_NULL_HANDLE) {
            vkDestroyImage(device, m_ColorImage, nullptr);
            m_ColorImage = VK_NULL_HANDLE;
        }
        if (m_ColorMemory != VK_NULL_HANDLE) {
            vkFreeMemory(device, m_ColorMemory, nullptr);
            m_ColorMemory = VK_NULL_HANDLE;
        }
        if (m_DepthView != VK_NULL_HANDLE) {
            vkDestroyImageView(device, m_DepthView, nullptr);
            m_DepthView = VK_NULL_HANDLE;
        }
        if (m_DepthImage != VK_NULL_HANDLE) {
            vkDestroyImage(device, m_DepthImage, nullptr);
            m_DepthImage = VK_NULL_HANDLE;
        }
        if (m_DepthMemory != VK_NULL_HANDLE) {
            vkFreeMemory(device, m_DepthMemory, nullptr);
            m_DepthMemory = VK_NULL_HANDLE;
        }
    }

    bool VulkanRenderer::CreateDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        return vkCreateDescriptorSetLayout(m_Context.GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) == VK_SUCCESS;
    }

    bool VulkanRenderer::CreateUniformBuffers() {
        m_CameraUBO = std::make_unique<VulkanBuffer>(
            m_Context,
            sizeof(CameraUBO),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
        return true;
    }

    bool VulkanRenderer::CreateDescriptorPool() {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = 10;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 10;

        return vkCreateDescriptorPool(m_Context.GetDevice(), &poolInfo, nullptr, &m_DescriptorPool) == VK_SUCCESS;
    }

    bool VulkanRenderer::CreateDescriptorSets() {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_DescriptorSetLayout;

        if (vkAllocateDescriptorSets(m_Context.GetDevice(), &allocInfo, &m_DescriptorSet) != VK_SUCCESS) {
            return false;
        }

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_CameraUBO->GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(CameraUBO);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(m_Context.GetDevice(), 1, &descriptorWrite, 0, nullptr);
        return true;
    }

    VkShaderModule VulkanRenderer::CreateShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(m_Context.GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            return VK_NULL_HANDLE;
        }
        return shaderModule;
    }

    bool VulkanRenderer::CreatePipeline() {
        auto vertShaderCode = ReadBinaryFile("build/bin/shaders/mesh.vert.spv");
        auto fragShaderCode = ReadBinaryFile("build/bin/shaders/mesh.frag.spv");

        if (vertShaderCode.empty() || fragShaderCode.empty()) {
            // Check fallback path
            vertShaderCode = ReadBinaryFile("shaders/mesh.vert.spv");
            fragShaderCode = ReadBinaryFile("shaders/mesh.frag.spv");
        }

        if (vertShaderCode.empty() || fragShaderCode.empty()) {
            LOG_ERROR("Failed to load compiled SPV shaders for Vulkan pipeline");
            return false;
        }

        VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo shaderStages[2]{};
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertShaderModule;
        shaderStages[0].pName = "main";

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].pName = "main";

        // Vertex Input
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, Position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, Normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, TexCoord);

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // Push constants (Model matrix + Color + PBR params)
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(MeshPushConstants);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_Context.GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            return false;
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.renderPass = m_OffscreenRenderPass;
        pipelineInfo.subpass = 0;

        VkResult res = vkCreateGraphicsPipelines(m_Context.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline);

        vkDestroyShaderModule(m_Context.GetDevice(), fragShaderModule, nullptr);
        vkDestroyShaderModule(m_Context.GetDevice(), vertShaderModule, nullptr);

        return res == VK_SUCCESS;
    }

    void VulkanRenderer::UploadPrimitiveMeshes() {
        auto uploadGeometry = [this](PrimitiveType type, const MeshGeometry& geo) {
            GPUMesh mesh;
            mesh.IndexCount = static_cast<uint32>(geo.Indices.size());

            // Vertex Buffer
            VkDeviceSize vSize = sizeof(Vertex) * geo.Vertices.size();
            mesh.VertexBuffer = std::make_unique<VulkanBuffer>(
                m_Context,
                vSize,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
            mesh.VertexBuffer->UploadData(geo.Vertices.data(), vSize);

            // Index Buffer
            VkDeviceSize iSize = sizeof(uint32) * geo.Indices.size();
            mesh.IndexBuffer = std::make_unique<VulkanBuffer>(
                m_Context,
                iSize,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
            mesh.IndexBuffer->UploadData(geo.Indices.data(), iSize);

            m_PrimitiveMeshes[type] = std::move(mesh);
        };

        uploadGeometry(PrimitiveType::Cube, Primitives::CreateCube(1.0f));
        uploadGeometry(PrimitiveType::Sphere, Primitives::CreateSphere(0.5f, 32, 32));
        uploadGeometry(PrimitiveType::Plane, Primitives::CreatePlane(20.0f));
    }

    void VulkanRenderer::RenderScene(const Scene& scene, const Camera& camera) {
        if (m_OffscreenFramebuffer == VK_NULL_HANDLE || m_Pipeline == VK_NULL_HANDLE) {
            return;
        }

        // 1. Update Camera UBO
        CameraUBO ubo;
        ubo.View = camera.GetViewMatrix();
        ubo.Proj = camera.GetProjectionMatrix();
        ubo.CameraPos = camera.GetPosition();
        m_CameraUBO->UploadData(&ubo, sizeof(CameraUBO));

        // 2. Record Render Command Buffer
        VkCommandBuffer cmd = m_Context.BeginSingleTimeCommands();

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_OffscreenRenderPass;
        renderPassInfo.framebuffer = m_OffscreenFramebuffer;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = {m_ViewportWidth, m_ViewportHeight};

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.11f, 0.12f, 0.14f, 1.0f}}; // UE5 Slate dark viewport background
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_ViewportWidth);
        viewport.height = static_cast<float>(m_ViewportHeight);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {m_ViewportWidth, m_ViewportHeight};
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);

        // Render each mesh actor in scene
        for (const auto& actor : scene.GetActors()) {
            if (!actor->bIsVisible) continue;

            auto meshComp = actor->GetComponent<StaticMeshComponent>();
            if (!meshComp) continue;

            auto it = m_PrimitiveMeshes.find(meshComp->MeshType);
            if (it == m_PrimitiveMeshes.end()) continue;

            const GPUMesh& gpuMesh = it->second;

            MeshPushConstants pc;
            pc.Model = actor->Transform.GetMatrix();
            pc.BaseColor = meshComp->BaseColor;
            pc.Metallic = meshComp->Metallic;
            pc.Roughness = meshComp->Roughness;

            vkCmdPushConstants(cmd, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &pc);

            VkBuffer vBuffers[] = {gpuMesh.VertexBuffer->GetBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(cmd, 0, 1, vBuffers, offsets);
            vkCmdBindIndexBuffer(cmd, gpuMesh.IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(cmd, gpuMesh.IndexCount, 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(cmd);
        m_Context.EndSingleTimeCommands(cmd);
    }

} // namespace Engine
