/**
 * @file CollisionComponent.cpp
 * @brief Implementation and reflection metadata for CollisionComponent.
 */

#include "CollisionComponent.hpp"
#include "StaticMeshComponent.hpp"
#include "core/Log.hpp"
#include <algorithm>

namespace Engine {

    CollisionComponent::CollisionComponent()
        : Component("CollisionComponent") {}

    void CollisionComponent::FitToMesh(const StaticMeshComponent& mesh) {
        MeshGeometry geo = mesh.GetGeometry();
        if (geo.Vertices.empty()) return;

        glm::vec3 minBound(1e9f);
        glm::vec3 maxBound(-1e9f);

        for (const auto& v : geo.Vertices) {
            minBound = glm::min(minBound, v.Position);
            maxBound = glm::max(maxBound, v.Position);
        }

        BoxHalfExtents = (maxBound - minBound) * 0.5f;
        SphereRadius = std::max({BoxHalfExtents.x, BoxHalfExtents.y, BoxHalfExtents.z});
        Offset = (maxBound + minBound) * 0.5f;

        LOG_INFO("Fitted collision bounds to mesh (HalfExtents: [{:.2f}, {:.2f}, {:.2f}], Radius: {:.2f})",
                 BoxHalfExtents.x, BoxHalfExtents.y, BoxHalfExtents.z, SphereRadius);
    }

    ENGINE_CLASS_BEGIN(CollisionComponent, Component)
        ENGINE_PROPERTY(CollisionComponent, BoxHalfExtents, Vec3, "Collision", "Half dimensions for box collider")
        ENGINE_PROPERTY(CollisionComponent, SphereRadius, Float, "Collision", "Radius for sphere collider")
        ENGINE_PROPERTY(CollisionComponent, Restitution, Float, "Collision", "Bounciness factor [0..1]")
        ENGINE_PROPERTY(CollisionComponent, Friction, Float, "Collision", "Friction factor")
        ENGINE_PROPERTY(CollisionComponent, bIsTrigger, Bool, "Collision", "Is collision volume a trigger")
    ENGINE_CLASS_END()

} // namespace Engine
