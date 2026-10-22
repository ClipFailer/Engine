/**
 * @file VulkanRenderer.hpp
 * @brief High-performance Vulkan forward renderer with offscreen viewport rendering and ImGui integration.
 */

#pragma once

#include "VulkanContext.hpp"
#include "VulkanBuffer.hpp"
#include "scene/Scene.hpp"
#include "scene/Camera.hpp"
#include "scene/Primitives.hpp"
#include "scene/StaticMeshComponent.hpp"
#include <memory>
#include <unordered_map>

namespace Engine {

    struct PointLightData {
        glm::vec4 PositionAndRadius{0.0f};
        glm::vec4 ColorAndIntensity{1.0f};
    };

    struct SpotLightData {
        glm::vec4 PositionAndRadius{0.0f};     ///< xyz: Position, w: Radius
        glm::vec4 DirectionAndInner{0.0f};     ///< xyz: Forward direction, w: cos(InnerAngle)
        glm::vec4 ColorAndOuter{1.0f};         ///< rgb: LightColor * Intensity, w: cos(OuterAngle)
    };

    /**
     * @struct SceneUBO
     * @brief Uniform buffer layout passed to vertex/fragment shaders with dynamic lights.
     */
    struct SceneUBO {
        glm::mat4 View{1.0f};
        glm::mat4 Proj{1.0f};
        glm::vec4 CameraPos{0.0f};
        glm::vec4 DirLightDirAndEnabled{0.0f};
        glm::vec4 DirLightColorAndIntensity{1.0f};
        PointLightData PointLights[4];
        SpotLightData SpotLights[2];
        glm::ivec4 RenderModes{0, 0, 0, 1};              // x: PointLightCount, y: LightingModel, z: ToneMappingMode, w: AntiAliasingMode
        glm::vec4 RenderParams{1.0f, 2.2f, 0.6f, 0.4f};  // x: Exposure, y: Gamma, z: AOStrength, w: BloomIntensity
        glm::ivec4 ShaderVolume{0, 1, 0, 0};             // x: bActive, y: EffectType, z: SpotLightCount, w: 0
        glm::vec4 ShaderParams{1.0f, 0.85f, 0.25f, 0.0f};// x: Intensity, y: Param1, z: Param2, w: Param3
    };

    /**
     * @struct MeshPushConstants
     * @brief Push constants sent per-mesh draw call.
     */
    struct MeshPushConstants {
        glm::mat4 Model{1.0f};
        glm::vec4 BaseColor{1.0f};
        float Metallic{0.1f};
        float Roughness{0.5f};
        float Padding[2]{0.0f, 0.0f};
    };

    /**
     * @struct GPUMesh
     * @brief Holds vertex/index buffers allocated on the GPU for a primitive geometry.
     */
    struct GPUMesh {
        std::unique_ptr<VulkanBuffer> VertexBuffer;
        std::unique_ptr<VulkanBuffer> IndexBuffer;
        uint32 IndexCount{0};
    };

    /**
     * @class VulkanRenderer
     * @brief Renders 3D scenes into offscreen textures for the Unreal Engine 5 editor viewport.
     */
    class VulkanRenderer {
    public:
        VulkanRenderer(VulkanContext& context);
        ~VulkanRenderer();

        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer& operator=(const VulkanRenderer&) = delete;

        /**
         * @brief Initializes pipelines, descriptor sets, and procedural GPU meshes.
         * @return True if initialized.
         */
        bool Initialize();

        /**
         * @brief Releases renderer resources.
         */
        void Shutdown();

        /**
         * @brief Resizes offscreen viewport texture to match ImGui viewport panel.
         * @param width New width in pixels.
         * @param height New height in pixels.
         */
        void ResizeViewport(uint32 width, uint32 height);

        /**
         * @brief Renders the scene from the camera's perspective into the offscreen target.
         * @param scene Active 3D scene.
         * @param camera Active perspective camera.
         */
        void RenderScene(const Scene& scene, const Camera& camera);

        /**
         * @brief Returns ImGui texture ID for displaying the viewport image.
         * @return ImTextureID (VkDescriptorSet).
         */
        [[nodiscard]] VkDescriptorSet GetViewportTexture() const noexcept { return m_ViewportTextureId; }
        [[nodiscard]] uint32 GetViewportWidth() const noexcept { return m_ViewportWidth; }
        [[nodiscard]] uint32 GetViewportHeight() const noexcept { return m_ViewportHeight; }

    private:
        bool CreateOffscreenResources();
        void CleanupOffscreenResources();
        bool CreateDescriptorSetLayout();
        bool CreatePipeline();
        bool CreateUniformBuffers();
        bool CreateDescriptorPool();
        bool CreateDescriptorSets();
        void UploadPrimitiveMeshes();

        VkShaderModule CreateShaderModule(const std::vector<char>& code);

        VulkanContext& m_Context;

        uint32 m_ViewportWidth{1280};
        uint32 m_ViewportHeight{720};

        VkRenderPass m_OffscreenRenderPass{VK_NULL_HANDLE};
        VkImage m_ColorImage{VK_NULL_HANDLE};
        VkDeviceMemory m_ColorMemory{VK_NULL_HANDLE};
        VkImageView m_ColorView{VK_NULL_HANDLE};
        VkSampler m_ColorSampler{VK_NULL_HANDLE};
        VkFramebuffer m_OffscreenFramebuffer{VK_NULL_HANDLE};

        VkImage m_DepthImage{VK_NULL_HANDLE};
        VkDeviceMemory m_DepthMemory{VK_NULL_HANDLE};
        VkImageView m_DepthView{VK_NULL_HANDLE};

        VkDescriptorSet m_ViewportTextureId{VK_NULL_HANDLE};

        VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};
        VkDescriptorPool m_DescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet m_DescriptorSet{VK_NULL_HANDLE};
        VkPipelineLayout m_PipelineLayout{VK_NULL_HANDLE};
        VkPipeline m_Pipeline{VK_NULL_HANDLE};

        std::unique_ptr<VulkanBuffer> m_SceneUBO;
        std::unordered_map<PrimitiveType, GPUMesh> m_PrimitiveMeshes;
        std::unordered_map<std::string, GPUMesh> m_CustomMeshes;
    };

} // namespace Engine
