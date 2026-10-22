/**
 * @file Primitives.hpp
 * @brief Vertex definitions and procedural geometry generators for 3D shapes.
 */

#pragma once

#include "core/Math.hpp"
#include "core/Types.hpp"
#include <vector>

namespace Engine {

    /**
     * @struct Vertex
     * @brief Interleaved vertex layout for 3D mesh rendering.
     */
    struct Vertex {
        glm::vec3 Position{0.0f};
        glm::vec3 Normal{0.0f, 1.0f, 0.0f};
        glm::vec2 TexCoord{0.0f};

        bool operator==(const Vertex& other) const {
            return Position == other.Position &&
                   Normal == other.Normal &&
                   TexCoord == other.TexCoord;
        }
    };

    /**
     * @struct MeshGeometry
     * @brief CPU mesh representation holding vertex and index buffers.
     */
    struct MeshGeometry {
        std::vector<Vertex> Vertices;
        std::vector<uint32> Indices;
    };

    /**
     * @brief Procedural primitive generators for engine geometry.
     */
    class Primitives {
    public:
        /**
         * @brief Generates a cube mesh.
         * @param size Side length.
         * @return Generated mesh geometry.
         */
        [[nodiscard]] static MeshGeometry CreateCube(float size = 1.0f);

        /**
         * @brief Generates a UV sphere mesh with smooth normals.
         * @param radius Radius of sphere.
         * @param rings Number of latitudinal segments.
         * @param sectors Number of longitudinal segments.
         * @return Generated mesh geometry.
         */
        [[nodiscard]] static MeshGeometry CreateSphere(float radius = 0.5f, uint32 rings = 32, uint32 sectors = 32);

        /**
         * @brief Generates a flat ground plane.
         * @param size Plane width/length.
         * @return Generated mesh geometry.
         */
        [[nodiscard]] static MeshGeometry CreatePlane(float size = 20.0f);
    };

} // namespace Engine
