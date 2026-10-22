/**
 * @file Camera.hpp
 * @brief Perspective 3D camera supporting Editor orbit, fly controls, and projection matrices.
 */

#pragma once

#include "core/Math.hpp"

namespace Engine {

    /**
     * @class Camera
     * @brief 3D perspective camera for rendering and viewport interaction.
     */
    class Camera {
    public:
        Camera() { UpdateMatrices(); }

        /**
         * @brief Updates camera projection matrix based on aspect ratio and viewport dimensions.
         * @param width Viewport width in pixels.
         * @param height Viewport height in pixels.
         */
        void SetViewportSize(float width, float height) {
            if (width > 0.0f && height > 0.0f) {
                m_AspectRatio = width / height;
                UpdateMatrices();
            }
        }

        [[nodiscard]] const glm::mat4& GetViewMatrix() const noexcept { return m_ViewMatrix; }
        [[nodiscard]] const glm::mat4& GetProjectionMatrix() const noexcept { return m_ProjMatrix; }
        [[nodiscard]] const glm::vec3& GetPosition() const noexcept { return m_Position; }
        [[nodiscard]] const glm::vec3& GetRotation() const noexcept { return m_Rotation; }

        void SetPosition(const glm::vec3& pos) { m_Position = pos; UpdateMatrices(); }
        void SetRotation(const glm::vec3& rot) { m_Rotation = rot; UpdateMatrices(); }

        /**
         * @brief Centers and focuses camera on a target point at a specified distance.
         * @param target World coordinates to focus on.
         * @param distance Distance from target along view direction.
         */
        void FocusOn(const glm::vec3& target, float distance = 3.5f) {
            glm::quat q = glm::quat(glm::radians(m_Rotation));
            glm::vec3 forward = q * glm::vec3(0.0f, 0.0f, -1.0f);
            m_Position = target - forward * (std::max)(1.0f, distance);
            UpdateMatrices();
        }

        [[nodiscard]] float GetFOV() const noexcept { return m_FOV; }
        void SetFOV(float fov) { m_FOV = fov; UpdateMatrices(); }

        [[nodiscard]] float GetNearClip() const noexcept { return m_NearClip; }
        [[nodiscard]] float GetFarClip() const noexcept { return m_FarClip; }

        /**
         * @brief Translates camera in local camera coordinates.
         * @param delta Movement delta (X=Right, Y=Up, Z=Forward).
         */
        void MoveLocal(const glm::vec3& delta) {
            glm::quat q = glm::quat(glm::radians(m_Rotation));
            glm::vec3 forward = q * glm::vec3(0.0f, 0.0f, -1.0f);
            glm::vec3 right   = q * glm::vec3(1.0f, 0.0f, 0.0f);
            glm::vec3 up      = glm::vec3(0.0f, 1.0f, 0.0f);

            m_Position += right * delta.x + up * delta.y + forward * delta.z;
            UpdateMatrices();
        }

        /**
         * @brief Rotates camera by yaw and pitch deltas.
         * @param deltaPitch Degrees change in pitch (X).
         * @param deltaYaw Degrees change in yaw (Y).
         */
        void Rotate(float deltaPitch, float deltaYaw) {
            m_Rotation.x = glm::clamp(m_Rotation.x + deltaPitch, -89.0f, 89.0f);
            m_Rotation.y += deltaYaw;
            if (m_Rotation.y > 360.0f) m_Rotation.y -= 360.0f;
            if (m_Rotation.y < 0.0f)   m_Rotation.y += 360.0f;
            UpdateMatrices();
        }

    private:
        void UpdateMatrices() {
            glm::quat orientation = glm::quat(glm::radians(m_Rotation));
            glm::vec3 forward = orientation * glm::vec3(0.0f, 0.0f, -1.0f);
            m_ViewMatrix = glm::lookAt(m_Position, m_Position + forward, glm::vec3(0.0f, 1.0f, 0.0f));

            m_ProjMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
            // Vulkan Y-clip inversion
            m_ProjMatrix[1][1] *= -1.0f;
        }

        glm::vec3 m_Position{0.0f, 3.0f, 8.0f};
        glm::vec3 m_Rotation{-15.0f, 0.0f, 0.0f}; // Pitch, Yaw, Roll in degrees
        float m_FOV{60.0f};
        float m_AspectRatio{1.7777f};
        float m_NearClip{0.1f};
        float m_FarClip{1000.0f};

        glm::mat4 m_ViewMatrix{1.0f};
        glm::mat4 m_ProjMatrix{1.0f};
    };

} // namespace Engine
