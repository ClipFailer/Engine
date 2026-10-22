/**
 * @file CollisionComponent.hpp
 * @brief Collision volume definition, bounds fitting, and trigger detection.
 */

#pragma once

#include "Component.hpp"
#include "core/Math.hpp"

namespace Engine {

    class StaticMeshComponent;

    enum class CollisionShape : uint8 {
        Box = 0,
        Sphere = 1
    };

    /**
     * @class CollisionComponent
     * @brief Defines spatial collision boundaries for physics resolution and overlap triggers.
     */
    class CollisionComponent : public Component {
        GENERATED_BODY()
    public:
        CollisionComponent();

        CollisionShape Shape{CollisionShape::Box};
        glm::vec3 BoxHalfExtents{0.5f, 0.5f, 0.5f}; ///< Half extents for Box collision.
        float SphereRadius{0.5f};                  ///< Radius for Sphere collision.
        glm::vec3 Offset{0.0f};                    ///< Local center offset from actor position.

        float Restitution{0.6f};                   ///< Bounciness factor [0..1].
        float Friction{0.2f};                      ///< Surface friction factor.
        bool bIsTrigger{false};                    ///< Trigger volume (detects overlaps without physical blocking).

        /**
         * @brief Automatically scales and fits the collision shape to enclose the provided mesh geometry.
         * @param mesh StaticMeshComponent to fit against.
         */
        void FitToMesh(const StaticMeshComponent& mesh);
    };

} // namespace Engine
