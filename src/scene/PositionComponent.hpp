/**
 * @file PositionComponent.hpp
 * @brief 3D spatial transformation component giving an entity coordinates in world space.
 */

#pragma once

#include "Component.hpp"
#include "core/Math.hpp"

namespace Engine {

    /**
     * @class PositionComponent
     * @brief Spatial transform component (Location, Rotation, Scale) for scene positioning.
     */
    class PositionComponent : public Component {
        GENERATED_BODY()
    public:
        PositionComponent();
        explicit PositionComponent(const glm::vec3& position);

        Transform Transform;

        [[nodiscard]] glm::vec3 GetPosition() const noexcept { return Transform.Position; }
        [[nodiscard]] glm::vec3 GetRotation() const noexcept { return Transform.Rotation; }
        [[nodiscard]] glm::vec3 GetScale() const noexcept { return Transform.Scale; }

        void SetPosition(const glm::vec3& pos) noexcept { Transform.Position = pos; }
        void SetRotation(const glm::vec3& rot) noexcept { Transform.Rotation = rot; }
        void SetScale(const glm::vec3& sc) noexcept { Transform.Scale = sc; }

        [[nodiscard]] glm::mat4 GetMatrix() const { return Transform.GetMatrix(); }
        [[nodiscard]] glm::vec3 GetForward() const { return Transform.GetForward(); }
        [[nodiscard]] glm::vec3 GetRight() const { return Transform.GetRight(); }
        [[nodiscard]] glm::vec3 GetUp() const { return Transform.GetUp(); }
    };

} // namespace Engine
