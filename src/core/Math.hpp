/**
 * @file Math.hpp
 * @brief Mathematical primitives, vectors, matrices, and spatial transforms.
 */

#pragma once

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace Engine {

    /**
     * @struct Transform
     * @brief Spatial transform representing translation, rotation (Euler angles in degrees), and scale.
     */
    struct Transform {
        glm::vec3 Position{0.0f, 0.0f, 0.0f};  ///< Translation vector (X, Y, Z).
        glm::vec3 Rotation{0.0f, 0.0f, 0.0f};  ///< Rotation in degrees (Pitch, Yaw, Roll).
        glm::vec3 Scale{1.0f, 1.0f, 1.0f};     ///< Non-uniform scale factors.

        /**
         * @brief Default constructor.
         */
        Transform() = default;

        /**
         * @brief Constructs a transform with explicit position, rotation, and scale.
         * @param pos World or local position.
         * @param rot Rotation in Euler degrees.
         * @param sc Scale vector.
         */
        Transform(const glm::vec3& pos, const glm::vec3& rot = glm::vec3(0.0f), const glm::vec3& sc = glm::vec3(1.0f))
            : Position(pos), Rotation(rot), Scale(sc) {}

        /**
         * @brief Computes the combined 4x4 affine model matrix.
         * @return 4x4 transformation matrix (Translate * Rotate * Scale).
         */
        [[nodiscard]] glm::mat4 GetMatrix() const {
            glm::mat4 rotationMat = glm::mat4(GetOrientation());
            return glm::translate(glm::mat4(1.0f), Position) *
                   rotationMat *
                   glm::scale(glm::mat4(1.0f), Scale);
        }

        /**
         * @brief Computes quaternion orientation from Euler angles in degrees.
         * @return Orientation quaternion.
         */
        [[nodiscard]] glm::quat GetOrientation() const {
            return glm::quat(glm::radians(Rotation));
        }

        /**
         * @brief Computes normalized forward vector facing in the orientation direction.
         * @return Normalized direction vector.
         */
        [[nodiscard]] glm::vec3 GetForward() const {
            return GetOrientation() * glm::vec3(0.0f, 0.0f, -1.0f);
        }

        /**
         * @brief Computes normalized right vector.
         * @return Normalized direction vector.
         */
        [[nodiscard]] glm::vec3 GetRight() const {
            return GetOrientation() * glm::vec3(1.0f, 0.0f, 0.0f);
        }

        /**
         * @brief Computes normalized up vector.
         * @return Normalized direction vector.
         */
        [[nodiscard]] glm::vec3 GetUp() const {
            return GetOrientation() * glm::vec3(0.0f, 1.0f, 0.0f);
        }
    };

} // namespace Engine
