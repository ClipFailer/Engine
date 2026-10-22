/**
 * @file StaticMeshComponent.cpp
 * @brief Implementation and reflection metadata for StaticMeshComponent.
 */

#include "StaticMeshComponent.hpp"

namespace Engine {

    StaticMeshComponent::StaticMeshComponent()
        : Component("StaticMeshComponent"), MeshType(PrimitiveType::Cube) {}

    StaticMeshComponent::StaticMeshComponent(PrimitiveType type)
        : Component("StaticMeshComponent"), MeshType(type) {}

    MeshGeometry StaticMeshComponent::GetGeometry() const {
        switch (MeshType) {
            case PrimitiveType::Cube:   return Primitives::CreateCube(1.0f);
            case PrimitiveType::Sphere: return Primitives::CreateSphere(0.5f, 32, 32);
            case PrimitiveType::Plane:  return Primitives::CreatePlane(20.0f);
            case PrimitiveType::Custom: return Primitives::CreateCube(1.0f);
        }
        return Primitives::CreateCube(1.0f);
    }

    ENGINE_CLASS_BEGIN(StaticMeshComponent, Component)
        ENGINE_PROPERTY(StaticMeshComponent, BaseColor, Color4, "Material", "Base albedo color")
        ENGINE_PROPERTY(StaticMeshComponent, Metallic, Float, "Material", "Metallic reflection coefficient")
        ENGINE_PROPERTY(StaticMeshComponent, Roughness, Float, "Material", "Surface roughness coefficient")
        ENGINE_PROPERTY(StaticMeshComponent, bCastShadows, Bool, "Rendering", "Enable dynamic shadows")
    ENGINE_CLASS_END()

} // namespace Engine
