/**
 * @file StaticMeshComponent.hpp
 * @brief Component representing 3D geometry and material visual parameters.
 */

#pragma once

#include "Component.hpp"
#include "Primitives.hpp"

namespace Engine {

    /**
     * @brief Built-in procedural primitive types.
     */
    enum class PrimitiveType : uint8 {
        Cube = 0,
        Sphere = 1,
        Plane = 2,
        Custom = 3
    };

    /**
     * @class StaticMeshComponent
     * @brief Visual mesh rendering component holding geometry and material properties.
     */
    class StaticMeshComponent : public Component {
        GENERATED_BODY()
    public:
        StaticMeshComponent();
        explicit StaticMeshComponent(PrimitiveType type);

        PrimitiveType MeshType{PrimitiveType::Cube};
        glm::vec4 BaseColor{0.8f, 0.8f, 0.82f, 1.0f}; ///< RGBA material tint.
        float Metallic{0.1f};                         ///< Surface metallic factor [0..1].
        float Roughness{0.5f};                        ///< Surface roughness factor [0..1].
        bool bCastShadows{true};                      ///< Shadow casting flag.
        std::string ModelPath{};                      ///< File path to imported 3D model (.obj)
        std::string TexturePath{};                    ///< File path to diffuse/albedo texture image (.png, .jpg, .bmp)

        MeshGeometry CustomGeometry;                  ///< Uploaded or imported vertex/index geometry

        /**
         * @brief Loads an external 3D model file into this component.
         * @param filepath Path to 3D model on disk.
         * @return True if model was imported successfully.
         */
        bool LoadModel(const std::string& filepath);

        /**
         * @brief Assigns an albedo texture image file to this mesh component.
         * @param filepath Path to texture on disk.
         * @return True if texture file exists and assigned.
         */
        bool LoadTexture(const std::string& filepath);

        /**
         * @brief Returns the procedural or custom geometry for the selected primitive type.
         * @return MeshGeometry with vertices and indices.
         */
        [[nodiscard]] MeshGeometry GetGeometry() const;
    };

} // namespace Engine
