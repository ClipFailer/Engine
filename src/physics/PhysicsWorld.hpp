/**
 * @file PhysicsWorld.hpp
 * @brief 3D physics simulation engine handling gravity, impulses, collisions, and raycasts.
 */

#pragma once

#include "core/Math.hpp"
#include "core/Types.hpp"
#include <vector>

namespace Engine {

    class Scene;
    class Actor;

    /**
     * @struct RaycastHit
     * @brief Result information returned from a spatial raycast test.
     */
    struct RaycastHit {
        bool bHit{false};
        Actor* HitActor{nullptr};
        glm::vec3 Point{0.0f};
        glm::vec3 Normal{0.0f, 1.0f, 0.0f};
        float Distance{0.0f};
    };

    /**
     * @class PhysicsWorld
     * @brief Manages spatial rigid body simulation, gravity, and collision responses.
     */
    class PhysicsWorld {
    public:
        PhysicsWorld();

        /**
         * @brief Advances physics simulation by deltaTime seconds.
         * @param scene Active scene containing actors.
         * @param deltaTime Sub-step delta time.
         */
        void Step(Scene& scene, float deltaTime);

        /**
         * @brief Performs a 3D raycast against scene actors.
         * @param scene Scene to test against.
         * @param rayOrigin Starting point of ray.
         * @param rayDir Normalized ray direction.
         * @param maxDistance Maximum travel distance.
         * @return RaycastHit information.
         */
        [[nodiscard]] RaycastHit Raycast(const Scene& scene,
                                         const glm::vec3& rayOrigin,
                                         const glm::vec3& rayDir,
                                         float maxDistance = 1000.0f) const;

        glm::vec3 Gravity{0.0f, -9.81f, 0.0f}; ///< Global gravity acceleration vector.
    };

} // namespace Engine
