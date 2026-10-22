/**
 * @file StaticMeshComponent.cpp
 * @brief Implementation and reflection metadata for StaticMeshComponent.
 */

#include "StaticMeshComponent.hpp"
#include "MeshLoader.hpp"
#include "core/Log.hpp"
#include <filesystem>

namespace Engine {

    StaticMeshComponent::StaticMeshComponent()
        : Component("StaticMeshComponent"), MeshType(PrimitiveType::Cube) {}

    StaticMeshComponent::StaticMeshComponent(PrimitiveType type)
        : Component("StaticMeshComponent"), MeshType(type) {}

    bool StaticMeshComponent::LoadModel(const std::string& filepath) {
        if (!std::filesystem::exists(filepath)) {
            LOG_WARN("StaticMeshComponent: Model file not found '{}'", filepath);
            return false;
        }

        MeshGeometry geo;
        if (MeshLoader::LoadOBJ(filepath, geo)) {
            CustomGeometry = std::move(geo);
            ModelPath = filepath;
            MeshType = PrimitiveType::Custom;
            LOG_INFO("StaticMeshComponent: Loaded custom 3D mesh model from '{}'", filepath);
            return true;
        }
        return false;
    }

    bool StaticMeshComponent::LoadTexture(const std::string& filepath) {
        if (!std::filesystem::exists(filepath)) {
            LOG_WARN("StaticMeshComponent: Texture file not found '{}'", filepath);
            return false;
        }
        TexturePath = filepath;
        LOG_INFO("StaticMeshComponent: Assigned texture '{}'", filepath);
        return true;
    }

    MeshGeometry StaticMeshComponent::GetGeometry() const {
        if (MeshType == PrimitiveType::Custom && !CustomGeometry.Vertices.empty()) {
            return CustomGeometry;
        }

        switch (MeshType) {
            case PrimitiveType::Cube:   return Primitives::CreateCube(1.0f);
            case PrimitiveType::Sphere: return Primitives::CreateSphere(0.5f, 32, 32);
            case PrimitiveType::Plane:  return Primitives::CreatePlane(20.0f);
            case PrimitiveType::Custom:
                if (!CustomGeometry.Vertices.empty()) return CustomGeometry;
                return Primitives::CreateCube(1.0f);
        }
        return Primitives::CreateCube(1.0f);
    }

    ENGINE_CLASS_BEGIN(StaticMeshComponent, Component)
        ENGINE_PROPERTY(StaticMeshComponent, BaseColor, Color4, "Material", "Base albedo color")
        ENGINE_PROPERTY(StaticMeshComponent, Metallic, Float, "Material", "Metallic reflection coefficient")
        ENGINE_PROPERTY(StaticMeshComponent, Roughness, Float, "Material", "Surface roughness coefficient")
        ENGINE_PROPERTY(StaticMeshComponent, bCastShadows, Bool, "Rendering", "Enable dynamic shadows")
        ENGINE_PROPERTY(StaticMeshComponent, TexturePath, String, "Material", "Texture image path (.png, .jpg)")
        ENGINE_PROPERTY(StaticMeshComponent, ModelPath, String, "Geometry", "Imported 3D mesh model path (.obj)")
    ENGINE_CLASS_END()

} // namespace Engine
